// Anthony Sicoie (20214793)

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int rank;
unsigned long long timer_overhead;
unsigned long long rtt_1;
double g_0;

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
  double g_m_over_m;
  unsigned long long o_r;
  unsigned long long o_s;
  unsigned long long rtt_m;

} result;

typedef struct Node {
  result R;
  struct Node *next;
  struct Node *prev;
} Node;

typedef struct {
  Node *head;
  Node *tail;
  int size;
} List;

void freeList(List *list) {

  Node *current = list->tail;

  while (current != NULL) {
    Node *prev = current->prev;
    free(current);
    current = prev;
  }

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
}

void appendNode(List *list, result newResult) {

  Node *newNode = malloc(sizeof(Node));

  newNode->R = newResult;
  newNode->next = NULL;
  newNode->prev = list->tail;

  if (list->head == NULL) {
    list->head = newNode;
    list->tail = newNode;
  } else {
    list->tail->next = newNode;
    list->tail = newNode;
  }

  list->size++;
}

void insertNode(List *list, result newResult) {
  Node *newNode = malloc(sizeof(Node));
  newNode->R = newResult;

  Node *currentNode = list->head;
  while (currentNode != NULL && currentNode->R.m < newNode->R.m &&
         currentNode->next != NULL) {
    currentNode = currentNode->next;
  }

  if (currentNode == NULL ||
      (currentNode->next == NULL && currentNode->R.m <= newNode->R.m)) {
    appendNode(list, newResult);
    free(newNode);
    return;

  } else {
    newNode->next = currentNode;
    newNode->prev = currentNode->prev;

    if (currentNode->prev != NULL)
      currentNode->prev->next = newNode;
    else
      list->head = newNode;

    currentNode->prev = newNode;
    list->size++;
  }
}

void printList(List *list) {
  Node *currentNode = list->head;
  int node = 0;
  while (currentNode != NULL) {
    printf("m = %d, g_m = %llu, g_m/m = %.8f, o_s = %llu, o_r = %llu, rtt_m = "
           "%llu \n",
           currentNode->R.m, currentNode->R.g_m, currentNode->R.g_m_over_m,
           currentNode->R.o_s, currentNode->R.o_r, currentNode->R.rtt_m);
    currentNode = currentNode->next;
    node++;
  }
}

unsigned long long get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * (unsigned long long)1e9 + ts.tv_nsec;
}

// figures out whether g(m)/m is within the expected range based on the previous
// values and returns a binary 1 or 0
int extrapolateGMOverM(List *list) {
  float e;

  // extrapolate using
  if (!rank) {
    // calculates via y = y1 + (((y2-y1) / (x2-x1)) * (x-x1))
    double y = list->tail->prev->R.g_m_over_m +
               (list->tail->R.m - list->tail->prev->R.m) *
                   ((list->tail->prev->prev->R.g_m_over_m -
                     list->tail->prev->R.g_m_over_m) /
                    (list->tail->prev->prev->R.m - list->tail->prev->R.m));

    // gets the percentage difference between the measured value and y
    e = epsilon(list->tail->R.g_m_over_m, y);

    printf("true = %.8f, guess = %.8f ", list->tail->R.g_m_over_m, y);

    if (!rank)
      printf("e = %f\n", e);

    if (e > 1) {
      return 1;
    } else {
      return 0;
    }
  }

  return 1;
}

