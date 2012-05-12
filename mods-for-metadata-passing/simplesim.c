/*
 * Event consumer for mctracer: simple cache simulator.
 * For ETI @ TUM, (C) 2011 Josef Weidendorfer
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shmlib/shm_consumer.h"

// 64-bit type for addresses: this needs mctracer to be 64bit binary !!
typedef unsigned long long Addr;

// type Addr is used in events definitions
#include "tr_shmevents.h"

/* ----------------------------------------------------------------*/

/*
 * Simulator for a shared cache
 */

// Cache with 8192 cache lines (default) a 64 byte = 1 MB cache size
// Associativity 16 (= number of cache lines per set)


#define LINESIZE 64
int cachelines = 8192;
int setsize = 16;

// derived parameter
#define SETS (cachelines / setsize)

//Define true and false
#define TRUE 1
#define FALSE 0

#define DEBUG_ON
#ifdef DEBUG_ON
#define DEBUG(code) code
#else 
#define DEBUG(code)
#endif

typedef struct _section{
	int bytes_used[LINESIZE+1];
	int homogenity[101];
	int id;
	unsigned int misses;
	char description[64];
} Section;

typedef struct _sectionnode{
	Section* section;
	struct _sectionnode* next; 
} SectionNode;

typedef struct _sectionlist
{ 
	SectionNode* first;
	SectionNode* last; 
} SectionList;

SectionList* createSectionList(Section* section)
{
	SectionList* list=malloc(sizeof(SectionList));
	SectionNode* node=malloc(sizeof(SectionNode));
	node->section=section;
	node->next=NULL;
	list->first=node;
	list->last=node;
	return list;
}
void addSection(SectionList* list, Section* section)
{
	SectionNode* node=malloc(sizeof(SectionNode));
	node->section=section;
	node->next=NULL;
	if(list->last==NULL)
	{
		list->first=node;
	}
	else
	{	
		list->last->next=node;	
	}
	list->last=node;
}

void addSectionChecked(SectionList* list, Section* section)
{
	//check if section is already in list
	SectionNode* nextSection=list->first;
	while(nextSection!=NULL)
	{
		if(nextSection->section->id==section->id)
		{
			return;
		}
		nextSection=nextSection->next;	
	}
	SectionNode* node=malloc(sizeof(SectionNode));
	node->section=section;
	node->next=NULL;
	if(list->last==NULL)
	{
		list->first=node;
	}
	else
	{	
		list->last->next=node;	
	}
	list->last=node;
}

void freeSectionList(SectionList* list, int keepSections)
{
	SectionNode* next=list->first;
	SectionNode* tmp=NULL;
	while(next!=NULL)
	{
		if(keepSections==FALSE)
			free(next->section);
		tmp=next->next;
		free(next);
		next=tmp;
	}
}

typedef struct _data{
	Addr start;
	Addr end;
	Section* section;
} Data;

typedef struct _datanode{
	Data* data;
	struct _datanode* next; 
} DataNode;

typedef struct _datalist
{ 
	DataNode* first;
	DataNode* last; 
} DataList;


void addData(DataList* list, Data* data)
{
	DataNode* node=malloc(sizeof(DataNode));
	node->data=data;
	node->next=NULL;
	if(list->last==NULL)
	{
		list->first=node;
	}
	else
	{	
		list->last->next=node;	
	}
	list->last=node;
}

void freeDataList(DataList* list)
{
	DataNode* next=list->first;
	DataNode* tmp=NULL;
	while(next!=NULL)
	{
		free(next->data);
		tmp=next->next;
		free(next);
		next=tmp;
	}
}

typedef struct _cacheline {
    Addr tag;            // 64 bit architecture
    int accesses[LINESIZE];
    SectionList sections;
} Cacheline;


Cacheline* cache;

Section* currentSection=NULL;
SectionList sections={NULL,NULL};
DataList dataList={NULL,NULL};
unsigned int misses=0;


void cache_clear()
{
	free(cache);
	cache = (Cacheline* ) malloc(sizeof(Cacheline) * cachelines);
    int i;
    int j;
    for(i=0; i<cachelines; i++) 
    {
      cache[i].tag = 0;
      for(j=0;j<LINESIZE;++j)
	cache[i].accesses[j]=0;
    }
}

