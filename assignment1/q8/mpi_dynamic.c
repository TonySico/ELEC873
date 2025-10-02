// Anthony Sicoie (20214793)

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X_RESN 800 /* x resolution */
#define Y_RESN 800 /* y resolution */
#define N_RUNS 100

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

  double total_time = 0.0;

  for (int run = 0; run < N_RUNS; run++) {
    double tmp = MPI_Wtime();
    double start = MPI_Wtime();

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
        // send initial rows
        MPI_Send(&work_row, 1, MPI_INT, reqRank, WORK, MPI_COMM_WORLD);
        // Increment work row as well as count number of processes "out" doing
        // work
        work_row++;
        out++;
      }

      do {
        // recieve new calculated row
        MPI_Recv(tempBuf, Y_RESN, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
                 MPI_COMM_WORLD, &status);
        // decrement processes out for work
        out--;

        // Copy into the array using the tag sent back by the process to figure
        // out whcih row it worked on
        memcpy(&k[status.MPI_TAG * Y_RESN], tempBuf, Y_RESN * sizeof(int));
        // Store the rank that came back to know who to send work to if any is
        // left
        reqRank = status.MPI_SOURCE;

        // if there is still work left to dom send it out to the process that
        // just returned
        if (work_row < X_RESN) {
          MPI_Send(&work_row, 1, MPI_INT, reqRank, WORK, MPI_COMM_WORLD);
          out++;
          work_row++;
        } else {
          // terminate the processes as they return
          MPI_Send(&work_row, 1, MPI_INT, reqRank, TERM, MPI_COMM_WORLD);
        }
      } while (status.MPI_TAG < X_RESN && out != 0);
    }

    if (rank) {
      // Receive the working row and compute the proper values
      MPI_Recv(&work_row, 1, MPI_INT, 0, WORK, MPI_COMM_WORLD, &status);
      // Work until terminate tag is sent back
      while (status.MPI_TAG) {

        for (i = 0; i < Y_RESN; i++) {
          z.real = z.imag = 0.0;
          c.real = ((float)i - (Y_RESN / 2.0)) / (Y_RESN / 4.0);
          c.imag = ((float)work_row - (X_RESN / 2.0)) / (X_RESN / 4.0);
          k[i] = 0;

          do { /* iterate for pixel color */
            temp = z.real * z.real - z.imag * z.imag + c.real;
            z.imag = 2.0 * z.real * z.imag + c.imag;
            z.real = temp;
            lengthsq = z.real * z.real + z.imag * z.imag;
            k[i]++;
          } while (lengthsq < 4.0 && k[i] < 100);
        }

        // Encode the worked on row in the tag and send the correct values back
        MPI_Send(k, Y_RESN, MPI_INT, 0, work_row, MPI_COMM_WORLD);
        // Recieve the next row to work on or a termination message
        MPI_Recv(&work_row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                 &status);
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

    double end = MPI_Wtime();
    double timer_overhead = start - tmp;
    total_time += (end - start - timer_overhead);
  }

  double avg_time = total_time / N_RUNS;

  if (rank == 0) {

    FILE *f = fopen("dynamic.csv", "a");
    if (f == NULL) {
      fprintf(stderr, "Error opening results.csv\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    fprintf(f, "%d,%.6f\n", size, avg_time);
    fclose(f);

    printf("Processes=%d AvgTime=%.6f sec\n", size, avg_time);
  }

  MPI_Finalize();
  return 0;
}
