// Anthony Sicoie (20214793)

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int nthreads;

void *slave(void *arg) {
  int myid = (int)(intptr_t)arg;
  printf("Hello World, I am thread %d out of %d threads.\n", myid, nthreads);
  return NULL;
}

int main(int argc, char *argv[]) {
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

  return 0;
}