void save_line(Cacheline l)
{
  int cl_bytes_used=0;
  int sum_accesses=0;
  int max_accesses=0;
  int i;
  SectionNode* nextSection;
  for(i=0;i<LINESIZE;++i)
  {
    if(l.accesses[i]>0)
      cl_bytes_used++;
    sum_accesses+=l.accesses[i];
    if(l.accesses[i]>max_accesses)
      max_accesses=l.accesses[i];
  } 
  if(max_accesses==0)
	return;
  int cl_homogenity=((float)(sum_accesses)/(float)(LINESIZE))/(float)max_accesses*100.0f;
  if(sum_accesses>0)
  {
	nextSection=l.sections.first;
	while(nextSection!=NULL)
	{
		nextSection->section->bytes_used[cl_bytes_used]++;
    	nextSection->section->homogenity[cl_homogenity]++;
		nextSection->section->misses++;
		nextSection=nextSection->next;	
	}
	freeSectionList(&l.sections,TRUE);   
  }
}

// a reference into a set of the cache, return 1 on hit
int cache_setref(int set_no, Addr tag, int byte)
{
    int i, j;
    Cacheline* set = cache + set_no * setsize;
    unsigned old_mask;

    /* Test all lines in the set for a tag match
     * If the tag is another than the MRU, move it into the MRU spot, count access
     * and shuffle the rest down.
     */
    for (i = 0; i < setsize; i++) {
        if (tag == set[i].tag) {
		 Cacheline accessed_line=set[i];
            for (j = i; j > 0; j--) {
                set[j]= set[j - 1];
            }
            set[0] = accessed_line;
            set[0].accesses[byte]++;
			addSectionChecked(&(set[0].sections),currentSection);
            return 1;
        }
    }

    /* A miss; save LRU to file, install this tag as MRU, shuffle rest down. */
    save_line(set[setsize-1]);    
    for (j = setsize - 1; j > 0; j--) {
        set[j]= set[j - 1];
    }
	misses++;

    Cacheline new_line;
    new_line.tag=tag;
    for(i=0;i<LINESIZE;++i)
    {
        new_line.accesses[i]=0;
    }
    new_line.accesses[byte]++;
	new_line.sections.first=NULL;
	new_line.sections.last=NULL;
	addSection(&(new_line.sections),currentSection);
    set[0]=new_line;
    return 0;
}

// a reference at address <a> with size <s>, return 1 on hit
int cache_ref(Addr a, int size)
{
    int i;
    int hit=1;
	int ref_hit=1;
	DataNode* nextData;
	SectionNode* nextSection;
	int found=FALSE;
	int lastSet=-1;
    for(i=0;i<size;++i)
    {
        int  set = ( (a+i) / LINESIZE) & (SETS-1);
        Addr tag = (a+i) / LINESIZE / SETS;
        int byte = (a+i)& (LINESIZE-1);   // equals (a+i)%LINESIZE
		ref_hit=cache_setref(set,tag,byte);
		hit*=ref_hit;
		if(lastSet!=set)
		{
			lastSet=set;

			nextData=dataList.first;
			while(nextData!=NULL)
			{
				if(a+i>=nextData->data->start && a+i<=nextData->data->end)
					addSectionChecked(&(cache[set*setsize+0].sections),nextData->data->section);
				nextData=nextData->next;
			}
		}
    }
    return hit;
}


/* ----------------------------------------------------------------*/

/* global counters for cache simulation */
int loads = 0, stores = 0, lmisses = 0, smisses = 0;

/* thread executing next memory accesses */
int tid = 0;


void run_tid(ev_run_tid* e)
{  
  // not really needed here: we assume a shared cache for all threads
  tid = e->tid;
}

void data_read(ev_data_read* e)
{
  int res;
  res = cache_ref(e->addr, e->len);
  // printf(" > Load  by T%d at %p, size %2d: %s\n",
  //	 tid, (void*) e->addr, e->len, res ? "Hit ":"Miss");
  loads++;
  if (res == 0) lmisses++;
}

void data_write(ev_data_write* e)
{
  int res;
  res = cache_ref(e->addr, e->len);
  // printf(" > Store by T%d at %p, size %2d: %s\n",
  //	 tid, (void*) e->addr, e->len, res ? "Hit ":"Miss");
  stores++;
  if (res == 0) smisses++;
}

void configure(ev_simplesim_configure* e)
{
	if(strcmp(e->setting, "cachelines") == 0){
		cachelines = e->value;
		DEBUG(printf("Reconfigured for %d cachelines\n", cachelines);)
	}else if(strcmp(e->setting, "setsize") == 0){
		setsize = e->value;
	}
	cache_clear();
}

