#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <utility> // For std::pair

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using u16 = uint16_t;  // 0-65k range - fine for dimensions, colors
using u8 = uint8_t;    // 0-255 range - perfect for counters, colors
using u32 = uint32_t;  // 0-4B range - for calculations that might overflow u16
using i32 = int32_t;   // Signed 32-bit for offset calculations
using i16 = int16_t;   // -32k to +32k range - fine for screen coordinates
using Mat3 = std::array<float, 9>;
using Mat2 = std::array<float, 4>;

inline constexpr static float TWO_PI = 2.0f * static_cast<float>(M_PI);
inline constexpr static float INV_TWO_PI = 1.0f / TWO_PI;


// Forward declaration to avoid circular dependency
namespace objects {
    struct Rectangle;
}

// Global variables accessible across the entire application
extern bool enable_vsync;
extern float rotation_speed; // Speed of rotation for objects (radians per second)
extern float screen_width;  // Width of the rendering window (float for high-DPI support)
extern float screen_height; // Height of the rendering window (float for high-DPI support)

// World coordinate system (resolution independent)
extern float world_width;   // World width in world units (constant)
extern float world_height;  // World height in world units (constant)
extern float world_scale;   // Scale factor from world to screen coordinates
extern float world_offset_x; // X offset for centering world in screen
extern float world_offset_y; // Y offset for centering world in screen (anchor bottom center)

extern std::vector<std::vector<objects::Rectangle*>> render_order; // Track render order for rectangles
extern std::vector<std::unique_ptr<objects::Rectangle>> rectangles; // Unique pointers to rectangles
extern int rectangle_count; // Global count of rectangles for easy access

extern std::unique_ptr<objects::Rectangle> world_background; // World background rectangle

extern float rectangle_speed; // Default speed for rectangle movement (in world units per second)

// Mouse state tracking for hold functionality
extern bool left_mouse_held;
extern bool right_mouse_held;
extern bool middle_mouse_held;
extern float mouse_current_x, mouse_current_y;       // Current mouse position
extern double mouse_hold_duration;                    // How long mouse has been held (seconds)

// Precomputed sine and cosine values for angles
extern std::vector<std::pair<float, float>> trig_table; // Holds pairs of (sin, cos) for angles
extern size_t trig_table_size; // Size of the trig_table for indexing


extern std::vector<Mat2> mat_table;

std::vector<std::unique_ptr<objects::Rectangle>> spawn_rectangles(float x, float y, bool randomize);


// Mouse hold utility functions
void update_mouse_hold_duration(double delta_time);
void handle_mouse_hold_continuous();

void precompute_trig_angles();
size_t angle_to_index(float angle);

// World coordinate system functions
void update_world_transform(float screen_w, float screen_h);

inline float world_to_screen_x(float world_x) {
    return world_x * world_scale + world_offset_x;
}

inline float world_to_screen_y(float world_y) {
    // Flip Y coordinate for screen space (screen Y=0 is top, world Y=0 is bottom)
    return screen_height - (world_y * world_scale + world_offset_y);
}

inline float screen_to_world_x(float screen_x) {
    return (screen_x - world_offset_x) / world_scale;
}

inline float screen_to_world_y(float screen_y) {
    // Flip Y coordinate back to world space
    return (screen_height - screen_y - world_offset_y) / world_scale;
}

inline float world_to_screen_scale(float world_size) {
    return world_size * world_scale;
}

inline float screen_to_world_scale(float screen_size) {
    return screen_size / world_scale;
}

// World coordinate and bounds management functions
void update_world_transform(float screen_w, float screen_h);
void initialize_world_background();
void remove_out_of_bounds_rectangles();

// Game state globals (you can add more as needed)
// extern bool game_paused;
// extern float delta_time;
// extern int current_level;

