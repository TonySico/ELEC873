// Anthony Sicoie (20214793)

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N_RUNS 1000

int main(int argc, char *argv[]) {
  int rank, size;
  MPI_Status status;
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

  int DATA_COUNT = atoi(argv[1]);

  char *data;
  data = malloc(DATA_COUNT * sizeof(char));

  if (!rank) {
    for (int i = 0; i < DATA_COUNT; i++) {
      data[i] = (char)i;
    }
  }

  // Loop for benchmarking
  for (int run = 0; run < N_RUNS; run++) {

    double offset = MPI_Wtime();
    double start = MPI_Wtime();

    if (!rank) {
      MPI_Send(data, DATA_COUNT, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
      MPI_Recv(data, DATA_COUNT, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
    }

    if (rank) {
      MPI_Recv(data, DATA_COUNT, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
      MPI_Send(data, DATA_COUNT, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    double end = MPI_Wtime();
    double timer_overhead = start - offset;
    total_time += (end - start - timer_overhead);
  }

  free(data);

  if (!rank) {

    double avg_time = total_time / N_RUNS;
    double avg_latency = avg_time / 2.0;

    double bandwidth = (DATA_COUNT / avg_latency) / (1024.0 * 1024.0);

    FILE *f = fopen("results.csv", "a");
    if (f == NULL) {
      fprintf(stderr, "Error opening results.csv\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    fprintf(f, "%d,%d,%.9f,%.9f,%.6f\n", size, DATA_COUNT, avg_time,
            avg_latency, bandwidth);
    fclose(f);

    printf("Processes=%d Arraysize=%d AvgTime=%.9f sec Latency=%.9f sec "
           "Latency=%.6f MB/s\n",
           size, DATA_COUNT, avg_time, avg_latency, bandwidth);
  }

  MPI_Finalize();

  return 0;
}
