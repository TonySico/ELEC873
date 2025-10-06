// Anthony Sicoie (20214793)

#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Used for development, removed for performane testing
// #define ARRAY_SIZE 256
#define N_RUNS 100

int main(int argc, char *argv[]) {
  int rank, size;

  srand(20214793);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Allow user to specify array size (for use in performance testing script)
  if (argc < 2) {
    if (rank == 0) {
      fprintf(stderr, "Usage: %s <array_size>\n", argv[0]);
    }
    MPI_Finalize();
    return 1;
  }

  int ARRAY_SIZE = atoi(argv[1]);

  int *data;
  int split = ARRAY_SIZE / size;
  int global_min;

  double total_time = 0.0;

  // Loop for benchmarking
  for (int run = 0; run < N_RUNS; run++) {
    double tmp = MPI_Wtime();
    double start = MPI_Wtime();

    if (rank == 0) {
      data = malloc(ARRAY_SIZE * sizeof(int));
      for (int i = 0; i < ARRAY_SIZE; i++)
        data[i] = rand() % 1000;
    } else {
      data = malloc(split * sizeof(int));
    }

    MPI_Scatter(data, split, MPI_INT, data, split, MPI_INT, 0, MPI_COMM_WORLD);

    int local_min = data[0];

    // use OpenMP directive to find thread local mins
#pragma omp parallel for reduction(min : local_min)
    for (int i = 0; i < ARRAY_SIZE / size; i++) {
      if (data[i] < local_min)
        local_min = data[i];
    }

    // Reduce the local mins found by each processes
    MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0)
      printf("Global min = %d\n", global_min);

    free(data);

    double end = MPI_Wtime();
    double timer_overhead = start - tmp;
    total_time += (end - start - timer_overhead);
  }

  double avg_time = total_time / N_RUNS;

  if (rank == 0) {
    int threads = omp_get_max_threads();

    FILE *f = fopen("results.csv", "a");
    if (f == NULL) {
      fprintf(stderr, "Error opening results.csv\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    fprintf(f, "%d,%d,%d,%.6f\n", size, threads, ARRAY_SIZE, avg_time);
    fclose(f);

    printf("Processes=%d Threads=%d Arraysize=%d AvgTime=%.6f sec\n", size,
           threads, ARRAY_SIZE, avg_time);
  }

  MPI_Finalize();
  return 0;
}
