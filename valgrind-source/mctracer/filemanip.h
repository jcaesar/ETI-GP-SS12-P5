// Beware of massive trickery: Valgrind doesn't seem to supply VG_(fopen) and alikes, so this is a workaround
// Replace it with something "normal" asap!

#include <stdint.h>
#define ASL "\n\t"

#define HX_WRONLY 0x01
#define HX_CREATE  0x40

// Liunux calling conventions used here
uint64_t hxopen(char * filename, uint64_t modif, uint64_t mode)
{
	uint64_t fd;
	__asm__("mov $5,%%rax" ASL
			"mov %1,%%rbx" ASL
			"mov %2,%%rcx" ASL
			"mov %3,%%rdx" ASL
			"int $0x80" ASL
			"mov %%rax,%0"
			:"=r"(fd)
			:"r"(filename), "r"(modif), "r"(mode)
			:"%rax", "%rbx", "%rcx"
		   );
	return fd;
}

#define WRITE_ATONCE 512
char writebuf[WRITE_ATONCE];
void hxwrite(uint64_t fd, void * buffer, uint64_t length)
{
	uint64_t i;
	for(i = 0; i < length; i += WRITE_ATONCE)
	{
		uint64_t segl = length - i > WRITE_ATONCE ? WRITE_ATONCE : length - i;
		uint64_t j; 
		for(j = 0; j < segl; ++j)
			writebuf[j] = *(((char*)buffer) + j);
		__asm__ __volatile__(
			"mov $4,%%rax" ASL // write it
			"mov %0,%%rbx" ASL
			"mov %1,%%rcx" ASL
			"mov %2,%%rdx" ASL
			"int $0x80" ASL
			:
			:"r"(fd), "r"(writebuf), "r"(segl)
			:"%rax", "%rbx", "%rcx", "%rdx"
		   );
	}
}
#undef writebuf
#undef WRITE_ATONCE

void hxputc(uint64_t fd, char c)
{
	hxwrite(fd, &c, 1);
}	

void hxclose(uint64_t fd)
{
	__asm__("mov $6,%%rax" ASL
			"mov %0,%%rbx" ASL
			"int $0x80" ASL
			:
			:"r"(fd)
			:"rax", "rbx"
		   );
}

#undef ASL
