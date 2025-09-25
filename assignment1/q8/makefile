# Compiler settings
MPISCC    = mpicc
MPIDCC    = mpicc
MANCC     = gcc 
MANDISPCC = gcc 

# Executable names
MPIS_EXE     = mpi_static
MPID_EXE     = mpi_dynamic
MAN_EXE     = man
MANDISP_EXE = mandisp

# Sources
MPIS_SRC     = mpi_static.c
MPID_SRC     = mpi_dynamic.c
MAN_SRC  		= mandelbrot-calculation.c
MANDISP_SRC  		= mandelbrot-display.c

# Default build rule
build: $(MPIS_EXE) $(MPID_EXE) $(MAN_EXE) $(MANDISP_EXE) 

$(MPIS_EXE): $(MPIS_SRC)
	$(MPISCC) -o $@ $<

$(MPID_EXE): $(MPID_SRC)
	$(MPIDCC) -o $@ $<

$(MAN_EXE): $(MAN_SRC)
	$(MANCC) -o $@ $<

$(MANDISP_EXE): $(MANDISP_SRC)
	$(MANDISPCC) -o $@ $< -lX11 -lm

# Run all programs with a user-specified number of processes/threads
# Usage: make runall N=4
runall: runm runman rund

runms:
ifndef n
	$(error Please provide n, e.g. make runm n=4)
endif
	mpirun --oversubscribe -np $(n) ./$(MPIS_EXE) 2>&1 | tee mpi.log

runmd:
ifndef n
	$(error Please provide n, e.g. make runm n=4)
endif
	mpirun --oversubscribe -np $(n) ./$(MPID_EXE) 2>&1 | tee mpi.log

runman:
	./$(MAN_EXE) 

rund:
	./$(MANDISP_EXE) 

# Clean up binaries and logs
clean:
	rm -f $(MPIS_EXE) $(MPID_EXE) $(MAN_EXE) $(MANDISP_EXE) *.log

