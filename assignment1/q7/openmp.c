#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 256

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int *data;
  int global_min;

  data = malloc(ARRAY_SIZE * sizeof(int));
  for (int i = 0; i < ARRAY_SIZE; i++)
    data[i] = rand() % 1000;

#pragma omp parallel for reduction(min : global_min)
  for (int i = 0; i < ARRAY_SIZE; i++) {
    if (data[i] < global_min)
      global_min = data[i];
  }

  // For verification
  // for (int i = 0; i < ARRAY_SIZE; i++) {
  //   printf("%d,", data[i]);
  // }
  // printf("\n");
  printf("Global min = %d", global_min);

  return 0;
}
