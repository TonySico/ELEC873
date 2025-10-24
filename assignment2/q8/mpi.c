// Anthony Sicoie (20214793)

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int rank;

#define RTT1_AVG 1024

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
  unsigned long long g_end, rtt_end, rtt1_total = 0, rtt_1 = 100, rttn = 0;

  // run the 0 message size ping pong 1000 times to stabalize rtt1 value
  for (int i = 0; i < RTT1_AVG; i++) {
    // start of rtt1 calc for use in calculating g(0)
    if (!rank) {
      g_rttn_start = get_time();
      MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
      MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
      rtt_end = get_time();
      rtt_1 = rtt_end - g_rttn_start - timer_overhead;
      rtt1_total += rtt_1;
    }

    if (rank) {
      MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
      MPI_Send(&data, DATA_COUNT, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
  }

  if (!rank) {
    rtt_1 = rtt1_total / RTT1_AVG;
  }
  // end rtt1 calc

  // Tags for send and recieve
  int WORK = 1, STOP = 0;
  int flag = WORK;

  // Loop for calculating g0 and RTTn
  while (epsilon(g[1], g[0]) > 1 && flag && rtt_1 > (0.01 * rttn)) {

    // Set prev g value to current for new calculation
    if (!rank) {
      g[0] = g[1];
      g_rttn_start = get_time();
    }

    for (int i = 0; i < n_runs && flag; i++) {
      if (!rank) {
        MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, WORK, MPI_COMM_WORLD);
      }

      if (rank) {
        MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                 &status);
        flag = status.MPI_TAG;
      }
    }

    if (!rank) {
      g_end = get_time();

      MPI_Recv(&data, DATA_COUNT, MPI_CHAR, 1, MPI_ANY_TAG, MPI_COMM_WORLD,
               &status);
      rtt_end = get_time();

      g[1] = (g_end - g_rttn_start - timer_overhead) / n_runs;
      rttn = (rtt_end - g_rttn_start - timer_overhead);
    }

    if (rank) {
      MPI_Send(&data, DATA_COUNT, MPI_CHAR, 0, WORK, MPI_COMM_WORLD);
    }

    n_runs *= 2;
    // ensures that if n goes too high, it doesn't waste server resources and
    // just fails out
    if (n_runs > pow(2, 20)) {
      printf("error, n_runs was too high\n");
      break;
    }
  }

  double g_0 = g[1];

  // Tells the mirror to stop waiting on a recv
  if (!rank) {
    MPI_Send(&data, DATA_COUNT, MPI_CHAR, 1, STOP, MPI_COMM_WORLD);
  }

  // debug

  // if (!rank) {
  //   printf("rtt_1 = %.8f, g0 = %.8f, rttn = %.8f, L = %f \n", (double)rtt_1,
  //          g_0, (double)rttn, L);
  // }

  // end of g0 and rttn calculation
  //
  //
  //
  //
  //
  // Start of part 2, calculating Os(m), Or(m), L, g(m) and RTT(m)

  double L = (double)rtt_1 / 2 - g_0;

  MPI_Finalize();

  return 0;
}
