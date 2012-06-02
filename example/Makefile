ifeq ($(wildcard ../valgrind-source/install/include/valgrind/mctracer.h),)
	INC =
else 
	INC = -I../valgrind-source/install/include
endif

CFLAGS=-g -O2 $(INC)

all: redblack jc mm

clean:
	rm -f redblack jc mm
