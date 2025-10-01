// Anthony Sicoie (20214793)

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define ARRAY_SIZE 256

int main(int argc, char *argv[]) {
  srand(20214793);
  int *data;
  int global_min = INT_MAX;

  data = malloc(ARRAY_SIZE * sizeof(int));
  for (int i = 0; i < ARRAY_SIZE; i++)
    data[i] = rand() % 1000;

#pragma omp parallel for reduction(min : global_min)
  for (int i = 0; i < ARRAY_SIZE; i++) {
      if (data[i] < global_min)
        global_min = data[i];
  }

  // For verification
  // int tempMin = data[0];
  // for (int i = 0; i < ARRAY_SIZE; i++) {
  //   if (data[i] < tempMin)
  //     tempMin = data[i];
  // }
  // printf("Global min = %d, verify min = %d\n", global_min, tempMin);
  printf("Global min = %d\n", global_min);

  return 0;
}
