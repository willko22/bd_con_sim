#include "utils/globals.h"
#include "entities/objects.h"
#include "utils/functions.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <cmath>
#include <algorithm>
#include <random>


// ########## GLOBAL VARIABLE DEFINITIONS ##########

// Physics constants
const float GRAVITY = 2.5f; // Speed of changing velocity vector to downward direction
const float ROTATION_SPEED = 1.0f;    // Rotation speed in radians per second
const float RECTANGLE_SPEED = 30.0f; // Movement speed in world units per second
const float SPAWN_SPEED = 350.0f; // Speed of spawning rectangles
const float DECAY_RATE = 4.0f;

const float MOUSE_RADIUS = 6.0f; // Radius for mouse interaction



// random number generation
std::mt19937 random_engine(std::random_device{}());
std::uniform_real_distribution<float> random_angle(0.0f, TWO_PI);
std::uniform_real_distribution<float> random_radius(MOUSE_RADIUS, MOUSE_RADIUS * 2);
std::uniform_real_distribution<float> random_decay_var(-2.0f, 2.0f);

// Graphics and rendering
bool enable_vsync = true;
float screen_width = 800.0f;
float screen_height = 600.0f;

// World coordinate system
float world_width = 1000.0f;    // World width in world units
float world_height = 600.0f;   // World height in world units  
float world_scale = 1.0f;       // Scale factor from world to screen
float world_offset_x = 0.0f;    // X offset for centering
float world_offset_y = 0.0f;    // Y offset for positioning

constexpr float RECT_WIDTH = 6.0f;            // Rectangle width in world units
constexpr float RECT_HEIGHT = 6.0f;           // Rectangle height in world units

// Rectangle storage and rendering layers (0: background, 1: rectangles)
std::vector<std::vector<objects::Rectangle*>> render_order(1);
std::vector<std::unique_ptr<objects::Rectangle>> rectangles;
int rectangle_count = 0;
std::unique_ptr<objects::Rectangle> world_background = nullptr;

size_t layer_rectangles = 0;

// Input state
bool left_mouse_held = false;
bool right_mouse_held = false;
bool middle_mouse_held = false;
float mouse_current_x = 0.0f;
float mouse_current_y = 0.0f;
double mouse_hold_duration = 0.0;

// Performance tables
std::vector<std::pair<float, float>> trig_table; // (sin, cos) pairs for angles
size_t trig_table_size = 0;

// ########## TRIGONOMETRY OPTIMIZATION ##########

void precompute_trig_angles() {
    // Precompute sine and cosine values for 0 to 2π radians
    // Using 0.001 rad increments for smooth interpolation
    constexpr float ANGLE_INCREMENT = 0.001f;
    const size_t num_angles = static_cast<size_t>(TWO_PI / ANGLE_INCREMENT) + 1;
    
    trig_table.resize(num_angles);
    for (size_t i = 0; i < num_angles; ++i) {
        float angle = i * ANGLE_INCREMENT;
        trig_table[i] = std::make_pair(std::sin(angle), std::cos(angle));
    }
    
    trig_table_size = trig_table.size();
}

size_t angle_to_index(float angle) {
    if (trig_table_size == 0) return 0; // Safety check
    
    // Normalize angle to [0, 2π) range
    if (angle < 0) angle += TWO_PI;
    
    // Convert to table index
    float normalized = angle * INV_TWO_PI;
    size_t index = static_cast<size_t>(normalized * static_cast<float>(trig_table_size - 1));
    
    // Ensure index stays within bounds
    return std::min(index, trig_table_size - 1);
}



// ########## RECTANGLE SPAWNING ##########

