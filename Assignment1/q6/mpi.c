// Anthony Sicoie (20214793)

#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int rank, size;

  // For use in output printing to see who data was recieved from
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int elemCount = size - rank;
  char data[elemCount];

  int position = 0;
  char sendBuf[elemCount];

  if (rank == 0) {
    for (int i = 0; i < size; i++) {
      data[i] = (char)(i + 1);
    }

    // pack everything rank 0 won't hold
    MPI_Pack(&data[1], elemCount, MPI_CHAR, &sendBuf, elemCount * sizeof(char),
             &position, MPI_COMM_WORLD);

    // send based on the positoin variable minus 1 to only send the remaining
    // data that rank one no longer needs
    MPI_Send(&sendBuf, position - 1, MPI_PACKED, 1, 0, MPI_COMM_WORLD);

    printf("Process %d recieved %d items, kept 1 (data = %d), and sent the "
           "remaining %d items to %d\n",
           rank, elemCount, data[0], position - 1, (rank + 1) % size);
  } else {
    if (MPI_Recv(&data, elemCount, MPI_PACKED, (rank - 1 + size) % size, 0,
                 MPI_COMM_WORLD, &status) == MPI_SUCCESS) {

      // unpack the received data into the pre allocated data buffer
      MPI_Unpack(data, elemCount, &position, &sendBuf, elemCount, MPI_CHAR,
                 MPI_COMM_WORLD);

      // reset the positoin var for reuse in pack
      position = 0;

      // the last rank doesn't need to pack anything else so they just print
      // that they are the last rank
      if (rank != size - 1) {
        // Pack from the first element on to only send what isn't being kept in
        // data buff
        MPI_Pack(&data[1], elemCount - 1, MPI_CHAR, sendBuf,
                 (elemCount - 1) * sizeof(char), &position, MPI_COMM_WORLD);

        // Send the packed data to the next rank
        MPI_Send(&sendBuf, position, MPI_PACKED, rank + 1, 0, MPI_COMM_WORLD);
        printf("Process %d recieved %d items from process %d, kept 1 (data = "
               "%d), and sent the remaining %d items to process %d\n",
               rank, elemCount, status.MPI_SOURCE, data[0], position, rank + 1);
      } else {
        printf("Process %d recieved %d items from process %d, kept 1 (data = "
               "%d), and is the last process to recieve data\n",
               rank, elemCount, status.MPI_SOURCE, data[0]);
      }
    }
  }

  MPI_Finalize();
  return 0;
}
