# Anthony Sicoie (20214793)

# Compiler settings
MPICC   = mpicc

# Executable names
MPI_EXE     = mpi

# Sources
MPI_SRC     = mpi.c

build: $(MPI_EXE) 

$(MPI_EXE): $(MPI_SRC)
	$(MPICC) -o $@ $< -lm

runall: runm 

runm:
ifndef n
	$(error Please provide n, e.g. make runm n=4)
endif
	mpirun --oversubscribe -np $(n) ./$(MPI_EXE) 2>&1 | tee mpi.log

clean:
	rm -f $(MPI_EXE) *.log

