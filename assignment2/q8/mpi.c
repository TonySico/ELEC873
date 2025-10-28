// Anthony Sicoie (20214793)

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int rank;
unsigned long long timer_overhead;

#define ZERO_DATA_COUNT 0
#define RANK_ZERO 0
#define RANK_ONE 1

#define STOP 0
#define WORK 1
#define WORK_OR 2
#define SYNC 3
#define READY_OR 4
#define ZERO_CHECK 5

#define K_M 18
#define m(x) (1ULL << x)

#define AVG_RUNS 1024
#define RTT1_AVG 1024

float epsilon(double new, double old) {
  float e = fabs((new - old) / old * 100.0f);
  return (e);
}

typedef struct {
  int m;
  unsigned long long g_m;
  unsigned long long o_r;
  unsigned long long o_s;
  unsigned long long rtt_m;

} result;

unsigned long long get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * (unsigned long long)1e9 + ts.tv_nsec;
}

void getResult(result *R, unsigned long long g0, unsigned long long rtt1) {
  unsigned long long oS_rtt_start, rtt_end,
      rtt_temp = 0, o_r_start, o_s_temp = 0, o_r_end, o_s_end, o_r_temp = 0;

  char *data = malloc(R->m);
  memset(data, 'a', R->m);

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Status status;
  struct timespec rttm;

  unsigned long long r[2] = {10.0, 1.0};
  int flag = WORK;
  int nruns = 1;

  while (epsilon(r[1], r[0]) > 1 && flag) {

    if (!rank) {
      r[0] = r[1];
    }

    for (int i = 0; i < nruns && flag; i++) {
      if (!rank) {
        MPI_Recv(data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, SYNC,
                 MPI_COMM_WORLD, &status); // Synch

        oS_rtt_start = get_time();
        // printf("failing here?1 on R.m = %d\n", R->m);
        MPI_Send(&data, R->m, MPI_CHAR, RANK_ONE, WORK, MPI_COMM_WORLD);
        o_s_end = get_time();
        MPI_Recv(data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, ZERO_CHECK,
                 MPI_COMM_WORLD, &status);

        rtt_end = get_time();

        o_s_temp += o_s_end - oS_rtt_start - timer_overhead;
        rtt_temp += rtt_end - oS_rtt_start - 2 * timer_overhead;
      }

      if (rank) {
        MPI_Send(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, SYNC,
                 MPI_COMM_WORLD); // Synch
        MPI_Recv(data, R->m, MPI_CHAR, RANK_ZERO, MPI_ANY_TAG, MPI_COMM_WORLD,
                 &status);
        flag = status.MPI_TAG;

        if (flag)
          MPI_Send(data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, ZERO_CHECK,
                   MPI_COMM_WORLD);
      }
    }
    if (nruns < 60 && R->m <= m(10) && flag)
      nruns += 2;
    else if (nruns < 15 && R->m > m(10) && flag)
      nruns += 2;
    else {
      if (!rank)
        flag = STOP;
    }

    if (!rank) {
      r[1] = rtt_temp / nruns;
    }
  }

  if (!rank) {
    MPI_Send(data, R->m, MPI_CHAR, RANK_ONE, STOP, MPI_COMM_WORLD);
    R->rtt_m = r[1];
    R->o_s = o_s_temp / nruns;
    R->g_m = R->rtt_m - rtt1 + g0;

    unsigned long long temp = (unsigned long long)(R->rtt_m * 1.1);

    rttm.tv_sec = temp / 1000000000ULL;
    rttm.tv_nsec = temp % 1000000000ULL;
  }

  printf(
      "Rank %d is entering looping 2 and running nrun = %d times, R.m = %d\n",
      rank, nruns, R->m);

  for (int i = 0; i < nruns; i++) {
    if (!rank) {
      MPI_Send(data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, READY_OR,
               MPI_COMM_WORLD); // Synch

      // Sleep for just slightly longer than rttm as per paper
      nanosleep(&rttm, NULL);

      o_r_start = get_time();
      MPI_Recv(data, R->m, MPI_CHAR, RANK_ONE, WORK_OR, MPI_COMM_WORLD,
               &status);
      o_r_end = get_time();

      o_r_temp += o_r_end - o_r_start - timer_overhead;
    }

    if (rank) {
      MPI_Recv(data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, READY_OR,
               MPI_COMM_WORLD, &status); // Synch
      MPI_Send(data, R->m, MPI_CHAR, RANK_ZERO, WORK_OR, MPI_COMM_WORLD);
    }
  }

  if (!rank) {
    R->o_r = o_r_temp / nruns;
  }

  free(data);

} // getResult()

