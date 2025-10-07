#!/bin/bash

# Anthony Sicoie (20214793)

EXEC=./mpi

ARRAY_SIZES=(1 10 100 1024 10240 102400 1048576)
#           1B, 10B, 100B, 1KB, 10KB, 100KB and 1MB

rm results.csv
trap "echo 'Interrupted! Exiting.'; exit 1" SIGINT

for size in "${ARRAY_SIZES[@]}"; do
echo "===== Array size: $size ====="
    mpirun -np 2 $EXEC $size
    echo
done
