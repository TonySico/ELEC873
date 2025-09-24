#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int ierror, rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  ierror = MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (ierror != MPI_SUCCESS)
    MPI_Abort(MPI_COMM_WORLD, ierror);
  printf("Hello World, I am process %d out of %d processes.\n", rank, size);

  MPI_Finalize();
  return 0;
}
