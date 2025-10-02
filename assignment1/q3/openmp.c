// Anthony Sicoie (20214793)

#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int integers[1000];
  for (int i = 0; i < 1000; i++)
    integers[i] = i;

  int sum = 0;

  // leveraging the OpenMP reduction directive
#pragma omp parallel for reduction(+ : sum)
  for (long i = 0; i < 1000; i++) {
    sum += integers[i];
  }

  printf("Sum = %d\n", sum);

  return 0;
}