int main(int argc, char *argv[]) {
  int size, n_runs = 10;
  MPI_Status status;
  double total_time = 0.0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  char data = '1';

  unsigned long long g[2] = {10.0, 1.0};
  int flag = WORK;

  unsigned long long offset = get_time();
  unsigned long long g_rttn_start = get_time();
  timer_overhead = g_rttn_start - offset;
  unsigned long long g_end, rtt_end, rtt1_total = 0, rtt_1 = 100, rttn = 0;

  // run the 0 message size ping pong 1000 times to stabalize rtt1 value
  for (int i = 0; i < RTT1_AVG; i++) {
    // start of rtt1 calc for use in calculating g(0)
    if (!rank) {
      g_rttn_start = get_time();
      MPI_Send(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, WORK,
               MPI_COMM_WORLD);
      MPI_Recv(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, WORK, MPI_COMM_WORLD,
               &status);
      rtt_end = get_time();

      // discount the first 200 runs to ensure "warmup"
      if (i > 199) {
        rtt_1 = rtt_end - g_rttn_start - timer_overhead;
        rtt1_total += rtt_1;
      }
    }

    if (rank) {
      MPI_Recv(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, WORK,
               MPI_COMM_WORLD, &status);
      MPI_Send(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, WORK,
               MPI_COMM_WORLD);
    }
  }

  if (!rank) {
    rtt_1 = rtt1_total / (RTT1_AVG - 200);
  }
  // end rtt1 calc

  // Loop for calculating g0 and RTTn
  while (epsilon(g[1], g[0]) > 1 && flag && rtt_1 > (0.01 * rttn)) {

    // Set prev g value to current for new calculation
    if (!rank) {
      g[0] = g[1];
      g_rttn_start = get_time();
    }

    for (int i = 0; i < n_runs && flag; i++) {
      if (!rank) {
        MPI_Send(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, WORK,
                 MPI_COMM_WORLD);
      }

      if (rank) {
        MPI_Recv(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, MPI_ANY_TAG,
                 MPI_COMM_WORLD, &status);
        flag = status.MPI_TAG;
      }
    }

    if (!rank) {
      g_end = get_time();

      MPI_Recv(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, MPI_ANY_TAG,
               MPI_COMM_WORLD, &status);
      rtt_end = get_time();

      g[1] = (g_end - g_rttn_start - timer_overhead) / n_runs;
      rttn = (rtt_end - g_rttn_start - 2 * timer_overhead);
    }

    if (rank) {
      MPI_Send(&data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, WORK,
               MPI_COMM_WORLD);
    }

    n_runs *= 2;
    // ensures that if n goes too high, it doesn't waste server resources and
    // just fails out
    if (n_runs > pow(2, 20)) {
      printf("error, n_runs was too high\n");
      break;
    }
  }

  unsigned long long g_0 = g[1];

  // Tells the mirror to stop waiting on a recv
  if (!rank) {
    MPI_Send(&data, ZERO_DATA_COUNT, MPI_CHAR, 1, STOP, MPI_COMM_WORLD);
  }
  // end of g0 and rttn calculation

  double L = (double)rtt_1 / 2 - g_0;

  // Print values for part 1
  if (!rank) {
    printf("rtt_1 = %llu, g0 = %llu, rttn = %llu, L = %f \n", rtt_1, g_0, rttn,
           L);
  }

  // Start of part 2/3, calculating Os(m), Or(m), L, g(m) and RTT(m)
  result R;

  for (int k = 0; k <= K_M; k++) {
    R.m = m(k);
    getResult(&R, g_0, rtt_1);
    MPI_Barrier(MPI_COMM_WORLD);

    if (!rank) {
      printf("m = %d, g_m = %llu, o_s = %llu, o_r = %llu, rtt_m = %llu \n", R.m,
             R.g_m, R.o_s, R.o_r, R.rtt_m);
    }
  }

  // Print values for part 2 and 3

  MPI_Finalize();

  return 0;
}
