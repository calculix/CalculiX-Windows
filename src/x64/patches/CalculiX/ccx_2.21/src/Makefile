
CFLAGS = -Wall -O3  -I ../../../SPOOLES.2.2 -I ../../../pthreads-w32-2-9-1-release/Pre-built.2/include -DARCH="Linux" -DSPOOLES -DARPACK -DMATRIXSTORAGE -D_SC_NPROCESSORS_CONF=1 -posix -std=c99
FFLAGS = -Wall -O3 -fopenmp

CC=gcc
FC=gfortran

.c.o :
	$(CC) $(CFLAGS) -c $<
.f.o :
	$(FC) $(FFLAGS) -c $<

include Makefile.inc

SCCXMAIN = ccx_2.21.c

OCCXF = $(SCCXF:.f=.o)
OCCXC = $(SCCXC:.c=.o)
OCCXMAIN = $(SCCXMAIN:.c=.o)

DIR=../../../SPOOLES.2.2

LIBS = \
       $(DIR)/spooles.a \
	../../../ARPACK/libarpack_MINGW64.a \
       ../../../pthreads-w32-2-9-1-release/Pre-built.2/lib/x64/libpthreadGC2.a -lm

ccx_2.21: $(OCCXMAIN) ccx_2.21.a
	./date.pl; $(CC) $(CFLAGS) -c ccx_2.21.c; $(FC) -fopenmp -Wall -O3 -o $@ $(OCCXMAIN) ccx_2.21.a $(LIBS)

ccx_2.21.a: $(OCCXF) $(OCCXC)
	ar vr $@ $?
                                                                               
