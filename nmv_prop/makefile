
#FC=ifort
FC=gfortran

LDFLAGS=-L/opt/intel/mkl/lib/intel64 -llapack #-L/home/pnroy/Dev/lib64/

asymrho.x:	asymrho.o
		$(FC) -o $@ asymrho.o $(LDFLAGS)

.f.o:
	${FC} -O3 -c $<

clean:
	rm -f *.o asymrho.x
