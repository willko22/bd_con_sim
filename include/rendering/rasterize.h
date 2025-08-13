#pragma once

#include <GLFW/glfw3.h>
#include "entities/objects.h"

// Initialize the rasterizer (sets up shaders, buffers, etc.)
bool rasterize_init();

// Clean up rasterizer resources
void rasterize_cleanup();

// Draw a basic triangle with different colors at each corner
// Triangle will be centered on the display
void draw_test_triangle();

// Draw a polygon using the Polygon class
void draw_polygon(const objects::Polygon& polygon);

// Draw a rectangle using the Rectangle class
void draw_rectangle(const objects::Rectangle& rectangle);
