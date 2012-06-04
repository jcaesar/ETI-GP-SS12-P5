#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <time.h>

#define SIZE 1024

#define min(x,y) (((x)<(y))?(x):(y))
#define abs(x)   (((x)<0)?(-(x)):(x))

bool signaled = false;
void sighandler(int sig) { signaled = true; }

int main()
{
	signal(SIGINT, sighandler);

	initscr();
	cbreak();
	int screen_sx,screen_sy;
	getmaxyx(stdscr, screen_sy, screen_sx);
	if(has_colors())
		start_color();
	init_pair(1, COLOR_RED,  COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_RED,  COLOR_GREEN);

	double* m;
	int i, j, it;

	m = (double*) malloc(sizeof(double)*SIZE*SIZE);
	typedef struct _disphlp {
		unsigned short mappc;
		double mapsum;
	} disphelp;
	disphelp * disp = (disphelp*) malloc(screen_sy*screen_sx*sizeof(disphelp));

	// init points
	for(i=0; i<SIZE; i++)
		for(j=0; j<SIZE; j++)
			m[i*SIZE+j] = 0.0;
	// set left/right border
	for(i=0; i<SIZE; i++)
	{
		m[ i*SIZE + 0        ] = 10.0;
		m[ i*SIZE + (SIZE-1) ] = -10.0;
	}

	// add some random points
	srand(time(NULL));
	for(i=0; i < 200; ++i) 
	{
		int x = (rand()%SIZE);
		m[ (rand()%SIZE)*SIZE + x ] = 50000.0 * ((rand()%SIZE)>x?1:-1);
	}

	while(!signaled)
	{
		// update inner black points
		for(i=1; i<SIZE-1; i++)
			for(j=1+(i%2); j<SIZE-1; j+=2)
				m[i*SIZE+j] = ( m[(i-1)*SIZE +j] +
				                m[(i+1)*SIZE +j] +
				                m[ i*SIZE + j-1] +
				                m[ i*SIZE + j+1] )/4.0;

		// update inner red points
		for(i=1; i<SIZE-1; i++)
			for(j=1+((i+1)%2); j<SIZE-1; j+=2)
				m[i*SIZE+j] = ( m[(i-1)*SIZE +j] +
				                m[(i+1)*SIZE +j] +
				                m[ i*SIZE + j-1] +
				                m[ i*SIZE + j+1] )/4.0;

		memset(disp, 0, screen_sy*screen_sx*sizeof(disphelp));
		for(i=0; i<SIZE; ++i)
			for(j=0; j<SIZE; ++j)
			{
				int dx = j / ((double)SIZE/screen_sx), dy = i / ((double)SIZE/screen_sy);
				dx = min(screen_sx-1, dx); dy = min(screen_sy-1, dy); // float arithmetics, do you trust it?
				disp[dy*screen_sx+dx].mapsum += m[i*SIZE+j];
				disp[dy*screen_sx+dx].mappc++;
			}
		for(i = 0; i < screen_sy*screen_sx; ++i)
		{
			const char dispvals [] = " .,o&%$#@"; // I'm soo tempted to use japanese characters, you can make a nice grey gradient with those...
			int cnt = disp[i].mappc;
			char chr;
			int color;
			if(cnt == 0)
			{
				chr = 'E';
				color = 3;
			}
			else 
			{
				double val = disp[i].mapsum / cnt / 10.0 * strlen(dispvals);
				chr = dispvals[min(strlen(dispvals)-1,(int)abs(val))];
				color = (val < 0) ? 2 : 1;
			}
			attron(COLOR_PAIR(color));
			mvwaddch(stdscr, i/screen_sx, i%screen_sx, chr);
			attroff(COLOR_PAIR(color));
		}
		wrefresh(stdscr);

	}
	bool hadcolors = has_colors();
	endwin();

	// print checksum
	double asum = 0.0, sum = 0.0;
	for(i=0; i<SIZE; i++)
		for(j=0; j<SIZE; j++)
		{
			sum += m[i*SIZE+j];
			asum += abs(m[i*SIZE+j]);
		}
	if(!hadcolors)
		printf("Terminal didn't support colors. :(\n");
	printf("Sum: %f Abs-Sum: %f\n", sum, asum);

	return 1;
}

