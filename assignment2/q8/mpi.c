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

int epsilon(int new, int old) {
  return (ceil(fabs(((((float)new - (float)old) / (float)old) / 100.0))));
}

int main(int argc, char *argv[]) {
  int rank, size, n_runs = 10;
  MPI_Status status;
  double total_time = 0.0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int DATA_COUNT = atoi(argv[2]);

  char *data;
  data = malloc(DATA_COUNT * sizeof(char));

  float g[2] = {100.0, 1.0};

  double offset = MPI_Wtime();
  double g_start = MPI_Wtime();
  double timer_overhead = g_start - offset;
  double g_end, rtt_start, rtt_end;

  int WORK = 1, TAG = WORK, STOP = 0;

  // Loop for benchmarking
  while (epsilon(g[0], g[1]) > 1 && TAG) {

    // Set prev g value to current for new calculation
    g[0] = g[1];

    for (int i; i < n_runs; i++ && TAG) {
      if (!rank) {
        g_start = MPI_Wtime();
        MPI_Send(data, DATA_COUNT, MPI_CHAR, 1, WORK, MPI_COMM_WORLD);
      }

      if (rank) {
        MPI_Recv(data, DATA_COUNT, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD,
                 &status);
        TAG = status.MPI_TAG;
      }
    }

    if (!rank) {
      g_end = MPI_Wtime();
      g[1] = (g_end - g_start - timer_overhead) / n_runs;
      MPI_Recv(data, DATA_COUNT, MPI_CHAR, 1, MPI_ANY_TAG, MPI_COMM_WORLD,
               &status);
    }

    if (rank) {
      MPI_Send(data, DATA_COUNT, MPI_CHAR, 0, WORK, MPI_COMM_WORLD);
    }

    n_runs *= 2;
  }

  // Tells the mirror to stop waiting on a recv
  if (!rank) {
    MPI_Send(data, DATA_COUNT, MPI_CHAR, 1, STOP, MPI_COMM_WORLD);
  }

  if (!rank) {
    printf("gap = %.2f", g[1]);
  }

  MPI_Finalize();

  return 0;
}
