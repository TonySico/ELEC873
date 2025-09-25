/* Sequential Mandelbrot program */

#include <stdio.h>

#define X_RESN 800 /* x resolution */
#define Y_RESN 800 /* y resolution */

typedef struct complextype {
  float real, imag;
} Compl;

int main() {
  /* Mandlebrot variables */
  int i, j, k[X_RESN][Y_RESN];
  Compl z, c;
  float lengthsq, temp;

  /* Calculate and save points into array */

  for (i = 0; i < X_RESN; i++)
    for (j = 0; j < Y_RESN; j++) {

      z.real = z.imag = 0.0;
      c.real =
          ((float)j - 400.0) / 200.0; /* scale factors for 800 x 800 window */
      c.imag = ((float)i - 400.0) / 200.0;
      k[i][j] = 0;

      do { /* iterate for pixel color */
        temp = z.real * z.real - z.imag * z.imag + c.real;
        z.imag = 2.0 * z.real * z.imag + c.imag;
        z.real = temp;
        lengthsq = z.real * z.real + z.imag * z.imag;
        k[i][j]++;
      } while (lengthsq < 4.0 && k[i][j] < 100);
    }

  /* Save array of points out to file */
  FILE *output;

  output = fopen("mandelbrot_data.txt", "w+");
  if (output == NULL) {
    printf("Error opening file\n");
    return -1;
  }

  for (i = 0; i < X_RESN; i++)
    for (j = 0; j < Y_RESN; j++) {
      fprintf(output, "%d\n", k[i][j]);
    }

  fclose(output);

  return 0;
}
