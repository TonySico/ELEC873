/* Sequential Mandelbrot program */

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X_RESN 800 /* x resolution */
#define Y_RESN 800 /* y resolution */

int main() {
  Window win;                        /* initialization for a window */
  unsigned int width, height,        /* window size */
      x, y,                          /* window position */
      border_width,                  /*border width in pixels */
      display_width, display_height, /* size of screen */
      screen;                        /* which screen */

  char *window_name = "Mandelbrot Set", *display_name = NULL;
  GC gc;
  unsigned long valuemask = 0;
  XGCValues values;
  Display *display;
  XSizeHints size_hints;
  Pixmap bitmap;
  XPoint points[800];
  FILE *fp;
  char str[100];

  XSetWindowAttributes attr[1];

  /* Mandlebrot variables */
  int i, j, k;

  /* Open file containing data */
  FILE *input_file;

  input_file = fopen("mandelbrot_data_dynamic.txt", "r");
  // input_file = fopen("mandelbrot_data_static.txt", "r");
  if (input_file == NULL) {
    printf("Error opening file \n");
    return -1;
  }

  /* connect to Xserver */

  if ((display = XOpenDisplay(display_name)) == NULL) {
    fprintf(stderr, "drawon: cannot connect to X server %s\n",
            XDisplayName(display_name));
    exit(-1);
  }

  /* get screen size */

  screen = DefaultScreen(display);
  display_width = DisplayWidth(display, screen);
  display_height = DisplayHeight(display, screen);

  /* set window size */

  width = X_RESN;
  height = Y_RESN;

  /* set window position */

  x = 0;
  y = 0;

  /* create opaque window */

  border_width = 4;
  win = XCreateSimpleWindow(display, RootWindow(display, screen), x, y, width,
                            height, border_width, BlackPixel(display, screen),
                            WhitePixel(display, screen));

  size_hints.flags = USPosition | USSize;
  size_hints.x = x;
  size_hints.y = y;
  size_hints.width = width;
  size_hints.height = height;
  size_hints.min_width = 300;
  size_hints.min_height = 300;

  XSetNormalHints(display, win, &size_hints);
  XStoreName(display, win, window_name);

  /* create graphics context */

  gc = XCreateGC(display, win, valuemask, &values);

  XSetBackground(display, gc, WhitePixel(display, screen));
  XSetForeground(display, gc, BlackPixel(display, screen));
  XSetLineAttributes(display, gc, 1, LineSolid, CapRound, JoinRound);

  attr[0].backing_store = Always;
  attr[0].backing_planes = 1;
  attr[0].backing_pixel = BlackPixel(display, screen);

  XChangeWindowAttributes(
      display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

  XMapWindow(display, win);
  XSync(display, 0);

  /* Read in points and draw them */

  for (i = 0; i < X_RESN; i++)
    for (j = 0; j < Y_RESN; j++) {

      fscanf(input_file, "%d\n", &k);
      unsigned long color;

      // Bright neon-like gradient from blue->pink->purple
      int r_neon = (int)(127.0 + 127.0 * sin(k * 0.3));
      int g_neon = (int)(127.0 + 127.0 * sin(k * 0.3 + 2));
      int b_neon = (int)(127.0 + 127.0 * sin(k * 0.3 + 4));
      color = (r_neon << 16) | (g_neon << 8) | b_neon;

      // Scaling RGB values based on k value
      if (k >= 65 && k <= 100) {
        int r = (k - 75) * 255 / (100 - 75);
        color = (r << 16); // RRRR RRRR 0000 0000 0000 0000
      } else if (k >= 50 && k <= 74) {
        int g = (k - 50) * 255 / (74 - 50);
        color = (g << 8); // 0000 0000 GGGG GGGG 0000 0000
      } else if (k >= 0 && k <= 49) {
        int b = k * 255 / 49;
        color = b; // 0000 0000 0000 0000 BBBB BBBB
      } else {
        color = 0; // black for any out-of-range values
      }

      XSetForeground(display, gc, color);
      XDrawPoint(display, win, gc, j, i);
    }

  fclose(input_file);

  XFlush(display);
  sleep(30);

  /* Program Finished */
  return 0;
}
