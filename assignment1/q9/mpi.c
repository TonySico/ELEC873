#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAYSIZE 1000

void calcData(int *dataCount) {}

int main(int argc, char *argv[]) {
  int rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int *data;
  int dataCount = 0;

  // if (!rank) {
  //   data = malloc(ARRAYSIZE * sizeof(int));
  //   for (int i = 0; i < ARRAYSIZE; i++)
  //     data[i] = i;
  // }

  if (!rank) {
    int maxDepth = ((int)ceil(log((double)size) / log((double)2)));
    printf("depth %d\n", maxDepth);
  }

  MPI_Finalize();
  return 0;
}
