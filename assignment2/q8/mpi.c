// Anthony Sicoie (20214793)
//
//
//
// get g(0) until g(0) calculated currently varries less than 1% from prev
//      - done by increasing the number of messages sent each time
//      - Make sure  the size is 0
//      - stop measuring when a single message is recieved
//
//
//

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int rank;

float epsilon(double new, double old) {
  float e = fabs((new - old) / old * 100.0f);
  return (e);
}

double get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char *argv[]) {
  int size, n_runs = 10;
  MPI_Status status;
  double total_time = 0.0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int DATA_COUNT = 0;
  char data = '1';

  float g[2] = {10.0, 1.0};

  double offset = get_time();
  double g_start = get_time();
  double timer_overhead = g_start - offset;
  double g_end, rtt_start, rtt_end;

  int WORK = 1, TAG = WORK, STOP = 0;

  // Loop for benchmarking
  while (epsilon(g[1], g[0]) > 1 && TAG) {

    // Set prev g value to current for new calculation
    if (!rank) {
      g[0] = g[1];
    }

    int i = 0;
    while (i < n_runs && TAG) {
      if (!rank) {
        g_start = get_time();
        MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, WORK, MPI_COMM_WORLD);
      }

      if (rank) {
        MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                 &status);
        TAG = status.MPI_TAG;
      }
      i++;
    }

    if (!rank) {
      g_end = get_time();
      g[1] = (g_end - g_start - timer_overhead) / n_runs;
      MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 1, MPI_ANY_TAG, MPI_COMM_WORLD,
               &status);
    }

    if (rank) {
      MPI_Send(&data, DATA_COUNT, MPI_CHAR, 0, WORK, MPI_COMM_WORLD);
    }

    n_runs *= 2;
  }

  double gapzero = g[1] * 1e9;

  // Tells the mirror to stop waiting on a recv
  if (!rank) {
    MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, STOP, MPI_COMM_WORLD);
  }

  if (!rank) {
    printf("gap = %.6f, determined after %d runs", gapzero, n_runs);
  }

  MPI_Finalize();

  return 0;
}
