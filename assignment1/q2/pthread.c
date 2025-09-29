// Anthony Sicoie (20214793)

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static long num_steps = 100000;

pthread_mutex_t mutex1;

int nthreads;

double step;
double pi;

void *slave(void *arg) {
  int myid = (int)(intptr_t)arg;
  double local_sum = 0.0, x;

  for (int i = myid; i < num_steps; i += nthreads) {
    x = (i + 0.5) * step;
    local_sum += 4.0 / (1.0 + x * x);
  }

  pthread_mutex_lock(&mutex1);
  pi += local_sum;
  pthread_mutex_unlock(&mutex1);

  return NULL;
}

int main(int argc, char *argv[]) {
  step = 1.0 / (double)num_steps;
  nthreads = atoi(argv[1]); // user specified thread count
  int i;

  pthread_t threads[nthreads];

  for (i = 0; i < nthreads; i++) {
    if (pthread_create(&threads[i], NULL, slave, (void *)(intptr_t)i) != 0)
      perror("Pthread_create fails");
  }

  for (i = 0; i < nthreads; i++) {
    if (pthread_join(threads[i], NULL) != 0)
      perror("Pthread_join fails");
  }

  pi *= step;

  printf("Pi = %f\n", pi);

  return 0;
}
