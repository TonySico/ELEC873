#include <math.h>
#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int size, rank, q;
  int dim_sizes[3], wrap_around[3], coordinates[3], free_coords[3];
  int my_grid_rank, grid_rank, row_test;
  int reorder = 0;
  MPI_Comm row_comm, grid_comm;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  q = (int)cbrt((double)size);

  dim_sizes[0] = dim_sizes[1] = dim_sizes[2] = q;
  wrap_around[0] = wrap_around[1] = wrap_around[2] = 1;

  MPI_Cart_create(MPI_COMM_WORLD, 3, dim_sizes, wrap_around, reorder,
                  &grid_comm);
  MPI_Comm_rank(grid_comm, &my_grid_rank);
  MPI_Cart_coords(grid_comm, my_grid_rank, 3, coordinates);
  MPI_Cart_rank(grid_comm, coordinates, &grid_rank);

  printf(
      "Process %d my_grid_rank = %d, coords  = (%d, %d, %d), grid_rank = % d\n",
      rank, my_grid_rank, coordinates[0], coordinates[1], coordinates[2],
      grid_rank);

  free_coords[0] = 0;
  free_coords[1] = 1;
  free_coords[2] = 0;

  MPI_Cart_sub(grid_comm, free_coords, &row_comm);
  if (coordinates[1] == 0)
    row_test = coordinates[0];
  else
    row_test = -1;

  MPI_Bcast(&row_test, 1, MPI_INT, 0, row_comm);
  printf("Process %d coords = (%d, %d, %d), row_test = %d\n", rank,
         coordinates[0], coordinates[1], coordinates[2], row_test);

  MPI_Finalize();
  return 0;
}
