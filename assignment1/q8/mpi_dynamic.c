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
  int TERM = 0;
  int WORK = 1;
  int RESULT = 2;

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
    int out = 0;

    for (reqRank = 1; reqRank < size; reqRank++) {
      MPI_Send(&work_row, 1, MPI_INT, reqRank, WORK, MPI_COMM_WORLD);
      work_row++;
      out++;
    }

    do {
      MPI_Recv(tempBuf, Y_RESN, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
               MPI_COMM_WORLD, &status);
      out--;

      memcpy(&k[status.MPI_TAG * Y_RESN], tempBuf, Y_RESN * sizeof(int));
      reqRank = status.MPI_SOURCE;

      if (work_row < X_RESN) {
        MPI_Send(&work_row, 1, MPI_INT, reqRank, WORK, MPI_COMM_WORLD);
        out++;
        work_row++;
      }
    } while (status.MPI_TAG < X_RESN && out != 0);

    for (reqRank = 1; reqRank < size; reqRank++) {
      MPI_Send(&work_row, 1, MPI_INT, reqRank, TERM, MPI_COMM_WORLD);
    }
  }

  if (rank) {
    MPI_Recv(&work_row, 1, MPI_INT, 0, WORK, MPI_COMM_WORLD, &status);
    while (status.MPI_TAG) {

      for (i = 0; i < Y_RESN; i++) {
        z.real = z.imag = 0.0;
        c.real = ((float)i - 400.0) / 200.0;
        c.imag = ((float)work_row - 400.0) / 200.0;
        k[i] = 0;

        do { /* iterate for pixel color */
          temp = z.real * z.real - z.imag * z.imag + c.real;
          z.imag = 2.0 * z.real * z.imag + c.imag;
          z.real = temp;
          lengthsq = z.real * z.real + z.imag * z.imag;
          k[i]++;
        } while (lengthsq < 4.0 && k[i] < 100);
      }

      MPI_Send(k, Y_RESN, MPI_INT, 0, work_row, MPI_COMM_WORLD);
      MPI_Recv(&work_row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
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
