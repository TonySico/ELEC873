#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAYSIZE 1000

int add(int *data, int dataCount) {
  if (dataCount == 1)
    return data[0];
  if (dataCount == 2)
    return data[0] + data[1];

  int newCount = dataCount / 2;
  return add(data, newCount) + add(data + newCount, dataCount - newCount);
}

int depth(int rank, int size) {
  if (size <= 1)
    return 0; // base case: only one process

  int mid = size / 2;
  if (rank == 0)
    return 0; // root process
  else if (rank < mid)
    return 1 + depth(rank, mid);
  else
    return 1 + depth(rank - mid, size - mid);
}

int main(int argc, char *argv[]) {
  int rank, size, *data, count, tempInt, dataCount = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  MPI_Status status;
  int myDepth = depth(rank, size);
  int maxDepth = ((int)ceil(log((double)size) / log((double)2)));

  if (!rank) {
    int total;
    data = malloc(ARRAYSIZE * sizeof(int));
    for (int i = 0; i < ARRAYSIZE; i++)
      data[i] = i;

    for (count = 0; count < size; count++) {
      MPI_Send(const void *buf, int count, MPI_INT, int dest, int tag,
               MPI_COMM_WORLD);
    }

    while (count) {
      MPI_Recv(&tempInt, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
               MPI_COMM_WORLD, &status);
      total += tempInt;
      count--;
    }
  }

  if (rank) {
    int elemCount = ;
    int bufferSize = ;
    data = malloc();
    count = myDepth;

    MPI_Recv(void *buf, int count, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
             MPI_COMM_WORLD, &status);

    while (count < maxDepth) {
      // send data till u have counted to the maxDepth
      elemCount /= 2;
      // figure out how to encode ranks in tag, potentially use each tens place
      // to retrace steps back up, essentially shifting by one decimal each time
      MPI_Send(const void *buf, elemCount, MPI_INT, int dest, int tag,
               MPI_COMM_WORLD);
      data = realloc(data, elemCount * sizeof(int));
      count++;
    }

    int localSum = add(data, elemCount);

    while (count > myDepth) {
      MPI_Recv(&tempInt, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
               MPI_COMM_WORLD, &status);
      localSum += tempInt;
      count--;
    }
  }

  if (!rank) {
    int verify = add(data, ARRAYSIZE);
  }

  MPI_Finalize();
  return 0;
}
