#include "utils/globals.h"
#include "entities/objects.h"
#include "utils/functions.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <cmath>
#include <algorithm>


size_t trig_table_size = 0; // Initialize size to 0
std::vector<std::pair<float, float>> trig_table; // Holds pairs of (sin, cos) for angles

// Global variable definitions (actual memory allocation)
bool enable_vsync = true;
float rotation_speed = 1.0f; // Default rotation speed for objects (radians per second)

float screen_width = 800.0f;  // Default width of the rendering window
float screen_height = 600.0f; // Default height of the rendering window

// World coordinate system (resolution independent)
float world_width = 1600.0f;   // World width in world units (constant, like 1600 units wide)
float world_height = 1200.0f;  // World height in world units (constant, like 1200 units tall)
float world_scale = 1.0f;      // Scale factor from world to screen coordinates
float world_offset_x = 0.0f;   // X offset for centering world in screen
float world_offset_y = 0.0f;   // Y offset for centering world in screen (anchor bottom center)

float rectangle_speed = 60.0f; // Default speed for rectangle movement (in world units per second)

// Define the global render_order variable with multiple layers
std::vector<std::vector<objects::Rectangle*>> render_order(2); // Layer 0: background, Layer 1: rectangles
std::vector<std::unique_ptr<objects::Rectangle>> rectangles; // Unique pointers to rectangles
int rectangle_count = 0; // Initialize rectangle count

