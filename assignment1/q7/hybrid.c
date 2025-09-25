#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 256

int main(int argc, char *argv[]) {
  int rank, size;

  srand(20214793);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int *data;
  int split = ARRAY_SIZE / size;
  int global_min;

  if (rank == 0) {
    data = malloc(ARRAY_SIZE * sizeof(int));
    for (int i = 0; i < ARRAY_SIZE; i++)
      data[i] = rand() % 1000;
  } else {
    data = malloc(split * sizeof(int));
  }

  MPI_Scatter(data, split, MPI_INT, data, split, MPI_INT, 0, MPI_COMM_WORLD);

  int local_min = data[0];

#pragma omp parallel for reduction(min : local_min)
  for (int i = 0; i < ARRAY_SIZE / size; i++) {
    if (data[i] < local_min)
      local_min = data[i];
  }

  MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    // For verification
    // for (int i = 0; i < ARRAY_SIZE; i++) {
    //   printf("%d,", data[i]);
    // }
    // printf("\n");
    printf("Global min = %d\n", global_min);
  }

  free(data);

  MPI_Finalize();
  return 0;
}
