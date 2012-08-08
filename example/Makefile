ifeq ($(wildcard ../valgrind-source/install/include/valgrind/mctracer.h),)
	INC =
else 
	INC = -I../valgrind-source/install/include
endif

CFLAGS=-g -O2 $(INC)

all: redblack jc mm batman

redblack: redblack.c

jc: jc.c

mm: mm.c

batman: batman.c

redblack_ncurses: redblack_ncurses.c
	$(CC) $(CFLAGS) redblack_ncurses.c -o redblack_ncurses -lcurses

clean:
	rm -f redblack redblack_ncurses jc mm
