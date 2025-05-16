#include "utils/color.h"

color rgb(color r, color g, color b) { return ((color)r << 16) | ((color)g << 8) | b; }