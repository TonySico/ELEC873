# Compiler settings
MPICC   = mpicc

# Executable names
MPI_EXE     = mpi

# Sources
MPI_SRC     = mpi.c

# Default build rule
build: $(MPI_EXE) $(OPENMP_EXE) $(PTHREAD_EXE)

$(MPI_EXE): $(MPI_SRC)
	$(MPICC) -o $@ $<


# Run all programs with a user-specified number of processes/threads
# Usage: make runall N=4
runall: runm 

runm:
ifndef n
	$(error Please provide n, e.g. make runm n=4)
endif
	mpirun --oversubscribe -np $(n) ./$(MPI_EXE) 2>&1 | tee mpi.log

# Clean up binaries and logs
clean:
	rm -f $(MPI_EXE) *.log