// World background rectangle (global variable)
std::unique_ptr<objects::Rectangle> world_background = nullptr;

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
std::vector<std::unique_ptr<objects::Rectangle>> spawn_rectangles(float screen_x, float screen_y, bool randomize) {
    std::vector<std::unique_ptr<objects::Rectangle>> new_rectangles;

    // Convert screen coordinates to world coordinates
    float world_x = screen_to_world_x(screen_x);
    float world_y = screen_to_world_y(screen_y);

    constexpr static int min_c = 10000;
    constexpr static int max_c = 10000;
    constexpr static float radius_max = 150.0f; // Maximum radius from click point (in world units)
    constexpr static float radius_min = 50.0f;  // Minimum radius from click point (in world units)

    size_t count = rand() % (max_c - min_c + 1) + min_c; // Random count between 50 and 50
    new_rectangles.reserve(count);
    rectangle_count += count; // Update global rectangle count
    
    for (size_t i = 0; i < count; ++i) {
        // Random position and size (in world coordinates)
        float rect_world_x, rect_world_y;
        if (randomize) {
            // Use polar coordinates for circular distribution
            // Random radius between min and max (annular distribution)
            float radius = radius_min + (radius_max - radius_min) * (static_cast<float>(rand()) / RAND_MAX);
            // Random angle in radians
            float angle = 2.0f * M_PI * (static_cast<float>(rand()) / RAND_MAX);
            
            // Convert polar to cartesian coordinates (world space)
            rect_world_x = world_x + radius * std::cos(angle);
            rect_world_y = world_y + radius * std::sin(angle);
            
            // Clamp to world boundaries
            rect_world_x = std::max(0.0f, std::min(world_width, rect_world_x));
            rect_world_y = std::max(0.0f, std::min(world_height, rect_world_y));
        } else {
            rect_world_x = 0.0f;
            rect_world_y = 0.0f;
        }

        float world_width_rect = 10.0f; // Rectangle width in world units
        float world_height_rect = 10.0f; // Rectangle height in world units
        
        // Random color
        Color<u8> color(rand() % 256, rand() % 256, rand() % 256, 255);

        // Calculate velocity from mouse position to rectangle position (in world space)
        // Normalize the velocity and scale it by rectangle_speed for consistent movement speed
        float velocity_x = (rect_world_x - world_x);
        float velocity_y = (rect_world_y - world_y);
        
        // Normalize velocity to unit vector and scale by speed
        float velocity_magnitude = std::sqrt(velocity_x * velocity_x + velocity_y * velocity_y);
        if (velocity_magnitude > 0.0f) {
            velocity_x = (velocity_x / velocity_magnitude) * rectangle_speed;
            velocity_y = (velocity_y / velocity_magnitude) * rectangle_speed;
        } else {
            velocity_x = 0.0f;
            velocity_y = 0.0f;
        }

        // Store rectangle at the CLICK position as initial position (world coordinates)
        // The rectangle will start at the click point and then move with velocity
        
        // Adjust position so rectangle center is at the click point
        float initial_world_x = world_x - world_width_rect / 2.0f;
        float initial_world_y = world_y - world_height_rect / 2.0f;
        
        // Create rectangle using initial world coordinates (at click position)
        auto rect = std::make_unique<objects::Rectangle>(initial_world_x, initial_world_y, world_width_rect, world_height_rect, color);
        
        // Disable rotation for rectangles
        rect->should_rotate = false;
        
        // Set spawn time for GPU-side time-based calculations
        rect->spawn_time = static_cast<float>(glfwGetTime());
        
        // Set random initial angles for GPU-side rotation calculation
        rect->initial_pitch = 2.0f * M_PI * (static_cast<float>(rand()) / RAND_MAX);
        rect->initial_yaw = 2.0f * M_PI * (static_cast<float>(rand()) / RAND_MAX);
        rect->initial_roll = 2.0f * M_PI * (static_cast<float>(rand()) / RAND_MAX);
        
        // Set initial velocity based on world coordinates (velocity is in world units)
        rect->setVelocity(velocity_x, velocity_y);
        rect->move = true; // Enable movement for physics simulation
        rect->setSpeed(rectangle_speed); // Set default speed for movement (world units per second)

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
            
            // Move new rectangles to the global vector and add pointers to render order (layer 1)
            for (auto& rect : new_rectangles) {
                objects::Rectangle* rect_ptr = rect.get();
                rectangles.push_back(std::move(rect));
                render_order[1].push_back(rect_ptr); // Add to layer 1 (rectangle layer)
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

// World coordinate system functions
void update_world_transform(float screen_w, float screen_h) {
    // Update screen dimensions
    screen_width = screen_w;
    screen_height = screen_h;
    
    // Calculate scale to fit world into screen, maintaining aspect ratio
    float scale_x = screen_w / world_width;
    float scale_y = screen_h / world_height;
    world_scale = std::min(scale_x, scale_y); // Use smaller scale to fit everything
    
    // Calculate offsets to center the world in the screen
    // For bottom-center anchoring: center horizontally, anchor to bottom
    world_offset_x = (screen_w - (world_width * world_scale)) * 0.5f; // Center horizontally
    world_offset_y = 0.0f; // Anchor to bottom (no offset needed with bottom anchoring)
}

// Initialize world background rectangle
void initialize_world_background() {
    if (!world_background) {
        // Create a background rectangle that covers the entire world
        Color<u8> background_color(0, 100, 120, 255); // Dark blue-gray background
        world_background = std::make_unique<objects::Rectangle>(0.0f, 0.0f, world_width, world_height, background_color);
        
        // Add to layer 0 (background layer)
        render_order[0].push_back(world_background.get());
        
        // Background doesn't move or rotate
        world_background->move = false;
        world_background->spawn_time = 0.0f; // Always existed
        world_background->should_rotate = false; // Disable rotation for background
        world_background->move = false; // No movement for background
    }
}

// Bounds checking - remove rectangles that are fully outside world bounds
void remove_out_of_bounds_rectangles() {
    float current_time = static_cast<float>(glfwGetTime());
    
    // Iterate through rectangles and remove those fully outside world bounds
    for (auto& rect : rectangles) {
        if (!rect->should_render) continue; // Skip if already marked for removal
        
        // Calculate current position using GPU movement formula: position = initial_position + (velocity * time)
        float time_since_spawn = current_time - rect->spawn_time;
        float current_center_x = rect->center.x + (rect->velocity.x * rect->speed * time_since_spawn);
        float current_center_y = rect->center.y + (rect->velocity.y * rect->speed * time_since_spawn);
        
        // Treat rectangle as circle with radius = max(width, height) / 2
        float radius = rect->bbox.radius;
        
        // Check if circle is completely outside world bounds
        bool outside_world = (current_center_x <= -radius) ||           // Completely to the left
                            (current_center_x - radius >= world_width) ||      // Completely to the right
                            (current_center_y <= -radius) ||             // Completely below
                            (current_center_y - radius >= world_height);       // Completely above
        
        if (outside_world) {
            // Remove from render_order first
            rect->should_render = false; // Set should_render to false
            rectangle_count--;
        }
    }
}