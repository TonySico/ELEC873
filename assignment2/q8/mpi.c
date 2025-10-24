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
  unsigned long long g_end, rtt_end, rtt1 = 1000000, rttn = 1;

  // start of rtt1 calc for use in calculating g(0)
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
  // end rtt1 calc

  int WORK = 1, STOP = 0;
  int flag = WORK;

  // Loop for benchmarking

  // printf("rtt1 = %llu, rttn = %llu\n", rtt1, rttn);
  while (epsilon(g[1], g[0]) > 1 && flag && rtt1 > (0.01 * rttn)) {

    // Set prev g value to current for new calculation
    if (!rank) {
      g[0] = g[1];
    }

    if (!rank) {
      g_rttn_start = get_time();
    }

    for (int i = 0; i < n_runs; i++) {
      if (!rank) {
        MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, WORK, MPI_COMM_WORLD);
      }

      if (rank) {
        MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                 &status);
        flag = status.MPI_TAG;
        if (!flag)
          break;
      }
    }

    if (!rank) {
      g_end = get_time();

      MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 1, MPI_ANY_TAG, MPI_COMM_WORLD,
               &status);
      rtt_end = get_time();

      g[1] =
          (g_end - g_rttn_start - timer_overhead) / (unsigned long long)n_runs;
      rttn = (rtt_end - g_rttn_start - timer_overhead);
      printf("gap_0_new = %llu, gap_0_old = %llu, rtt1 = %llu, rttn = %f, \n",
             g[1], g[0], rtt1, epsilon(g[1], g[0]) * rttn);
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
    printf("rtt1 = %.8f, g0 = %.8f, rttn = %.8f, determined after %d runs\n",
           (double)rtt1, gapzero, (double)rttn, n_runs);
  }

  MPI_Finalize();

  return 0;
}
