#include "utils.h"

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