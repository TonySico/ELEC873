#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int rank, size;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int dSRec = size - rank;
  int dSSend = dSRec - 1;
  char data[dSRec];

  int position = 0;
  char sendBuf[dSSend];
  int recData;

  if (rank == 0) {
    for (int i = 0; i < size; i++)
      data[i] = (char)i;

    MPI_Pack(data, dSSend, MPI_CHAR, &sendBuf, size, &position, MPI_COMM_WORLD);
    MPI_Send(&sendBuf, position, MPI_PACKED, (rank + 1) % size, 0,
             MPI_COMM_WORLD);
    printf("Process %d kept %d and sent the rest to %d\n", rank, data[0],
           (rank + 1) % size);
  } else {
    if (MPI_Recv(&data, dSRec, MPI_CHAR, (rank - 1 + size) % size, 0,
                 MPI_COMM_WORLD, &status) == MPI_SUCCESS) {
      MPI_Unpack(&data, dSRec, &position, &recData, 1, MPI_CHAR,
                 MPI_COMM_WORLD);

      MPI_Unpack(&data, dSRec, &position, &sendBuf, dSSend, MPI_CHAR,
                 MPI_COMM_WORLD);

      MPI_Pack(&data, dSSend, MPI_CHAR, &sendBuf, dSSend, &position,
               MPI_COMM_WORLD);

      if (rank != size - 1)
        MPI_Send(&sendBuf, position - 1, MPI_PACKED, (rank + 1) % size, 0,
                 MPI_COMM_WORLD);

      printf("Process %d recieved and kept %d from process %d and sent the "
             "rest to %d\n",
             rank, recData, status.MPI_SOURCE, rank + 1);
    }
  }

  MPI_Finalize();
  return 0;
}
