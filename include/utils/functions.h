#pragma once

#include "globals.h"
#include <algorithm>

// Inline utility functions

/**
 * Clips a value to be within screen boundaries (0 to screen_width/screen_height)
 * @param value The value to clip
 * @param is_x_coordinate True if this is an x-coordinate (clips to screen_width), false for y-coordinate (clips to screen_height)
 * @return The clipped value
 */
inline float clip_to_screen(float value, bool is_x_coordinate) {
    if (is_x_coordinate) {
        return std::min(value, screen_width);
    } else {
        return std::min(value, screen_height);
    }
}

// Legacy u16 version for backward compatibility
inline u16 clip_to_screen(u16 value, bool is_x_coordinate) {
    return static_cast<u16>(clip_to_screen(static_cast<float>(value), is_x_coordinate));
}

/**
 * Clips an x-coordinate to be within screen width (0 to screen_width)
 * @param x The x-coordinate to clip
 * @return The clipped x-coordinate
 */
inline float clip_x_to_screen(float x) {
    return std::min(x, screen_width);
}

// Legacy u16 version for backward compatibility
inline u16 clip_x_to_screen(u16 x) {
    return static_cast<u16>(clip_x_to_screen(static_cast<float>(x)));
}

/**
 * Clips a y-coordinate to be within screen height (0 to screen_height)
 * @param y The y-coordinate to clip
 * @return The clipped y-coordinate
 */
inline float clip_y_to_screen(float y) {
    return std::min(y, screen_height);
}

// Legacy u16 version for backward compatibility
inline u16 clip_y_to_screen(u16 y) {
    return static_cast<u16>(clip_y_to_screen(static_cast<float>(y)));
}

/**
 * Clips both x and y coordinates to screen boundaries
 * @param x Reference to x-coordinate to clip in-place
 * @param y Reference to y-coordinate to clip in-place
 */
inline void clip_coordinates_to_screen(float& x, float& y) {
    x = clip_x_to_screen(x);
    y = clip_y_to_screen(y);
}

// Legacy u16 version for backward compatibility
inline void clip_coordinates_to_screen(u16& x, u16& y) {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    clip_coordinates_to_screen(fx, fy);
    x = static_cast<u16>(fx);
    y = static_cast<u16>(fy);
}
