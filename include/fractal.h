#pragma once
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "SDL2/SDL.h"
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

struct Point {
  FLOAT x, y;
};

// the depth the program goes to determine the divergence of a point
extern int ITERATIONS;
extern uint16_t window_size;
// the matrix which stores the divergence speed of each point
extern int divergenceSpeed[SIZE][SIZE];
// the matrix which stores the divergence speed of the first zooms
extern int precomputedDivergenceSpeed[ZOOMS][SIZE][SIZE];
extern bool precompute;
// the point on which to zoom
extern Point zoomPosition;
// the point on which it zoomed last time
extern Point lastZoomPosition;
// the scale
extern FLOAT scale;
// number of times the fractal has been zoomed in or out relative to the initial position
extern int k;

// this function generates the Mandelbrot Set
int generateMandelbrotSet(int x, int y);

// this function generates a fractal based on a constant c
int generateFractal(Point c, int x, int y);

// this function takes each point from the visible plane and checks if it converges or diverges
void computeDivergenceSpeed();

// this function precomputes the first ZOOMS zooms
void precomputeDivergenceSpeed(int mouseX, int mouseY);