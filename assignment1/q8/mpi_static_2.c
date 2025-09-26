#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define X_RESN 800 /* x resolution */
#define Y_RESN 800 /* y resolution */

typedef struct complextype {
  float real, imag;
} Compl;

int main(int argc, char *argv[]) {
  int rank, size;

  int i, j, *k;
  Compl z, c;
  float lengthsq, temp;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int rowCount = X_RESN / size;

  if (!rank)
    k = malloc(X_RESN * Y_RESN * sizeof(int));

  if (rank)
    k = malloc(rowCount * Y_RESN * sizeof(int));

  int sendCount = (X_RESN * Y_RESN) / size;

  for (i = rank * rowCount; i < rowCount * (rank + 1); i++)
    for (j = 0; j < Y_RESN; j++) {

      z.real = z.imag = 0.0;
      c.real = ((float)j - 400.0) / 200.0;
      c.imag = ((float)i - 400.0) / 200.0;
      k[(i - rank * rowCount) * Y_RESN + j] = 0;

      do { /* iterate for pixel color */
        temp = z.real * z.real - z.imag * z.imag + c.real;
        z.imag = 2.0 * z.real * z.imag + c.imag;
        z.real = temp;
        lengthsq = z.real * z.real + z.imag * z.imag;
        k[(i - rank * rowCount) * Y_RESN + j]++;
      } while (lengthsq < 4.0 && k[(i - rank * rowCount) * Y_RESN + j] < 100);
    }

  MPI_Gather(k, rowCount * Y_RESN, MPI_INT, k, rowCount * Y_RESN, MPI_INT, 0,
             MPI_COMM_WORLD);

  if (rank == 0) {
    /* Save array of points out to file */
    FILE *output;

    output = fopen("mandelbrot_data_static.txt", "w+");
    if (output == NULL) {
      printf("Error opening file\n");
      return -1;
    }

    for (i = 0; i < X_RESN; i++)
      for (j = 0; j < Y_RESN; j++) {
        fprintf(output, "%d\n", k[i * Y_RESN + j]);
      }

    fclose(output);
  }

  free(k);

  MPI_Finalize();
  return 0;
}
