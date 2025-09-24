#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int nthreads, myid;

#pragma omp parallel private(nthreads, myid)
  {
    nthreads = omp_get_num_threads();
    myid = omp_get_thread_num();
    printf("Hello World, I am thread %d out of %d threads.\n", myid, nthreads);
  }

  return 0;
}
