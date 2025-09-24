#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int rank, size, recData;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    MPI_Send(&rank, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    if (MPI_Recv(&recData, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, &status) ==
        MPI_SUCCESS) {
      printf("Process %d recieved the data %d from process %d\n", rank, recData,
             status.MPI_SOURCE);
    }
  } else if (MPI_Recv(&recData, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD,
                      &status) == MPI_SUCCESS) {

    printf("Process %d recieved the data %d from process %d\n", rank, recData,
           status.MPI_SOURCE);

    if (rank == size - 1)
      MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    else
      MPI_Send(&rank, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}
