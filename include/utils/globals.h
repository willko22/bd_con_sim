#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <utility>
#include <array>
#include <random>

// ########## MATHEMATICAL CONSTANTS ##########

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline constexpr static float TWO_PI = 2.0f * static_cast<float>(M_PI);
inline constexpr static float INV_TWO_PI = 1.0f / TWO_PI;

// ########## TYPE ALIASES ##########

// Integer types with explicit size and range comments
using u8 = uint8_t;    // 0-255 range - for colors, small counters
using u16 = uint16_t;  // 0-65k range - for dimensions, entity IDs
using u32 = uint32_t;  // 0-4B range - for large calculations
using i16 = int16_t;   // -32k to +32k range - for screen coordinates  
using i32 = int32_t;   // Signed 32-bit for offset calculations

// Matrix types for transformations
using Mat2 = std::array<float, 4>;
using Mat3 = std::array<float, 9>;


// ########## FORWARD DECLARATIONS ##########

namespace obj {
    struct Rectangle;
}

// ########## Physics ##########
// Physics simulation constants
extern const float GRAVITY_ACCELERATION;  // Gravitational acceleration in world units/s²
extern const float DRAG_COEFF;  // Air drag coefficient
extern const float AIR_DENSITY;           // Density of air at sea level (kg/m³)
extern const float DEFAULT_MASS;          // Default mass for rectangles
extern const float EXPLOSION_STRENGTH;    // Initial explosion strength when spawning rectangles
extern const float FLUTTER_STRENGTH;      // Horizontal oscillation force strength
extern const float FLUTTER_SPEED;         // Speed of flutter effect oscillation

// Other constants
extern const float MOUSE_RADIUS;
extern const float ROTATION_SPEED;        // Rotation speed in radians per second


extern const float BG_COLOR_R; // Background color components
extern const float BG_COLOR_G;
extern const float BG_COLOR_B;


extern const float RECT_WIDTH;   // Rectangle width in world units
extern const float RECT_HEIGHT;  // Rectangle height in world units


extern const float METERS_TO_WORLD; // Conversion factor from meters to world units
extern const float WORLD_TO_METERS; // Conversion factor from world units to meters


// ########## RANDOM NUMBER GENERATION ##########
extern std::mt19937 random_engine;
extern std::uniform_real_distribution<float> random_angle; // Random angle distribution
extern std::uniform_real_distribution<float> random_radius; // Random radius distribution
extern std::uniform_real_distribution<float> random_impuls_increase; // Random decay variation


// ########## GRAPHICS AND RENDERING ##########

// V-sync and display settings
extern bool enable_vsync;
extern float screen_width;   // Current screen width in pixels
extern float screen_height;  // Current screen height in pixels

// ========== World Coordinate System ==========

// World dimensions and transformation
extern float world_width;    // World width in world units (constant)
extern float world_height;   // World height in world units (constant) 
extern float world_scale;    // Scale factor: world to screen coordinates
extern float world_offset_x; // X offset for centering world in screen
extern float world_offset_y; // Y offset for screen positioning

// ########## ENTITY MANAGEMENT ##########

// Rectangle storage and rendering
extern std::vector<std::vector<obj::Rectangle*>> render_order;
extern std::vector<std::unique_ptr<obj::Rectangle>> rectangles;
extern std::vector<obj::Rectangle*> activeRects;
extern std::vector<obj::Rectangle*> settledRects;
extern int rectangle_count;
extern std::unique_ptr<obj::Rectangle> world_background;

extern size_t layer_rectangles;

// ========== Entity Properties ==========
// ########## INPUT HANDLING ##########

// Mouse state tracking
extern bool left_mouse_held;
extern bool right_mouse_held; 
extern bool middle_mouse_held;
extern float mouse_current_x;
extern float mouse_current_y;
extern double mouse_hold_duration; // Duration in seconds

// ########## PERFORMANCE OPTIMIZATION ##########

// ========== Trigonometry Look-up Tables ==========

extern std::vector<std::pair<float, float>> trig_table; // (sin, cos) pairs
extern size_t trig_table_size;
extern std::vector<Mat2> mat_table;

// ########## FUNCTION DECLARATIONS ##########

// ========== Entity Creation ==========

// Spawn rectangles at specified position
void spawn_rectangles(float x, float y);

// ========== Input Processing ==========

// Mouse hold utility functions  
void update_mouse_hold_duration(double delta_time);
void handle_mouse_hold_continuous();

// ========== Performance Utilities ==========

// Trigonometry optimization
void precompute_trig_angles();
size_t angle_to_index(float angle);

// ========== Coordinate Transformations ==========

// World coordinate system functions
void update_world_transform(float screen_w, float screen_h);

// ========== Inline Coordinate Conversion Functions ==========

// Convert world coordinates to screen coordinates
inline float world_to_screen_x(float world_x) {
    return world_x * world_scale + world_offset_x;
}

inline float world_to_screen_y(float world_y) {
    // Flip Y coordinate: screen Y=0 is top, world Y=0 is bottom
    return screen_height - (world_y * world_scale + world_offset_y);
}

// Convert screen coordinates to world coordinates  
inline float screen_to_world_x(float screen_x) {
    return (screen_x - world_offset_x) / world_scale;
}

inline float screen_to_world_y(float screen_y) {
    // Vertex shader handles Y-flip, so no flip needed here
    return (screen_y - world_offset_y) / world_scale;
}

// Convert scale between coordinate systems
inline float world_to_screen_scale(float world_size) {
    return world_size * world_scale;
}

inline float screen_to_world_scale(float screen_size) {
    return screen_size / world_scale;
}

// World coordinate and bounds management functions
void update_world_transform(float screen_w, float screen_h);
void remove_out_of_bounds_rectangles();

// Game state globals (you can add more as needed)
// extern bool game_paused;
// extern float delta_time;
// extern int current_level;

