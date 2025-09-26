#include <omp.h>
#include <stdio.h>

static long num_steps = 100000;
double step;

int main(int argc, char *argv[]) {
  step = 1.0 / (double)num_steps;

  double x, pi = 0.0;

#pragma omp parallel for reduction(+ : pi) private(x)
  for (long i = 0; i < num_steps; i++) {
    x = (i + 0.5) * step;
    pi += 4.0 / (1.0 + x * x);
  }

  pi *= step;

  printf("Pi = %f\n", pi);

  return 0;
}
