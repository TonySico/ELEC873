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
  // Determine the depth in the binary tree each process has
  int myDepth = depth(rank, size);
  // Determine the largest depth possible given splits of 2
  int maxDepth = ((int)ceil(log((double)size) / log((double)2)));
  // Calculate how many elements a process should have at their depth
  int elemCount = ARRAYSIZE / (pow(2, myDepth));
  int bufferSize = elemCount * sizeof(int);
  // allocate this data buffer for each process
  data = malloc(bufferSize);
  // init counter value
  count = myDepth;

  // Fill test and regular array
  int testData[1000];
  if (!rank) {
    for (int i = 0; i < ARRAYSIZE; i++) {
      data[i] = i;
      testData[i] = i;
    }
    // get rank 0s first destination
    dest = size / 2;
  }

  // wait until data is recieved
  if (rank) {
    MPI_Recv(data, elemCount, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
             MPI_COMM_WORLD, &status);

    returnSource = status.MPI_SOURCE;
    dest = rank + ((rank - status.MPI_SOURCE) / 2);
  }

  // if a processes count is less than the max depth, it hasn't sent all the
  // data it needs to yet
  while (count < maxDepth) {
    // send the right half of the array
    elemCount /= 2;
    MPI_Send(&data[elemCount], elemCount, MPI_INT, dest, 0, MPI_COMM_WORLD);
    // equation to figure out next rank to send to
    dest = rank + ((dest - rank) / 2);
    // resize current array to only have data you are supposed to do
    data = realloc(data, elemCount * sizeof(int));
    count++;
  }

  // Calculate local totals
  total = add(data, elemCount);

  while (count > myDepth) {
    // recieve totals from children processes
    MPI_Recv(&tempInt, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);
    total += tempInt;
    count--;
  }

  // once you break out from the loop above, it means you are now a child to
  // someone else and must send your data up the tree
  if (rank) {
    MPI_Send(&total, 1, MPI_INT, returnSource, 0, MPI_COMM_WORLD);
  }

  if (!rank) {
    int verify = add(testData, ARRAYSIZE);
    printf("verified total = %d, calculated total = %d\n", verify, total);
  }

  MPI_Finalize();
  return 0;
}
