#include <math.h>
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

  int i, j, *k, *tempBuf;
  Compl z, c;
  float lengthsq, temp;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  MPI_Status status;
  int WORK = 1;

  int work_row;

  if (!rank) {
    k = malloc(X_RESN * Y_RESN * sizeof(int));
    tempBuf = malloc(Y_RESN * sizeof(int));
  }

  if (rank)
    k = malloc(Y_RESN * sizeof(int));

  if (!rank) {
    int reqRank;
    work_row = 0;
    int count = 0;
    int rowCount; // figure out this math

    for (reqRank = 1; reqRank < size; reqRank++) {
      MPI_Send(&work_row, 1, MPI_INT, reqRank, 0, MPI_COMM_WORLD);
      count++;
    }

    while (count != 0) {
      MPI_Recv(tempBuf, Y_RESN, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
               MPI_COMM_WORLD, &status);
      count--;
      // figures out whether or not the new buffer needs to be offset
      int offset = (status.MPI_SOURCE < (X_RESN % size)) ? 1 : 0;
      int bufferSize = (floor((double)X_RESN / size) + offset) * sizeof(int);

      memcpy(&k[status.MPI_TAG * Y_RESN], tempBuf, bufferSize);
    }
  }

  if (rank) {
    MPI_Recv(&work_row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

    for (i = rank * work_row; i < work_row * (rank + 1); i++)
      for (j = 0; j < Y_RESN; j++) {

        z.real = z.imag = 0.0;
        c.real = ((float)j - 400.0) / 200.0;
        c.imag = ((float)i - 400.0) / 200.0;
        k[(i - rank * work_row) * Y_RESN + j] = 0;

        do { /* iterate for pixel color */
          temp = z.real * z.real - z.imag * z.imag + c.real;
          z.imag = 2.0 * z.real * z.imag + c.imag;
          z.real = temp;
          lengthsq = z.real * z.real + z.imag * z.imag;
          k[(i - rank * work_row) * Y_RESN + j]++;
        } while (lengthsq < 4.0 && k[(i - rank * work_row) * Y_RESN + j] < 100);
      }

    MPI_Gather(k, work_row * Y_RESN, MPI_INT, k, work_row * Y_RESN, MPI_INT,
               work_row, MPI_COMM_WORLD);
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

  free(k);

  if (!rank)
    free(tempBuf);

  MPI_Finalize();
  return 0;
}
