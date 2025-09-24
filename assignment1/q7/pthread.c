#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 256

int nthreads;
int *data;

typedef struct {
  int thread_id;
  int *local_min;
} threadsInfo;

void *slave(void *arg) {
  threadsInfo *t = (threadsInfo *)arg;
  int id = t->thread_id;

  for (int i = id; i < ARRAY_SIZE; i += nthreads) {
    if (data[i] < *(t->local_min))
      *(t->local_min) = data[i];
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  nthreads = atoi(argv[1]); // user specified thread count
  int i;
  int *local_mins;

  srand(time(NULL));

  data = malloc(ARRAY_SIZE * sizeof(int));
  for (i = 0; i < ARRAY_SIZE; i++)
    data[i] = rand() % 1000;

  int global_min = data[0];

  local_mins = malloc(nthreads * sizeof(int));
  for (i = 0; i < nthreads; i++)
    local_mins[i] = data[i];

  pthread_t threads[nthreads];
  threadsInfo t[nthreads];

  for (i = 0; i < nthreads; i++) {
    t[i].thread_id = i;
    t[i].local_min = &local_mins[i];
    if (pthread_create(&threads[i], NULL, slave, &t[i]) != 0)
      perror("Pthread_create fails");
  }

  for (i = 0; i < nthreads; i++) {
    if (pthread_join(threads[i], NULL) != 0)
      perror("Pthread_join fails");
  }

  for (int i = 0; i < nthreads; i++) {
    if (*t[i].local_min < global_min)
      global_min = *t[i].local_min;
  }

  // For verification
  // for (int i = 0; i < ARRAY_SIZE; i++) {
  //   printf("%d,", data[i]);
  // }
  // printf("\n");
  printf("Global min = %d", global_min);

  return 0;
}
