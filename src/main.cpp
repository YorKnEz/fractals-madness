#include "fractal.h"
#include "utils.h"

bool quit = false;

uint fps = 60;
uint minframetime = 1000 / fps;

void events(SDL_Window *window);

void draw(SDL_Renderer *renderer);

void startAnimation(SDL_Renderer *renderer);

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

        if (precompute) {
          printf("Precomputation is on\n");

          precomputeDivergenceSpeed(mouseX, mouseY);
        } else {
          printf("Precomputation is off\n");
        }

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