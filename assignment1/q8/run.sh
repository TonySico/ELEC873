#!/bin/bash

# Anthony Sicoie (20214793)

SEQ_EXEC=./man
STATIC_EXEC=./mpi_static
DYNAMIC_EXEC=./mpi_dynamic

PROCS=(2 4 8 16 32)

rm -f seq.csv static.csv dynamic.csv

trap "echo 'Interrupted! Exiting.'; exit 1" SIGINT

echo "--- Running Sequential ---"
$SEQ_EXEC
echo

# Run MPI tests
for procs in "${PROCS[@]}"; do
    echo "--- Running with $procs processes ---"

    echo "Static:"
    mpirun -np $procs --oversubscribe $STATIC_EXEC

    echo "Dynamic:"
    mpirun -np $procs --oversubscribe $DYNAMIC_EXEC

    echo
done

