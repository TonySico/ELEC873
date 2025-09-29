// Anthony Sicoie (20214793)

#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int rank, size;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int dataSize = size - rank;
  char data[dataSize];

  int position = 0;
  char sendBuf[dataSize];

  if (rank == 0) {
    for (int i = 0; i < size; i++) {
      data[i] = (char)(i + 1);
    }

    MPI_Pack(&data[1], dataSize, MPI_CHAR, &sendBuf, dataSize * sizeof(char),
             &position, MPI_COMM_WORLD);
    MPI_Send(&sendBuf, position - 1, MPI_PACKED, (rank + 1) % size, 0,
             MPI_COMM_WORLD);
    printf("Process %d recieved %d items, kept 1 (data = %d), and sent the "
           "remaining %d items to %d\n",
           rank, dataSize, data[0], position - 1, (rank + 1) % size);
  } else {
    if (MPI_Recv(&data, dataSize, MPI_PACKED, (rank - 1 + size) % size, 0,
                 MPI_COMM_WORLD, &status) == MPI_SUCCESS) {
      MPI_Unpack(data, dataSize, &position, &sendBuf, dataSize, MPI_CHAR,
                 MPI_COMM_WORLD);

      position = 0;

      if (rank != size - 1) {
        MPI_Pack(&data[1], dataSize - 1, MPI_CHAR, sendBuf,
                 (dataSize - 1) * sizeof(char), &position, MPI_COMM_WORLD);

        MPI_Send(&sendBuf, position, MPI_PACKED, (rank + 1) % size, 0,
                 MPI_COMM_WORLD);
        printf("Process %d recieved %d items from process %d, kept 1 (data = "
               "%d), and sent the remaining %d items to process %d\n",
               rank, dataSize, status.MPI_SOURCE, data[0], position, rank + 1);
      } else {
        printf("Process %d recieved %d items from process %d, kept 1 (data = "
               "%d), and is the last process to recieve data\n",
               rank, dataSize, status.MPI_SOURCE, data[0]);
      }
    }
  }

  MPI_Finalize();
  return 0;
}
