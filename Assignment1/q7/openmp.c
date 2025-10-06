// Anthony Sicoie (20214793)

#include <limits.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 256

int main(int argc, char *argv[]) {
  srand(20214793);
  int *data;
  // Init global min to ensure a value is there to start
  int global_min = INT_MAX;

  data = malloc(ARRAY_SIZE * sizeof(int));
  for (int i = 0; i < ARRAY_SIZE; i++)
    data[i] = rand() % 1000;

  // use built in OpenMP directive to find global min
#pragma omp parallel for reduction(min : global_min)
  for (int i = 0; i < ARRAY_SIZE; i++) {
    if (data[i] < global_min)
      global_min = data[i];
  }

  printf("Global min = %d\n", global_min);

  return 0;
}
