#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int ierror, rank, size;

  static long num_steps = 100000;
  double step = 1.0 / (double)num_steps;

  double x, pi = 0.0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  ierror = MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (ierror != MPI_SUCCESS)
    MPI_Abort(MPI_COMM_WORLD, ierror);

  double local_sum;
  int i;

  for (i = rank; i < num_steps; i += size) {
    x = (i + 0.5) * step;
    local_sum += 4.0 / (1.0 + x * x);
  }

  MPI_Reduce(&local_sum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    printf("Pi = %f\n", pi * step);
  }

  MPI_Finalize();
  return 0;
}
