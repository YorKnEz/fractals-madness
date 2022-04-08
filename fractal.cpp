#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <boost/multiprecision/cpp_dec_float.hpp>

using namespace boost::multiprecision;

// the title of the window
#define TITLE "Fractal"
// the size of the window in pixels
#define SIZE 768
// the precision of the numbers, available options:
// float, double, cpp_dec_float_50, cpp_dec_float_100
#define FLOAT double
// the scale of the zoom
#define SCALE 2
// the amount of of zooms precomputed
#define ZOOMS 50
// the depth the program goes to determine the divergence of a point
int ITERATIONS = 100;

bool quit = false;

uint fps = 60;
uint minframetime = 1000 / fps;
uint16_t window_size = SIZE;

// the matrix which stores the divergence speed of each point
int divergenceSpeed[SIZE][SIZE];

// the matrix which stores the divergence speed of the first zooms
int precomputedDivergenceSpeed[ZOOMS][SIZE][SIZE];

bool precompute = false;

struct Point {
  FLOAT x, y;
};

// the point on which to zoom
Point zoomPosition = { 0, 0 };
// the point on which it zoomed last time
Point lastZoomPosition = { 0, 0 };
// the scale
FLOAT scale = (FLOAT)window_size / 4;
// number of times the fractal has been zoomed in or out relative to the initial position
int k = 0;

void events(SDL_Window *window);

void draw(SDL_Renderer *renderer);

void startAnimation(SDL_Renderer *renderer);

int generateMandelbrotSet(int x, int y);

int generateFractal(int x, int y);

void computeDivergenceSpeed();

void precomputeDivergenceSpeed(int mouseX, int mouseY);

uint32_t rgb(double ratio);

