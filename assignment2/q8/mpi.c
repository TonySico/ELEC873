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

unsigned long long get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * (unsigned long long)1e9 + ts.tv_nsec;
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

  unsigned long long g[2] = {10.0, 1.0};

  unsigned long long offset = get_time();
  unsigned long long g_rttn_start = get_time();
  unsigned long long timer_overhead = g_rttn_start - offset;
  unsigned long long g_end, rtt_end, rtt1;

  if (!rank) {
    g_rttn_start = get_time();
    MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
    rtt_end = get_time();
    rtt1 = rtt_end - g_rttn_start - timer_overhead;
    printf("RTT_1 = %llu (ns)\n", rtt1);
  }

  if (rank) {
    MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Send(&data, DATA_COUNT, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
  }

  int WORK = 1, STOP = 0;
  int flag = WORK;

  // Loop for benchmarking
  while (epsilon(g[1], g[0]) > 1 && flag) {

    // Set prev g value to current for new calculation
    if (!rank) {
      g[0] = g[1];
    }

    int i = 0;
    while (i < n_runs && flag) {
      if (!rank) {
        g_rttn_start = get_time();
        MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, WORK, MPI_COMM_WORLD);
      }

      if (rank) {
        MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                 &status);
        flag = status.MPI_TAG;
      }
      i++;
    }

    if (!rank) {
      g_end = get_time();
      printf("g_end = %llu, g_start = %llu, timer_overhead = %llu, total time "
             "- offset = %llu, n_run = %d\n",
             g_end, g_rttn_start, timer_overhead,
             g_end - g_rttn_start - timer_overhead, n_runs);
      g[1] =
          (g_end - g_rttn_start - timer_overhead) / (unsigned long long)n_runs;
      printf("Gap_0 = %llu\n", g[1]);
      MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 1, MPI_ANY_TAG, MPI_COMM_WORLD,
               &status);
    }

    if (rank) {
      MPI_Send(&data, DATA_COUNT, MPI_CHAR, 0, WORK, MPI_COMM_WORLD);
    }

    n_runs *= 2;
    if (n_runs > pow(2, 20)) {
      printf("error, n_runs was too high\n");
      break;
    }
  }

  double gapzero = g[1];

  // Tells the mirror to stop waiting on a recv
  if (!rank) {
    MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, STOP, MPI_COMM_WORLD);
  }

  if (!rank) {
    printf("gap = %.8f, determined after %d runs", gapzero, n_runs);
  }

  MPI_Finalize();

  return 0;
}
