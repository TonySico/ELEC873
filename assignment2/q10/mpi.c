// Anthony Sicoie (20214793)

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N_RUNS 1 // TODO: Change this back to 100

int main(int argc, char *argv[]) {
  int rank, size;
  MPI_Status status;
  int DATA_COUNT;
  double total_time_init = 0.0;
  double total_time_no_init = 0.0;

  // Allow user to specify data coutn (for use in performance testing script)
  if (argc < 2) {
    if (rank == 0) {
      fprintf(stderr, "Usage: %s <array_size>\n", argv[0]);
    }
    MPI_Finalize();
    return 1;
  }

  DATA_COUNT = atoi(argv[1]);

  double offset = MPI_Wtime();
  double start_init = MPI_Wtime();

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  double start_no_init = MPI_Wtime();

  int *data;

  data = malloc(DATA_COUNT * sizeof(int));

  if (!rank) {
    for (int i = 0; i < DATA_COUNT; i++) {
      data[i] = i;
    }
    MPI_Bcast(&data, DATA_COUNT, MPI_INT, 0, MPI_COMM_WORLD);
  }

  double end_no_init = MPI_Wtime();

  if (MPI_Finalize() != MPI_SUCCESS)
    return 1;

  double end_init = MPI_Wtime();

  double timer_overhead_init = start_init - (2 * offset);
  double timer_overhead_no_init = start_init - offset;

  total_time_init += (end_init - start_init - timer_overhead_init);
  total_time_no_init += (end_no_init - start_no_init - timer_overhead_no_init);

  double avg_time_init = total_time_init / N_RUNS;
  double avg_time_no_init = total_time_no_init / N_RUNS;

  if (!rank) {
    FILE *f = fopen("results.csv", "a");
    if (f == NULL) {
      fprintf(stderr, "Error opening results.csv\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    fprintf(f, "%d,%d,%.6f,%.6f\n", size, DATA_COUNT, avg_time_init,
            avg_time_no_init);
    fclose(f);

    printf(
        "Processes=%d Arraysize=%d AvgTimeInit=%.6f AvgTimeNoInit=%.6f sec\n",
        size, DATA_COUNT, avg_time_init, avg_time_no_init);
    // printf("sizeof(int) %lu\n", sizeof(int));
  }

  return 0;
}
