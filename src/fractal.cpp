#include "fractal.h"

int ITERATIONS = 100;
uint16_t window_size = SIZE;
int divergenceSpeed[SIZE][SIZE];
int precomputedDivergenceSpeed[ZOOMS][SIZE][SIZE];
bool precompute = false;
Point c = { -0.18, -0.67 };
Point zoomPosition = { 0, 0 };
Point lastZoomPosition = { 0, 0 };
FLOAT scale = (FLOAT)window_size / 4;
int k = 0;

int generateMandelbrotSet(int x, int y) {
  /*
    the way the Mandelbrot set is generated is that you take each constant C from the complex plane and you check if the sequence Zn = Zn-1^2 + C, with Z0 = 0, is a converging sequence
    
    the way the coloring works is that we check how fast a sequence diverges and color the pixel using a rainbow function that will take as input a number from 0 to 1
    
    in order to obtain this number (from 0 to 1) we take the divergenceSpeed and divide it by the number of iterations (or iterations * something if we want the colors to be more smooth)
  */

  // the constant of the sequence
  Point C = {
    (((FLOAT)x - (FLOAT)window_size / 2) / scale) + zoomPosition.x,
    (-((FLOAT)y - (FLOAT)window_size / 2) / scale) + zoomPosition.y
  };

  // current Z and next Z
  Point Z0 = { 0, 0 }, Z1;

  // we calculate the first ITERATIONS terms of the sequence so we can see if it diverges or converges
  for (int i = 1; i <= ITERATIONS; i++) {
    // calculate Z1 based on the recurence formula Zn = Zn-1^2 + C
    // the formula for Z^2, where Z = a+ib is Z^2 = a^2 - b^2 + 2abi
    Z1.x = Z0.x * Z0.x - Z0.y * Z0.y + C.x;
    Z1.y = 2 * Z0.x * Z0.y + C.y;

    // to check if the sequence diverged, we just need to check if it got outside the plane formed by the points (-2, -2) (2, 2) (outside the screen)
    if (abs(Z1.x) > 2 || abs(Z1.y) > 2) return i;

    Z0 = Z1;
  }

  return 0;
}

int generateFractal(Point c, int x, int y) {
  /*
    the way a fractal is generates is that you take each point from the complex plane and you check if the sequence Zn = Zn-1^2 + C, with Z0 being the point we take from the plane and C an arbitrary picked constant, is a converging sequence
    
    the way the coloring works is that we check how fast a sequence diverges and color the pixel using a rainbow function that will take as input a number from 0 to 1
    
    in order to obtain this number (from 0 to 1) we take the divergenceSpeed and divide it by the number of iterations (or iterations * something if we want the colors to be more smooth)
  */
  // the current point picked from the plane
  Point Z0 = {
    (((FLOAT)x - (FLOAT)window_size / 2) / scale) + zoomPosition.x,
    (-((FLOAT)y - (FLOAT)window_size / 2) / scale) + zoomPosition.y
  };

  // next Z
  Point Z1;

  // we calculate the first ITERATIONS terms of the sequence so we can see if it diverges or converges
  for (int i = 1; i <= ITERATIONS; i++) {
    // calculate Z1 based on the recurence formula Zn = Zn-1^2 + C
    // the formula for Z^2, where Z = a+ib is Z^2 = a^2 - b^2 + 2abi
    Z1.x = Z0.x * Z0.x - Z0.y * Z0.y + c.x;
    Z1.y = 2 * Z0.x * Z0.y + c.y;

    // to check if the sequence diverged, we just need to check if it got outside the plane formed by the points (-2, -2) (2, 2) (outside the screen)
    if (abs(Z1.x) > 2 || abs(Z1.y) > 2) return i;

    Z0 = Z1;
  }

  return 0;
}

void computeDivergenceSpeed() {
  // get time before starting the computation
  int startTime = SDL_GetTicks();

  // take each pixel from the screen and calculate it's divergence
  for (int x = 0; x < window_size; x++) {
    for (int y = 0; y < window_size; y++) {
      // here can be used either the generateMandelbrotSet function or the generateFractal function, which generates a fractal based on a constant c
      divergenceSpeed[x][y] = generateMandelbrotSet(x, y);
      // divergenceSpeed[x][y] = generateFractal(c, x, y);
    }
  }

  // get time after finishing the computation
  int endTime = SDL_GetTicks();
  // calculate the time elapsed in seconds
  float timeElapsed = float(endTime - startTime) / 1000;

  printf("Time elapsed: %.3fs\n", timeElapsed);
}

void precomputeDivergenceSpeed(int mouseX, int mouseY) {
  // get time before starting the precomputation
  int startTime = SDL_GetTicks();

  Point zoomPositionPreZoom;

  for (int zoomIndex = 0; zoomIndex < ZOOMS; zoomIndex++) {
    // take each pixel from the screen and calculate it's divergence
    for (int x = 0; x < window_size; x++) {
      for (int y = 0; y < window_size; y++) {
        // here can be used either the generateMandelbrotSet function or the generateFractal function, which generates a fractal based on a constant c
        precomputedDivergenceSpeed[zoomIndex][x][y] = generateMandelbrotSet(x, y);
        // precomputedDivergenceSpeed[zoomIndex][x][y] = generateFractal(c, x, y);
      }
    }

    // without this check, if at some point we zoom in on a black point, eventually the screen will become black, and continuing the precomputation would waste time
    if (!precomputedDivergenceSpeed[zoomIndex][mouseX][mouseY]) break;

    zoomPositionPreZoom = {
      +((FLOAT)mouseX - (FLOAT)window_size / 2) / scale + zoomPosition.x,
      -((FLOAT)mouseY - (FLOAT)window_size / 2) / scale + zoomPosition.y
    };

    scale *= SCALE;

    zoomPosition = {
      zoomPositionPreZoom.x - ((FLOAT)mouseX - (FLOAT)window_size / 2) / scale,
      zoomPositionPreZoom.y + ((FLOAT)mouseY - (FLOAT)window_size / 2) / scale
    };
  }

  scale = (FLOAT)window_size / 4;
  zoomPosition = { 0, 0 };

  // get time after finishing the computation
  int endTime = SDL_GetTicks();
  // calculate the time elapsed in seconds
  float timeElapsed = float(endTime - startTime) / 1000;

  printf("Time elapsed: %.3fs\n", timeElapsed);
}