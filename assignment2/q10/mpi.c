// Anthony Sicoie (20214793)

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N_RUNS 1000

int main(int argc, char *argv[]) {
  int rank, size;
  MPI_Status status;
  int DATA_COUNT;
  double total_time = 0.0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Allow user to specify data coutn (for use in performance testing script)
  if (argc < 2) {
    if (rank == 0) {
      fprintf(stderr, "Usage: %s <array_size>\n", argv[0]);
    }
    MPI_Finalize();
    return 1;
  }

  DATA_COUNT = atoi(argv[1]);

  char *data;

  data = malloc(DATA_COUNT * sizeof(char));

  if (!rank) {
    for (int i = 0; i < DATA_COUNT; i++) {
      data[i] = (char)i;
    }
  }

  for (int run = 0; run < N_RUNS; run++) {
    double offset = MPI_Wtime();
    double start = MPI_Wtime();

    MPI_Bcast(data, DATA_COUNT, MPI_CHAR, 0, MPI_COMM_WORLD);

    double end = MPI_Wtime();

    double timer_overhead = start - offset;
    total_time += (end - start - timer_overhead);
  }

  double avg_time = total_time / N_RUNS;

  if (!rank) {
    FILE *f = fopen("results.csv", "a");
    if (f == NULL) {
      fprintf(stderr, "Error opening results.csv\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    fprintf(f, "%d,%d,%.9f\n", size, DATA_COUNT, avg_time);
    fclose(f);

    printf("Processes=%d Arraysize=%d AvgTime=%.9f sec\n", size, DATA_COUNT,
           avg_time);
    // printf("sizeof(int) %lu\n", sizeof(int));
  }

  MPI_Finalize();

  return 0;
}
