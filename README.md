# About fractals-madness

A C++ program that aims to generate the Mandelbrot Set and other fractals based on a recurence of type Zn = Zn-1^2 + c.

# Prerequisites:

In order to run this you need g++.

# Compiling and running:

```
# Windows
g++ -std=c++11 src/*.cpp -Iinclude -Ilib/boost -Ilib/SDL2/x86/include -lmingw32 -lSDL2main -lSDL2 -Llib/SDL2/x86/lib -o fractal

# Linux
g++ src/*.cpp -Ilib/boost `sdl2-config --libs` -o fractal && ./fractal
```

# Available commands:

- Scroll Up - zoom in
- Scroll Down - zoom out
- R - reset zoom and prerendering
- P - toggle prerendering on or off

## Prerendering

Prerendering will precompute the first 50 (default) zooms on the location of the cursor at the moment of turning it on. When turned off it will simply redraw the fractal in the initial position.

## Note: Do not spam commands.
