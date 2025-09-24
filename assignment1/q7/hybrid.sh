#!/bin/bash

# Hybrid executable
EXE=./hybrid

# Check if the executable exists
if [[ ! -x $EXE ]]; then
    echo "Error: Hybrid executable not found. Build it first."
    exit 1
fi

# Define the combinations of MPI processes x OpenMP threads
COMBOS=("2 8" "4 4" "8 2")

# Loop over each combination
for combo in "${COMBOS[@]}"; do
    read np nt <<< "$combo"
    LOG_FILE="hybrid_${np}x${nt}.log"
    TIME_FILE="hybrid_${np}x${nt}.time"

    echo "Running hybrid with MPI=$np, OMP=$nt"
    echo "Logging to $LOG_FILE, timing to $TIME_FILE"

    # Run with timing
    { time OMP_NUM_THREADS=$nt mpirun --oversubscribe -np $np $EXE ; } &> >(tee "$LOG_FILE") 2> >(tee "$TIME_FILE" >&2)
done