int main(int argc, char **argv) { 
  
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  SDL_Window *window =
    SDL_CreateWindow(
      TITLE,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      window_size,
      window_size,
      SDL_WINDOW_OPENGL);

  int display_index = SDL_GetWindowDisplayIndex(window);

  SDL_DisplayMode mode;

  if (SDL_GetDesktopDisplayMode(display_index, &mode) == 0) {
    fps = mode.refresh_rate;
    minframetime = 1000 / fps;
  }

  SDL_Renderer *renderer =
    SDL_CreateRenderer(
      window,
      -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  // a nice animation showing the ITERATIONS go from 1 to 100
  startAnimation(renderer);

  while (!quit) {
    int start = SDL_GetTicks();

    events(window);
    draw(renderer);

    SDL_RenderPresent(renderer);

    int time = SDL_GetTicks() - start;
    int sleepTime = minframetime - time;
    if (sleepTime > 0) {
        SDL_Delay(sleepTime);
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

void events(SDL_Window *window) {
  SDL_Event event;

  while (SDL_PollEvent(&event) != 0) {
    // get current cursor position
    int mouseX, mouseY;

    SDL_GetMouseState(&mouseX, &mouseY);

    if (event.type == SDL_QUIT) {
      quit = true;
    }
    else if (event.type == SDL_KEYDOWN) {
      // get current key that is pressed
      SDL_Keycode keycode = event.key.keysym.sym;

      switch (keycode) {
      // r is for resetting zoom
      case SDLK_r:
        scale = (FLOAT)window_size / 4;
        
        zoomPosition = lastZoomPosition = { 0, 0 };

        k = 0;

        precompute = false;

        computeDivergenceSpeed();

        printf("Fractal reset\n");
        break;
      case SDLK_p:
        precompute = !precompute;

        if (precompute) printf("Precomputation is on\n");
        else printf("Precomputation is off\n");

        precomputeDivergenceSpeed(mouseX, mouseY);
        break;
      }
    }
    else if (event.type == SDL_MOUSEWHEEL) {
      // this is a scroll up
      if (event.wheel.y > 0) {
        printf("Scroll up\n");
        k++;

        if (!precompute) {
          Point zoomPositionPreZoom = {
            +((FLOAT)mouseX - (FLOAT)window_size / 2) / scale + zoomPosition.x,
            -((FLOAT)mouseY - (FLOAT)window_size / 2) / scale + zoomPosition.y
          };

          scale *= SCALE;

          lastZoomPosition = zoomPosition;

          zoomPosition.x = zoomPositionPreZoom.x - ((FLOAT)mouseX - (FLOAT)window_size / 2) / scale;
          zoomPosition.y = zoomPositionPreZoom.y + ((FLOAT)mouseY - (FLOAT)window_size / 2) / scale;

          computeDivergenceSpeed();
        }

        printf("Scale: %.2f^%d\n", (double)SCALE, k);
      }
      // this is a scroll down
      else if (event.wheel.y < 0) {
        printf("Scroll down\n");
        k--;

        if (!precompute) {
          scale /= SCALE;

          zoomPosition = lastZoomPosition;

          computeDivergenceSpeed();
        }

        printf("Scale: %.2f^%d\n", (double)SCALE, k);
      }
    }
  }
}

void draw(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  if (precompute) {
    // draw the fractal based on precomputedDivergenceSpeed
    for (int x = 0; x < window_size; x++) {
      for (int y = 0; y < window_size; y++) {
        if (precomputedDivergenceSpeed[k][x][y] > 0) {
          // set the color using rainbow funciton
          uint color = rgb((double)precomputedDivergenceSpeed[k][x][y] / 100 / 1.5);

          // decode the color to RGB values
          unsigned int red   = (color & 0x00ff0000) >> 16;
          unsigned int green = (color & 0x0000ff00) >> 8;
          unsigned int blue  = (color & 0x000000ff);

          SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
          SDL_RenderDrawPoint(renderer, x, y);
        }
      }
    }
  } else {
    // draw the fractal based on divergenceSpeed
    for (int x = 0; x < window_size; x++) {
      for (int y = 0; y < window_size; y++) {
        if (divergenceSpeed[x][y] > 0) {
          // set the color using rainbow funciton
          uint color = rgb((double)divergenceSpeed[x][y] / 100 / 1.5);

          // decode the color to RGB values
          unsigned int red   = (color & 0x00ff0000) >> 16;
          unsigned int green = (color & 0x0000ff00) >> 8;
          unsigned int blue  = (color & 0x000000ff);

          SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
          SDL_RenderDrawPoint(renderer, x, y);
        }
      }
    }
  }
}

void startAnimation(SDL_Renderer *renderer) {
  // remember the initial number of ITERATIONS in order to reset them after the animation is over
  int INITIAL_ITERATIONS = ITERATIONS;
  float frame = 0;
  ITERATIONS = 1;

  while (ITERATIONS < 100) {
    computeDivergenceSpeed();

    draw(renderer);

    SDL_RenderPresent(renderer);

    frame += 0.07;
    ITERATIONS += (int)frame;
  }

  ITERATIONS = INITIAL_ITERATIONS;
  
  printf("Starting animation done\n");
}

// this function generates the Mandelbrot Set
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

// this function generates a fractal based on a constant c
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

// this function takes each point from the visible plane and checks if it converges or diverges
void computeDivergenceSpeed() {
  // get time before starting the computation
  int startTime = SDL_GetTicks();

  // the constant used by generateFractal to generate a fractal, can be changed
  Point c = { -0.18, -0.67 };

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

// this function precomputes the first ZOOMS zooms
void precomputeDivergenceSpeed(int mouseX, int mouseY) {
  // get time before starting the precomputation
  int startTime = SDL_GetTicks();

  // the constant used by generateFractal to generate a fractal, can be changed
  Point c = { -0.18, -0.67 }, zoomPositionPreZoom;

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

// this function gives out an rgb array for a picked ratio
//input: ratio is between 0.0 to 1.0
//output: rgb color
uint32_t rgb(double ratio) {
    //we want to normalize ratio so that it fits in to 6 regions
    //where each region is 256 units long
    int normalized = int(ratio * 256 * 6);

    //find the region for this position
    int region = normalized / 256;

    //find the distance to the start of the closest region
    int x = normalized % 256;

    uint8_t r = 0, g = 0, b = 0;
    switch (region) {
    case 0:
      r = 255; g = 0;   b = 0;   g += x;
      break;
    case 1:
      r = 255; g = 255; b = 0;   r -= x;
      break;
    case 2:
      r = 0;   g = 255; b = 0;   b += x;
      break;
    case 3:
      r = 0;   g = 255; b = 255; g -= x;
      break;
    case 4:
      r = 0;   g = 0;   b = 255; r += x;
      break;
    case 5:
      r = 255; g = 0;   b = 255; b -= x;
      break;
    }

    return r + (g << 8) + (b << 16);
}