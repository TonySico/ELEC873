#!/bin/bash

# Anthony Sicoie (20214793)

EXEC=./mpi

# ARRAY_SIZES=(256 512 1024 2048 4096 8192 16384 32768)

ARRAY_SIZES=(8 16 32 64 128 256)
PROC_COUNTS=(4 8 16 32 64)

rm results.csv
trap "echo 'Interrupted! Exiting.'; exit 1" SIGINT

for count in "${PROC_COUNTS[@]}"; do
  echo "===== Proc Count: $count ====="
    for size in "${ARRAY_SIZES[@]}"; do
      echo "===== Array size: $size ====="
      mpirun -np $count --oversubscribe $EXEC $size
      echo
  done
done
