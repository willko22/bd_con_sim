#include "utils/globals.h"
#include "entities/objects.h"
#include "utils/functions.h"
#include <memory>
#include <cmath>


size_t trig_table_size = 0; // Initialize size to 0
std::vector<std::pair<float, float>> trig_table; // Holds pairs of (sin, cos) for angles

// Global variable definitions (actual memory allocation)
bool enable_vsync = true;
float rotation_speed = 1.0f; // Default rotation speed for objects (radians per second)

float screen_width = 800.0f;  // Default width of the rendering window
float screen_height = 600.0f; // Default height of the rendering window

// Define the global render_order variable
std::vector<std::vector<objects::Rectangle*>> render_order(1); // Initialize with at least one empty vector
std::vector<std::unique_ptr<objects::Rectangle>> rectangles; // Unique pointers to rectangles
int rectangle_count = 0; // Initialize rectangle count

// Mouse state tracking for hold functionality
bool left_mouse_held = false;
bool right_mouse_held = false;
bool middle_mouse_held = false;
float mouse_current_x = 0.0f, mouse_current_y = 0.0f;       // Current mouse position
double mouse_hold_duration = 0.0;                            // How long mouse has been held (seconds)

void precompute_trig_angles() {
    // Precompute sine and cosine values for angles 0 rad to 2*PI
    // 0.01 rad increments for smoothness
    const float increment = 0.001f;
    const size_t num_angles = static_cast<size_t>(TWO_PI / increment) + 1;
    
    trig_table.resize(num_angles);
    for (size_t i = 0; i < num_angles; ++i) {
        float angle = i * increment;
        trig_table[i].first = std::sin(angle);
        trig_table[i].second = std::cos(angle);
    }

    trig_table_size = trig_table.size();
}

inline size_t angle_to_index(float angle) {
    if (trig_table_size <= 0) return 0; // Guard against division by zero
    
    // Normalize angle to [0, 2π) range using fmod for better precision
    // angle = std::fmod(angle, TWO_PI);
    if (angle < 0) angle += TWO_PI;  // Handle negative angles
    
    // Convert to index
    float normalized = angle * INV_TWO_PI;  // Now guaranteed to be in [0, 1)
    size_t index = static_cast<size_t>(normalized * static_cast<float>(trig_table_size - 1));
    
    // Ensure index is always within bounds
    if (index >= trig_table_size) index = trig_table_size - 1;
    
    return index;
}



// Global variable definitions with default values
std::vector<std::unique_ptr<objects::Rectangle>> spawn_rectangles(float x, float y, bool randomize) {
    std::vector<std::unique_ptr<objects::Rectangle>> new_rectangles;

    constexpr static int min_c = 10000;
    constexpr static int max_c = 10000;
    constexpr static float radius_max = 100.0f; // Maximum radius from click point
    constexpr static float radius_min = 30.0f; // Minimum radius from click point

    size_t count = rand() % (max_c - min_c + 1) + min_c; // Random count between 4 and 8
    new_rectangles.reserve(count);
    rectangle_count += count; // Update global rectangle count
    
    for (size_t i = 0; i < count; ++i) {
        // Random position and size
        float rect_x, rect_y;
        if (randomize) {
            // Use polar coordinates for circular distribution
            // Random radius between min and max (annular distribution)
            float radius = radius_min + (radius_max - radius_min) * (static_cast<float>(rand()) / RAND_MAX);
            // Random angle in radians
            float angle = 2.0f * M_PI * (static_cast<float>(rand()) / RAND_MAX);
            
            // Convert polar to cartesian coordinates
            rect_x = x + radius * std::cos(angle);
            rect_y = y + radius * std::sin(angle);
            
            // Clamp to screen boundaries
            rect_x = std::max(0.0f, std::min(static_cast<float>(screen_width), rect_x));
            rect_y = std::max(0.0f, std::min(static_cast<float>(screen_height), rect_y));
        } else {
            rect_x = 0.0f;
            rect_y = 0.0f;
        }

        float width = 20.0f; // Random width between 20 and 120
        float height = 20.0f; // Random height between 20 and 120
        // Random color
        Color<u8> color(rand() % 256, rand() % 256, rand() % 256, 255);
        
        // Adjust position so rectangle center is at the click point, not top-left corner
        float adjusted_x = rect_x - width / 2.0f;
        float adjusted_y = rect_y - height / 2.0f;
        
        // Create rectangle as unique pointer (now using pure float coordinates!)
        auto rect = std::make_unique<objects::Rectangle>(adjusted_x, adjusted_y, width, height, color);
        
        // Set random initial angles for GPU-side rotation calculation
        rect->initial_pitch = 2.0f * M_PI * (static_cast<float>(rand()) / RAND_MAX);
        rect->initial_yaw = 2.0f * M_PI * (static_cast<float>(rand()) / RAND_MAX);
        rect->initial_roll = 2.0f * M_PI * (static_cast<float>(rand()) / RAND_MAX);
        
        // Add to vector
        new_rectangles.push_back(std::move(rect));
    }

    return new_rectangles;

}

// Mouse hold utility functions
void update_mouse_hold_duration(double delta_time) {
    if (left_mouse_held || right_mouse_held || middle_mouse_held) {
        mouse_hold_duration += delta_time;
    } else {
        mouse_hold_duration = 0.0;
    }
}

void handle_mouse_hold_continuous() {
    // Continuous behavior while mouse is held
    static double last_spawn_time = 0.0;
    
    if (left_mouse_held && mouse_hold_duration > 0.5) { // After 0.5 seconds of holding
        // Spawn rectangles continuously every 0.1 seconds
        if (mouse_hold_duration - last_spawn_time > 0.1) {
            auto new_rectangles = spawn_rectangles(mouse_current_x, mouse_current_y, true);
            
            // Move new rectangles to the global vector and add pointers to render order
            for (auto& rect : new_rectangles) {
                objects::Rectangle* rect_ptr = rect.get();
                rectangles.push_back(std::move(rect));
                render_order[0].push_back(rect_ptr);
                // Note: rectangle_count is already incremented in spawn_rectangles
            }
            
            last_spawn_time = mouse_hold_duration;
        }
    }
    
    // Reset spawn timer when mouse is released
    if (!left_mouse_held) {
        last_spawn_time = 0.0;
    }
}