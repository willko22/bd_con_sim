#pragma once
#include "utils/globals.h"

#include "entities/objects.h"

// Initialize the rasterizer (sets up shaders, buffers, etc.)
bool rasterize_init();

// Clean up rasterizer resources
void rasterize_cleanup();

// Update cached viewport dimensions for performance
void update_viewport_cache(int width, int height);

// Begin optimized batch rendering (sets up OpenGL state once)
void begin_batch_render();

// End optimized batch rendering (cleans up OpenGL state)
void end_batch_render();

// Draw a basic triangle with different colors at each corner
// Triangle will be centered on the display
void draw_test_triangle();

// Draw a polygon using the Polygon class
void draw_polygon(const obj::Polygon &polygon);

// Draw a rectangle using the Rectangle class
void draw_rectangle(const obj::Rectangle &rectangle);

// Batch rendering functions for performance
void batch_draw_rectangles(const std::vector<obj::Rectangle *> &rectangles);

// Instanced rendering for maximum performance
void instanced_draw_rectangles(const std::vector<obj::Rectangle *> &rectangles,
                               bool isBackground);

// Debug function to draw red dots at rectangle centers
void draw_center_dots(const std::vector<obj::Rectangle *> &rectangles);
