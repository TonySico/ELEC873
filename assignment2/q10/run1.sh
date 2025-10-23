#!/bin/bash

# Anthony Sicoie (20214793)

EXEC=./mpi

ARRAY_SIZES=(64 32768 1048576)
PROC_COUNTS=(4 8 16 32 64)

rm both.csv
trap "echo 'Interrupted! Exiting.'; exit 1" SIGINT

for count in "${PROC_COUNTS[@]}"; do
  echo "===== Proc Count: $count ====="
    for size in "${ARRAY_SIZES[@]}"; do
      echo "===== Array size: $size ====="
      mpirun -np $count --oversubscribe $EXEC $size
      echo
  done
done

mv results.csv both.csv