std::vector<std::unique_ptr<objects::Rectangle>> 
spawn_rectangles(float screen_x, float screen_y, bool randomize) {
    std::vector<std::unique_ptr<objects::Rectangle>> new_rectangles;

    // Convert screen coordinates to world coordinates
    float world_x = screen_to_world_x(screen_x);
    float world_y = screen_to_world_y(screen_y);

    // Validate spawn position is within world bounds
    if (world_x < 0.0f || world_x > world_width || 
        world_y < 0.0f || world_y > world_height) {
        return {}; // Return empty vector if outside bounds
    }

    // Spawn configuration constants
    constexpr int SPAWN_COUNT = 100;                // Number of rectangles to spawn4

    new_rectangles.reserve(SPAWN_COUNT);
    rectangle_count += SPAWN_COUNT;
    
    for (int i = 0; i < SPAWN_COUNT; ++i) {
        float rect_world_x, rect_world_y;
        
        if (randomize) {
            // Use polar coordinates for circular distribution around click point
            float radius = 0.01f;
            float angle = random_angle(random_engine);
            
            // Convert polar to cartesian coordinates
            rect_world_x = world_x + radius * std::cos(angle);
            rect_world_y = world_y + radius * std::sin(angle);
            
            // Clamp to world boundaries
            rect_world_x = std::max(0.0f, std::min(world_width, rect_world_x));
            rect_world_y = std::max(0.0f, std::min(world_height, rect_world_y));
        } else {
            rect_world_x = 0.0f;
            rect_world_y = 0.0f;
        }

        // Generate random color
        Color<u8> color(rand() % 256, rand() % 256, rand() % 256, 255);

        float dir_x = rect_world_x - world_x;
        float dir_y = rect_world_y - world_y;

        // Normalize the direction vector
        float length = std::sqrt(dir_x * dir_x + dir_y * dir_y);
        if (length != 0.0f) {
            dir_x /= length;
            dir_y /= length;
        } else {
            // fallback if the rectangle spawned exactly at the click
            dir_x = 1.0f; 
            dir_y = 0.0f;
        }


        // // edge case where directly up and no sideways movement
        // if (dir_x == 0.0f && dir_y == -1.0f) {
        //     dir_x = rand() % 2 == 0 ? 0.01f : -0.01f;
        //     dir_y = -0.99; // Prevent zero velocity
        // }

        // Position rectangle center at click point (will move with velocity)
        float initial_world_x = world_x - RECT_WIDTH / 2.0f;
        float initial_world_y = world_y - RECT_HEIGHT / 2.0f;
        
        // Create rectangle at click position
        auto rect = std::make_unique<objects::Rectangle>(
            initial_world_x, initial_world_y, RECT_WIDTH, RECT_HEIGHT, color);
        
        // Configure rectangle properties
        rect->should_rotate = true;
        rect->move = true;
        rect->spawn_time = static_cast<float>(glfwGetTime());
        rect->decay_rate = DECAY_RATE + random_decay_var(random_engine); // Add random decay variation
        
        
        // Set random initial rotation angles for GPU
        rect->initial_pitch = TWO_PI * (static_cast<float>(rand()) / RAND_MAX);
        rect->initial_yaw = TWO_PI * (static_cast<float>(rand()) / RAND_MAX);
        rect->initial_roll = TWO_PI * (static_cast<float>(rand()) / RAND_MAX);
        
        // Set velocity and speed
        rect->setDirection(dir_x, dir_y);
        rect->setSpeed(RECTANGLE_SPEED);

        new_rectangles.push_back(std::move(rect));
    }

    return new_rectangles;
}

// ########## MOUSE INPUT HANDLING ##########

void update_mouse_hold_duration(double delta_time) {
    if (left_mouse_held || right_mouse_held || middle_mouse_held) {
        mouse_hold_duration += delta_time;
    } else {
        mouse_hold_duration = 0.0;
    }
}

void handle_mouse_hold_continuous() {
    constexpr double HOLD_THRESHOLD = 0.5;    // Start continuous spawn after 0.5 seconds
    constexpr double SPAWN_INTERVAL = 0.1;    // Spawn every 0.1 seconds during hold
    
    static double last_spawn_time = 0.0;
    
    if (left_mouse_held && mouse_hold_duration > HOLD_THRESHOLD) {
        // Spawn rectangles continuously at intervals
        if (mouse_hold_duration - last_spawn_time > SPAWN_INTERVAL) {
            auto new_rectangles = spawn_rectangles(mouse_current_x, mouse_current_y, true);
            
            // Add new rectangles to global storage and render order (layer 1)
            for (auto& rect : new_rectangles) {
                objects::Rectangle* rect_ptr = rect.get();
                rectangles.push_back(std::move(rect));
                render_order[layer_rectangles].push_back(rect_ptr);
            }
            
            last_spawn_time = mouse_hold_duration;
        }
    }
    
    // Reset spawn timer when mouse is released
    if (!left_mouse_held) {
        last_spawn_time = 0.0;
    }
}

// ########## WORLD COORDINATE SYSTEM ##########

void update_world_transform(float screen_w, float screen_h) {
    // Update screen dimensions
    screen_width = screen_w;
    screen_height = screen_h;
    
    // Scale world to fit screen height perfectly
    // Width may be cropped or stretched based on aspect ratio
    world_scale = screen_h / world_height;
    
    // Center world horizontally, anchor to bottom vertically
    float scaled_world_width = world_width * world_scale;
    world_offset_x = (screen_w - scaled_world_width) * 0.5f;
    world_offset_y = 0.0f; // Bottom anchor
}

// Out-of-bounds cleanup function

void remove_out_of_bounds_rectangles() {
    float current_time = static_cast<float>(glfwGetTime());
    
    for (auto& rect : rectangles) {
        if (!rect->should_render) continue; // Skip already marked rectangles
        
        // Calculate current position: position = initial + (velocity * time)
        float time_since_spawn = current_time - rect->spawn_time;
        float current_center_x = rect->center.x + (rect->velocity.x * rect->speed * time_since_spawn);
        float current_center_y = rect->center.y + (rect->velocity.y * rect->speed * time_since_spawn);
        
        // Convert to screen coordinates for bounds checking
        float screen_x = world_to_screen_x(current_center_x);
        float screen_y = world_to_screen_y(current_center_y);
        
        // Treat rectangle as circle with radius = max(width, height) / 2
        float screen_radius = rect->bbox.radius * world_scale;
        
        // Check if completely outside screen bounds
        bool outside_screen = (screen_x + screen_radius < 0.0f) ||           // Left of screen
                             (screen_x - screen_radius > screen_width) ||     // Right of screen  
                             (screen_y + screen_radius < 0.0f) ||             // Above screen
                             (screen_y - screen_radius > screen_height);      // Below screen
        
        if (outside_screen) {
            rect->should_render = false;
            rectangle_count--;
        }
    }
}