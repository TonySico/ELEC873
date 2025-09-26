#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  MPI_Status status;

  int startRow = 0;

  if (!rank) {
    k = malloc(X_RESN * Y_RESN * sizeof(int));
  }

  if (!rank) {
    int count = 0;
    int rowCount; // figure out this math

    for (int sendRank = 1; sendRank < size; sendRank++) {
      int offset = (sendRank < (X_RESN % (size - 1))) ? 1 : 0;
      rowCount = (X_RESN / (size - 1)) + offset;

      MPI_Send(&startRow, 1, MPI_INT, sendRank, rowCount, MPI_COMM_WORLD);

      count++;
      startRow += rowCount;
    }

    while (count != 0) {
      int elemCount = (X_RESN / (size - 1)) + 1;
      int bufferSize = elemCount * Y_RESN * sizeof(int);

      int *tempBuf = malloc(bufferSize);

      MPI_Recv(tempBuf, elemCount * Y_RESN, MPI_INT, MPI_ANY_SOURCE,
               MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      count--;

      int offset = (status.MPI_SOURCE < (X_RESN % (size - 1))) ? 0 : 1;

      // figure out whether or not the new buffer needs to be offset
      memcpy(&k[status.MPI_TAG * Y_RESN], tempBuf,
             (elemCount - offset) * sizeof(int));
      free(tempBuf);
    }
  }

  if (rank) {
    MPI_Recv(&startRow, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    int rowCount = status.MPI_TAG;

    k = malloc(rowCount * Y_RESN * sizeof(int));

    for (i = 0; i < rowCount; i++)
      for (j = 0; j < Y_RESN; j++) {

        z.real = z.imag = 0.0;
        c.real = ((float)j - 400.0) / 200.0;
        c.imag = ((float)i - 400.0) / 200.0;
        k[i * Y_RESN + j] = 0;

        do { /* iterate for pixel color */
          temp = z.real * z.real - z.imag * z.imag + c.real;
          z.imag = 2.0 * z.real * z.imag + c.imag;
          z.real = temp;
          lengthsq = z.real * z.real + z.imag * z.imag;
          k[i * Y_RESN + j]++;
        } while (lengthsq < 4.0 && k[i * Y_RESN + j] < 100);
      }
    MPI_Send(k, rowCount * Y_RESN, MPI_INT, 0, startRow, MPI_COMM_WORLD);
    free(k);
  }

  if (rank == 0) {
    /* Save array of points out to file */
    FILE *output;

    output = fopen("mandelbrot_data_dynamic.txt", "w+");
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

  MPI_Finalize();
  return 0;
}