void getResult(result *R) {
  unsigned long long oS_rtt_start, rtt_end,
      rtt_temp = 0, o_r_start, o_s_temp = 0, o_r_end, o_s_end, o_r_temp = 0;

  char *data = calloc(R->m, 1);
  memset(data, 'a', R->m);

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
        MPI_Send(data, R->m, MPI_CHAR, RANK_ONE, WORK, MPI_COMM_WORLD);
        o_s_end = get_time();
        MPI_Recv(data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, ZERO_CHECK,
                 MPI_COMM_WORLD, &status);

        rtt_end = get_time();

        o_s_temp += o_s_end - oS_rtt_start - timer_overhead;
        rtt_temp += rtt_end - oS_rtt_start - 2 * timer_overhead;
      }

      if (rank) {
        MPI_Send(data, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, SYNC,
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
    R->g_m = R->rtt_m - rtt_1 + g_0;
    R->g_m_over_m = (double)R->g_m / (double)R->m;

    unsigned long long temp = (unsigned long long)(R->rtt_m * 1.1);

    rttm.tv_sec = temp / 1000000000ULL;
    rttm.tv_nsec = temp % 1000000000ULL;
  }

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

  int flag = WORK;

  unsigned long long offset = get_time();
  unsigned long long g_rttn_start = get_time();
  timer_overhead = g_rttn_start - offset;
  unsigned long long g_end, rtt_end, rtt1_total = 0, rttn = 0;
  rtt_1 = 100;

  // run the 0 message size ping pong 1000 times to stabalize rtt1 value
  for (int i = 0; i < RTT1_AVG; i++) {
    // start of rtt1 calc for use in calculating g(0)
    if (!rank) {
      g_rttn_start = get_time();
      MPI_Send(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, WORK, MPI_COMM_WORLD);
      MPI_Recv(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, WORK, MPI_COMM_WORLD,
               &status);
      rtt_end = get_time();

      // discount the first 200 runs to ensure "warmup"
      if (i > 199) {
        rtt_1 = rtt_end - g_rttn_start - timer_overhead;
        rtt1_total += rtt_1;
      }
    }

    if (rank) {
      MPI_Recv(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, WORK, MPI_COMM_WORLD,
               &status);
      MPI_Send(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, WORK,
               MPI_COMM_WORLD);
    }
  }

  if (!rank) {
    rtt_1 = rtt1_total / (RTT1_AVG - 200);
  }
  // end rtt1 calc

  double g[2] = {10.0, 1.0};
  // Loop for calculating g0 and RTTn
  while ((epsilon((double)g[1], (double)g[0]) > 1 || rtt_1 > (0.01 * rttn)) &&
         flag) {

    // Set prev g value to current for new calculation
    if (!rank) {
      g[0] = g[1];
      g_rttn_start = get_time();
    }

    for (int i = 0; i < n_runs && flag; i++) {
      if (!rank) {
        MPI_Send(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, WORK,
                 MPI_COMM_WORLD);
      }

      if (rank) {
        MPI_Recv(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, MPI_ANY_TAG,
                 MPI_COMM_WORLD, &status);
        flag = status.MPI_TAG;
      }
    }

    if (!rank) {
      g_end = get_time();

      MPI_Recv(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, MPI_ANY_TAG,
               MPI_COMM_WORLD, &status);
      rtt_end = get_time();

      g[1] = (g_end - g_rttn_start - timer_overhead) / (double)n_runs;
      rttn = (rtt_end - g_rttn_start - 2 * timer_overhead);
      printf("rttn = %llu, g[1] = %f, g[0] = %f, rtt_1, %llu\n", rttn, g[1],
             g[0], rtt_1);
    }

    if (rank) {
      MPI_Send(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, WORK,
               MPI_COMM_WORLD);
    }

    n_runs *= 2;
    if (n_runs > pow(2, 20)) {
      printf("error, n_runs was too high\n");
      break;
    }
  }

  g_0 = g[1];

  // Tells the mirror to stop waiting on a recv
  if (!rank) {
    MPI_Send(NULL, ZERO_DATA_COUNT, MPI_CHAR, 1, STOP, MPI_COMM_WORLD);
  }
  // end of g0 and rttn calculation

  double L = (double)rtt_1 / 2 - g_0;

  // Print values for part 1
  if (!rank) {
    printf("rtt_1 = %llu, g0 = %f, rttn (calculated with nruns = %d)= %llu, "
           "L = %f \n",
           rtt_1, g_0, n_runs, rttn, L);
  }

  result *R = malloc(sizeof(result));
  List *list = malloc(sizeof(List));
  list->tail = NULL;
  list->head = NULL;
  list->size = 0;

  // Extrapolate and check values for g(m)/m
  int k;
  for (k = 0; k < K_M; k++) {
    R->m = m(k);
    getResult(R);
    insertNode(list, *R);
  }

  // If the value at k is not within 1% of the value predicted by k-1 and
  k - 2,
      // increase k by one and generate new result and compare again

      // Assume that the paper meant g(m)/m must be within 1%
      flag = WORK;
  while (extrapolateGMOverM(list) && flag && k < 30) {

    if (!rank) {
      printf("k = %d ", k);
      MPI_Send(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, WORK, MPI_COMM_WORLD);
    } else {
      MPI_Recv(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ZERO, MPI_ANY_TAG,
               MPI_COMM_WORLD, &status);
      flag = status.MPI_TAG;
      if (!flag)
        break;
    }

    k++;
    R->m = m(k);
    getResult(R);
    insertNode(list, *R);
  }

  if (!rank) {
    MPI_Send(NULL, ZERO_DATA_COUNT, MPI_CHAR, RANK_ONE, STOP, MPI_COMM_WORLD);
  }

  if (!rank)
    printList(list);

  // Extrapolate and check values for g(m)/m

  freeList(list);
  free(list);

  // BUG:
  //  if (!rank) {
  //    List *list = malloc(sizeof(List));
  //    list->tail = NULL;
  //    list->head = NULL;
  //    list->size = 0;
  //    result R;
  //    for (int i = 0; i < 100; i++) {
  //      R.m = rand() % 100;
  //      printf("%d\n", R.m);
  //      insertNode(list, R);
  //    }
  //    printList(list);
  //    freeList(list);
  //    free(list);
  //  }

  // Print values for part 2 and 3

  MPI_Finalize();

  return 0;
}
