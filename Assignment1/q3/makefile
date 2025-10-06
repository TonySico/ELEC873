# Anthony Sicoie (20214793)

# Compiler settings
OMPCC   = gcc -fopenmp

# Executable names
OPENMP_EXE  = openmp

# Sources
OPENMP_SRC  = openmp.c

build: $(OPENMP_EXE) 

$(OPENMP_EXE): $(OPENMP_SRC)
	$(OMPCC) -o $@ $<

runall: runo 

runo:
ifndef n
	$(error Please provide n, e.g. make runo n=4)
endif
	OMP_NUM_THREADS=$(n) ./$(OPENMP_EXE) 2>&1 | tee openmp.log

clean:
	rm -f $(OPENMP_EXE) *.log

