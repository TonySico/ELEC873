# Anthony Sicoie (20214793)

# Compiler settings
MPICC   = mpicc

# Executable names
MPI_EXE     = mpi

# Sources
MPI_SRC     = mpi.c

build: $(MPI_EXE) 

$(MPI_EXE): $(MPI_SRC)
	$(MPICC) -o $@ $<

runall: runm 

runm:
	mpirun --oversubscribe -np 2 ./$(MPI_EXE) 2 2>&1 | tee mpi.log

clean:
	rm -f $(MPI_EXE) *.log

