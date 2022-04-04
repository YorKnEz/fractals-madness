/*
TO DO:
Compile command so that it works locally
Fix zooming for a custom SCALE
*/

#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <boost/multiprecision/cpp_dec_float.hpp>

using namespace boost::multiprecision;

#define TITLE "Fractal"
#define SIZE 512
#define FLOAT double
#define ITERATIONS 100
#define SCALE 2

bool quit = false;

uint fps = 60;
uint minframetime = 1000 / fps;
uint16_t window_size = SIZE;

int divergenceSpeed[SIZE][SIZE];

struct Point {
  FLOAT x, y;
};

// the constant that generates the fractal, can be changed
Point c = { -0.55, 0.56 };

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

// this function takes a point from the plane and sees if it converges or diverges
int computeDivergenceSpeedForPoint(int x, int y);

// this function takes each point from the visible plane and checks if it converges or diverges
void computeDivergenceSpeed();

// this function gives out an rgb array for a picked ratio
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

  // compute the convergence speed of each point before rendering
  computeDivergenceSpeed();

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
        printf("r key pressed\n");
        scale = window_size / 4;
        
        zoomPosition = lastZoomPosition = { 0, 0 };

        k = 0;

        computeDivergenceSpeed();
        break;
      }
    }
    else if (event.type == SDL_MOUSEWHEEL) {
      // this is a scroll up
      if (event.wheel.y > 0) {
        printf("Scroll up\n");
        scale *= SCALE; k++;
        printf("Scale: 2^%d\n", k);

        lastZoomPosition = zoomPosition;

        zoomPosition.x += ((FLOAT)mouseX - (FLOAT)window_size / 2) / scale;
        zoomPosition.y += -((FLOAT)mouseY - (FLOAT)window_size / 2) / scale;

        computeDivergenceSpeed();
        break;
      }
      // this is a scroll down
      else if (event.wheel.y < 0) {
        printf("Scroll down\n");
        scale /= SCALE; k--;
        printf("Scale: 2^%d\n", k);

        zoomPosition = lastZoomPosition;

        computeDivergenceSpeed();
        break;
      }
    }
  }
}

void draw(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // draw the fractal based on divergenceSpeed
  for (int x = 0; x < window_size; x++) {
    for (int y = 0; y < window_size; y++) {
      if (divergenceSpeed[x][y] > 0) {
        // set the color using rainbow funciton
        uint color = rgb((double)divergenceSpeed[x][y] / ITERATIONS);

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

int computeDivergenceSpeedForPoint(int x, int y) {
  // initial Z
  Point C = {
    ((FLOAT)x - (FLOAT)window_size / 2) / scale + zoomPosition.x,
    -((FLOAT)y - (FLOAT)window_size / 2) / scale + zoomPosition.y
  };

  // next Z
  Point Z0 = { 0, 0 }, Z1;

  for (int i = 1; i <= ITERATIONS; i++) {
    // calculate Z1 based on the recurence formula
    Z1.x = Z0.x * Z0.x - Z0.y * Z0.y + C.x;
    Z1.y = 2 * Z0.x * Z0.y + C.y;

    // Check if it diverged
    if (abs(Z1.x) > 2 || abs(Z1.y) > 2) return i;

    Z0 = Z1;
  }

  return 0;
}

// int computeDivergenceSpeedForPoint(int x, int y) {
//   // initial Z
//   Point Z0 = {
//     ((FLOAT)x - (FLOAT)window_size / 2) / scale + zoomPosition.x,
//     -((FLOAT)y - (FLOAT)window_size / 2) / scale + zoomPosition.y
//   };

//   // next Z
//   Point Z1;

//   for (int i = 1; i <= ITERATIONS; i++) {
//     // calculate Z1 based on the recurence formula
//     Z1.x = Z0.x * Z0.x - Z0.y * Z0.y + c.x;
//     Z1.y = 2 * Z0.x * Z0.y + c.y;

//     // Check if it diverged
//     if (abs(Z1.x) > 2 || abs(Z1.y) > 2) return i;

//     Z0 = Z1;
//   }

//   return 0;
// }

void computeDivergenceSpeed() {
  for (int x = 0; x < window_size; x++) {
    for (int y = 0; y < window_size; y++) {
      divergenceSpeed[x][y] = computeDivergenceSpeedForPoint(x, y);
    }
  }
}

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