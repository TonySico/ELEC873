# Anthony Sicoie (20214793)

# Compiler settings
MPICC   = mpicc
OMPCC   = gcc -fopenmp
PTHCC   = gcc -pthread

# Executable names
MPI_EXE     = mpi
OPENMP_EXE  = openmp
PTHREAD_EXE = pthread
HYBRID_EXE  = hybrid

# Sources
MPI_SRC     = mpi.c
OPENMP_SRC  = openmp.c
PTHREAD_SRC = pthread.c
HYBRID_SRC  = hybrid.c

# Default build rule
build: $(MPI_EXE) $(OPENMP_EXE) $(PTHREAD_EXE) $(HYBRID_EXE)

$(MPI_EXE): $(MPI_SRC)
	$(MPICC) -o $@ $<

$(OPENMP_EXE): $(OPENMP_SRC)
	$(OMPCC) -o $@ $<

$(PTHREAD_EXE): $(PTHREAD_SRC)
	$(PTHCC) -o $@ $<

$(HYBRID_EXE): $(HYBRID_SRC)
	$(MPICC) -fopenmp -o $@ $<

runall: runm runo runp

runm:
ifndef n
	$(error Please provide n, e.g. make runm n=4)
endif
	mpirun --oversubscribe -np $(n) ./$(MPI_EXE) 2>&1 | tee mpi.log

runo:
ifndef n
	$(error Please provide n, e.g. make runo n=4)
endif
	OMP_NUM_THREADS=$(n) ./$(OPENMP_EXE) 2>&1 | tee openmp.log

runp:
ifndef n
	$(error Please provide n, e.g. make runp n=4)
endif
	./$(PTHREAD_EXE) $(n) 2>&1 | tee pthread.log

runh:
ifndef np
	$(error Please provide np, e.g. make runh np=2 nt=4)
endif
ifndef nt
	$(error Please provide nt, e.g. make runh np=2 nt=4)
endif
	OMP_NUM_THREADS=$(nt) mpirun --oversubscribe -np $(np) ./$(HYBRID_EXE) 2>&1 | tee hybrid.log

clean:
	rm -f $(MPI_EXE) $(OPENMP_EXE) $(PTHREAD_EXE) $(HYBRID_EXE) *.log

