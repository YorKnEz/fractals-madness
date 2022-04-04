#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define TITLE "Fractal"
#define SIZE 512

bool quit = false;

uint fps = 60;
uint minframetime = 1000 / fps;
uint16_t window_size = SIZE;

int divergenceSpeed[SIZE][SIZE];

struct Point {
  double x, y;
};

// the constant that generates the fractal, can be changed
Point c = { -0.55, 0.56 };

// the point on which to zoom
Point zoomPosition = { 0, 0 };

// the point on which it zoomed last time
Point lastZoomPosition = { 0, 0 };

// the scale
double scale = (double)window_size / 4;

// number of times the fractal has been zoomed in or out
int k = 0;

void events(SDL_Window *window);

void draw(SDL_Renderer *renderer);

// this function takes a point from the plane and sees if it converges or diverges
int computeDivergenceSpeedForPoint(int x, int y);

// this function takes each point from the visible plane and checks if it converges or diverges
void computeDivergenceSpeed();

uint rainbow(uint n);

uint wave(uint n);

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

  fclose(stdout);

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
        scale *= 2; k++;
        printf("Scale: 2^%d\n", k);

        lastZoomPosition = zoomPosition;

        zoomPosition.x += ((double)mouseX - (double)window_size / 2) / scale;
        zoomPosition.y += -((double)mouseY - (double)window_size / 2) / scale;

        computeDivergenceSpeed();
        break;
      }
      // this is a scroll down
      else if (event.wheel.y < 0) {
        printf("Scroll down\n");
        scale /= 2; k--;
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
        uint color = rainbow(divergenceSpeed[x][y]);

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
  Point Z0 = {
    ((double)x - (double)window_size / 2) / scale + zoomPosition.x,
    -((double)y - (double)window_size / 2) / scale + zoomPosition.y
  };

  // next Z
  Point Z1;

  for (int i = 1; i <= 100; i++) {
    // calculate Z1 based on the recurence formula
    Z1.x = Z0.x * Z0.x - Z0.y * Z0.y + c.x;
    Z1.y = 2 * Z0.x * Z0.y + c.y;

    // Check if it diverged
    if (abs(Z1.x) > 2 || abs(Z1.y) > 2) return i;

    Z0 = Z1;
  }

  return 0;
}

void computeDivergenceSpeed() {
  for (int x = 0; x < window_size; x++) {
    for (int y = 0; y < window_size; y++) {
      divergenceSpeed[x][y] = computeDivergenceSpeedForPoint(x, y);
    }
  }
}

uint rainbow(uint n) {
  return (wave(n) << 16) + (wave(n + 85) << 8) + wave(n + 170);
}

uint wave(uint n) {
   return int(125 * sin(n / 15) + 126);
}