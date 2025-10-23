#!/bin/bash

# Anthony Sicoie (20214793)

EXEC=./mpi

DATA_SIZES=(4 32 256 2048 16384 131072 1048576)

rm 8.csv
trap "echo 'Interrupted! Exiting.'; exit 1" SIGINT

for size in "${DATA_SIZES[@]}"; do
    echo "===== Array size: $size ====="
    mpirun -np 8 --oversubscribe $EXEC $size
    echo
done

mv results.csv 8.csv
