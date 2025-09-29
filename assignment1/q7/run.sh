#!/bin/bash

# Anthony Sicoie (20214793)

EXEC=./hybrid

ARRAY_SIZES=(256 512 1024 2048 4096 8192 16384 32768)

rm results.csv
trap "echo 'Interrupted! Exiting.'; exit 1" SIGINT

declare -A configs
configs["2x8"]="2 8"
configs["4x4"]="4 4"
configs["8x2"]="8 2"

for size in "${ARRAY_SIZES[@]}"; do
    echo "===== Array size: $size ====="
    for label in "${!configs[@]}"; do
        procs=$(echo ${configs[$label]} | awk '{print $1}')
        threads=$(echo ${configs[$label]} | awk '{print $2}')

        echo "--- Config: $label (procs=$procs, threads=$threads) ---"
        export OMP_NUM_THREADS=$threads
        mpirun -np $procs $EXEC $size
        echo
    done
done