void data_define(ev_simplesim_define_data* define_data){
  SectionNode* nextSection=sections.first;
	int lowestID=0;
	Data* newData;
	int i;
	while(nextSection!=NULL)
	{
		if(nextSection->section->id < lowestID)
			lowestID=nextSection->section->id;
		nextSection=nextSection->next;
	}
	newData=malloc(sizeof(Data));
	newData->start=define_data->start;
	newData->end=define_data->start+define_data->size;
	newData->section=malloc(sizeof(Section));
	memset ( newData->section->bytes_used, 0, (LINESIZE+1)*sizeof (int) );
	memset ( newData->section->homogenity, 0, 101*sizeof (int) );
	newData->section->id=lowestID-1;
	newData->section->misses=0;
	for(i=0;i<64;++i)
	{
		newData->section->description[i] = define_data->description[i];
	}
	addData(&dataList,newData);
	addSection(&sections,newData->section);

  DEBUG(printf("user request, data define %s, start: %p, size %d\n", define_data->description, (void *) define_data->start, define_data->size);)
}

void change_section(ev_simplesim_change_section* section_change){
	Section* found=NULL;
	SectionNode* nextSection=sections.first;
	Section* section;
	int i;
	while(nextSection!=NULL)
	{
		if(nextSection->section->id==section_change->id)
		{
			found=nextSection->section;
		}
		nextSection=nextSection->next;
	}
	if(found==NULL)
	{
		section=malloc(sizeof(Section));
		memset ( section->bytes_used, 0, (LINESIZE+1)*sizeof (int) );
		memset ( section->homogenity, 0, 101*sizeof (int) );
		section->id=section_change->id;
		section->misses=0;
		for(i=0;i<64;++i)
		{
			section->description[i] = section_change->description[i];
	   	}
		addSection(&sections,section);
		currentSection=section;
	}
	else
	{
		currentSection=found;
	}
  	DEBUG(printf("user request, change section ID: %d \n",section_change->id);)//
}

int main(int argc, char* argv[])
{
    
    cache_clear();
    
    Section* mainSection=malloc(sizeof(Section));
    memset ( mainSection->bytes_used, 0, (LINESIZE+1)*sizeof (int) );
    memset ( mainSection->homogenity, 0, 101*sizeof (int) );
    mainSection->id=0;
    mainSection->misses=0;
    int i;
    char* desc="default";
    for(i=0;desc[i]!='\0';++i)
    {
	    mainSection->description[i] = desc[i];
    }
    mainSection->description[i]='\0';
    currentSection=mainSection;
    addSection(&sections,mainSection);
    
    shm_buf* buf;
    shm_rb* rb;
    rb_chunk* chunk;
    tr_event* e;

    /* initialize event passing via shared memory */
    buf = shm_init(argc, argv);
    rb = open_rb(buf, "tr_main");
    if (!rb) {
      printf("Cannot open ring buffer 'tr_main'\n");
      exit(1);
    }



    chunk = open_first(rb);
    while( (e = (tr_event*) next_event(&chunk)) ) {
      switch(e->tag) {
      case TR_RUN_TID:
	run_tid(&(e->run_tid));
	break;
      case TR_DATA_READ:
	data_read(&(e->data_read));
	break;
      case TR_DATA_WRITE:
	data_write(&(e->data_write));
	break;
      case TR_SIMPLESIM_DEFINE_DATA:
	data_define(&(e->simplesim_define_data));
	break;
	  case TR_SIMPLESIM_CHANGE_SECTION:
  	change_section(&(e->simplesim_change_section));
  	break;
  	  case TR_SIMPLESIM_CONFIGURE:
    configure(&(e->simplesim_configure));
    break;
      default:
	printf(" Unknown event tag %d\n", e->tag);
	abort();
	break;
      }
    }
    //save remaining cachelines
    for(i=0;i<cachelines;++i)
      save_line(cache[i]);
      

    printf("\n[%d,",misses);
    //write all sections
	SectionNode* next=sections.first;
	while(next!=NULL)
	{
		printf("[\"%s\",%d,[",next->section->description,next->section->misses);
		// write bytes_used to file
		for (i=0;i<LINESIZE;++i)
		{  
		  printf("%i,", next->section->bytes_used[i]);
		}
		printf("%i],", next->section->bytes_used[LINESIZE]); 


		// write homogenity to file
		printf("[");
		for (i=0;i<100;++i)
		{  
		  printf("%i,", next->section->homogenity[i]);
		}
		printf("%i]]", next->section->homogenity[100]);
	
		next=next->next;
		if(next!=NULL)
			printf(",");  
	}   
    
    printf("]\n");

    return 1;
}
