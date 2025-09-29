// Anthony Sicoie (20214793)

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

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
  int rank, size, *data, count, tempInt, dest, total, returnSource,
      dataCount = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  MPI_Status status;
  int myDepth = depth(rank, size);
  int maxDepth = ((int)ceil(log((double)size) / log((double)2)));
  int elemCount = ARRAYSIZE / (pow(2, myDepth));
  int bufferSize = elemCount * sizeof(int);
  data = malloc(bufferSize);
  count = myDepth;

  int testData[1000];

  if (!rank) {
    for (int i = 0; i < ARRAYSIZE; i++) {
      data[i] = i;
      testData[i] = i;
    }
    dest = size / 2;
  }

  if (rank) {
    MPI_Recv(data, elemCount, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
             MPI_COMM_WORLD, &status);

    returnSource = status.MPI_SOURCE;
    dest = rank + ((rank - status.MPI_SOURCE) / 2);
  }

  while (count < maxDepth) {
    elemCount /= 2;
    MPI_Send(&data[elemCount], elemCount, MPI_INT, dest, 0, MPI_COMM_WORLD);
    dest = rank + ((dest - rank) / 2);
    data = realloc(data, elemCount * sizeof(int));
    count++;
  }

  total = add(data, elemCount);

  while (count > myDepth) {
    MPI_Recv(&tempInt, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);
    total += tempInt;
    count--;
  }

  if (rank) {
    MPI_Send(&total, 1, MPI_INT, returnSource, 0, MPI_COMM_WORLD);
  }

  if (!rank) {
    int verify = add(testData, ARRAYSIZE);
    printf("verified total = %d, calculated total = %d", verify, total);
  }

  MPI_Finalize();
  return 0;
}
