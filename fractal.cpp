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

int convergenceSpeed[SIZE][SIZE];

struct Point {
  float x, y;
};

Point c = { -0.6, -0.39 };
int zoom = window_size / 4;
int roundOffPrecision = 1;

void events(SDL_Window *window);

void draw(SDL_Renderer *renderer);

int computeConvergenceForPoint(int x, int y);

void computeConvergence();

float roundOff(float value);

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
  computeConvergence();

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
    if (event.type == SDL_QUIT) {
      quit = true;
    }
    // code for zooming
    else if (event.type == SDL_KEYDOWN) {
      SDL_Keycode keycode = event.key.keysym.sym;

      switch (keycode) {
      // PgUp is zooming in
      case SDLK_PAGEUP:
        printf("PgUp key pressed\n");
        zoom *= 2;

        computeConvergence();
        break;
      // PgDown is zooming out
      case SDLK_PAGEDOWN: 
        printf("PgDown key pressed\n");
        zoom /= 2;

        computeConvergence();
        break;
      // 0 is for resetting precision
      case SDLK_0:
        printf("0 key pressed\n");
        roundOffPrecision = 1;

        computeConvergence();
        break;
      // 1 is for increasing precision
      case SDLK_1:
        printf("1 key pressed\n");
        roundOffPrecision++;

        computeConvergence();
        break;
      }
    }
  }
}

void draw(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 255, 236, 179, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 100, 221, 23, 255);
  // x axis
  SDL_RenderDrawLine(renderer, 0, window_size / 2, window_size, window_size / 2);
  // y axis
  SDL_RenderDrawLine(renderer, window_size / 2, 0, window_size / 2, window_size);

  // draw the fractal based on convergenceSpeed
  for (int x = 0; x < window_size; x++) {
    for (int y = 0; y < window_size; y++) {
      if (convergenceSpeed[x][y] > 0) {
        // set the color using rainbow funciton
        uint color = rainbow(convergenceSpeed[x][y]);

        // decode the color to RGB values
        unsigned int red   = (color & 0x00ff0000) >> 16;
        unsigned int green = (color & 0x0000ff00) >> 8;
        unsigned int blue  = (color & 0x000000ff);

        SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawPoint(renderer, x, y);
      }
    }
  }
}

int computeConvergenceForPoint(int x, int y) {
  // initial Z
  Point Z0 = {
    ((float)(x) - (float)(window_size) / 2) / zoom,
    -((float)(y) - (float)(window_size) / 2) / zoom
  };

  // current Z
  Point Z1 = {
    Z0.x * Z0.x - Z0.y * Z0.y + c.x,
    2 * Z0.x * Z0.y + c.y
  };

  // next Z
  Point Z2;

  for (int i = 1; i <= 70; i++) {
    Z2.x = Z1.x * Z1.x - Z1.y * Z1.y + c.x;
    Z2.y = 2 * Z1.x * Z1.y + c.y;

    // First check if it converged
    // If Z1 = Z2 then it converged
    if (roundOff(Z1.x) == roundOff(Z2.x) && roundOff(Z1.y) == roundOff(Z2.y)) return i;

    // Second check if it converged
    // There is a case in which Z starts alternating, that means Z0 = Z2
    if (roundOff(Z0.x) == roundOff(Z2.x) && roundOff(Z0.y) == roundOff(Z2.y)) return i;

    // Check if it diverged
    if (abs(Z2.x) > 10000000 || abs(Z2.y) > 1000000) return 0;

    Z0 = Z1;
    Z1 = Z2;
  }

  return 71;
}

void computeConvergence() {
  for (int x = 0; x < window_size; x++) {
    for (int y = 0; y < window_size; y++) {
      convergenceSpeed[x][y] = computeConvergenceForPoint(x, y);
    }
  }
}

float roundOff(float value) {
  float pow_10 = pow(10.0f, (float)roundOffPrecision);
  return round(value * pow_10);
}

uint rainbow(uint n) {
  return (wave(n) << 16) + (wave(n + 85) << 8) + wave(n + 170);
}

uint wave(uint n) {
   return int(125 * sin(n / 15) + 126);
}