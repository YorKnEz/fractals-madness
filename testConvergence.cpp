#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

uint16_t window_size = 512;

typedef struct ColorRGBA {
  int R, G, B, A;
} colors[10];

struct Point {
  float x, y;
};

Point c = { -0.9, 0 };
int zoom = window_size / 4;

float roundOff(float value, int prec) {
  float pow_10 = pow(10.0f, (float)prec);
  return round(value * pow_10);
}

int computeConvergenceForPoint(int x, int y) {
  Point Z;

  Z.x = ((float)(x) - (float)(window_size) / 2) / zoom;
  Z.y = -((float)(y) - (float)(window_size) / 2) / zoom; 

  Point Z0 = Z, Zn;

  for (int i = 1; i <= 100; i++) {
    Zn.x = Z.x * Z.x - Z.y * Z.y + c.x;
    Zn.y = 2 * Z.x * Z.y + c.y;

    printf("Z(%f, %f) Zn(%f, %f)\n", Z.x, Z.y, Zn.x, Zn.y);

    // Check if it converged
    if (roundOff(Z.x, 1) == roundOff(Zn.x, 1) && roundOff(Z.y, 1) == roundOff(Zn.y, 1)) return i;

    Z = Zn;

    // Check if it diverged
    if (abs(Z.x) > 512 || abs(Z.y) > 512) return 0;
  }

  return 100;
}

int main(int argc, char **argv) {
  int x, y;
  while (true) {
    scanf("%d %d", &x, &y);

    printf("%d\n", computeConvergenceForPoint(x, y));
  }

  return 0;
}