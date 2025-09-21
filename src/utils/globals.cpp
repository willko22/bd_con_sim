#include "utils/globals.h"
#include "entities/objects.h"
#include "utils/functions.h"

// ########## GLOBAL VARIABLE DEFINITIONS ##########

// Physics constants
const float GRAVITY_ACCELERATION = 9.81f;
const float DRAG_COEFF = 1.00f;
const float AIR_DENSITY = 1.225f;
const float DEFAULT_MASS = .0001f;
const float EXPLOSION_STRENGTH = 400.0f; // m/s

const float MOUSE_MASS = 80.0f;   // mass for mouse interaction
const float MOUSE_RADIUS = 10.0f; // Radius for mouse interaction
const float MOUSE_DRAG = 0.1f;    // drag applied when mouse is moving

// Movement constants
const float ROTATION_SPEED = 1.0f;   // Rotation speed in radians per second
const float FLUTTER_STRENGTH = 0.5f; // Flutter effect strength
const float FLUTTER_SPEED = 1.0f;    // Flutter effect frequency

const float BG_COLOR_R = 210.0f; // Background color components
const float BG_COLOR_G = 205.0f;
const float BG_COLOR_B = 200.0f;

const float METERS_TO_WORLD = 100.0f; // 1 meter = 100 world units
const float WORLD_TO_METERS =
    1.0f / METERS_TO_WORLD; // 0.01 meters per world unit

// random number generation
std::mt19937 random_engine(std::random_device{}());
std::uniform_real_distribution<float> random_angle(0.0f, TWO_PI);
std::uniform_real_distribution<float>
    random_impuls_increase(-EXPLOSION_STRENGTH * 0.95f,
                           EXPLOSION_STRENGTH * 0.5f);
// std::uniform_real_distribution<float> random_impuls_increase(
//     0.f,0.f
// );

// Graphics and rendering
bool enable_vsync = true;
float screen_width = 800.0f;
float screen_height = 600.0f;

bool apply_gravity = true;

// Viewport system (for aspect ratio preservation)
int viewport_x = 0;        // Viewport X offset within window
int viewport_y = 0;        // Viewport Y offset within window
int viewport_width = 800;  // Viewport width in pixels
int viewport_height = 600; // Viewport height in pixels

// World coordinate system
float world_width = 720.0f;  // World width in world units
float world_height = 480.0f; // World height in world units
float world_scale = 1.0f;    // Scale factor from world to screen
float world_offset_x = 0.0f; // X offset for centering
float world_offset_y = 0.0f; // Y offset for positioning

const float RECT_WIDTH = 3.0f;  // Rectangle width in world units
const float RECT_HEIGHT = 3.0f; // Rectangle height in world units

// Rectangle width in simulation calculations
const float RECT_SIM_WIDTH = 3.0f;
// Rectangle height in simulation calculations
const float RECT_SIM_HEIGHT = 3.0f;

// Rectangle storage and rendering layers
// 0: background, 1: text, 2: rectangles
std::vector<std::vector<obj::Rectangle *>> render_order(3);
std::vector<std::unique_ptr<obj::Rectangle>> rectangles;
std::vector<obj::Rectangle *> activeRects;
std::vector<obj::Rectangle *> settledRects;
int rectangle_count = 0;
std::unique_ptr<obj::Rectangle> world_background = nullptr;

size_t layer_background = 0;
size_t layer_text = 1;
size_t layer_rectangles = 2;

std::unique_ptr<obj::Rectangle> background = nullptr;

// Input state
bool left_mouse_held = false;
bool right_mouse_held = false;
bool middle_mouse_held = false;
float mouse_current_x = 0.0f;
float mouse_current_y = 0.0f;
float mouse_world_x = 0.0f;
float mouse_world_y = 0.0f;
float mouse_world_x_prev = 0.0f;
float mouse_world_y_prev = 0.0f;
float mouse_last_t = 0.0f;
float mouse_current_t = 0.0f;
double mouse_hold_duration = 0.0;

// Performance tables
std::vector<std::pair<float, float>> trig_table; // (sin, cos) pairs for angles
size_t trig_table_size = 0;

// ########## TRIGONOMETRY OPTIMIZATION ##########

void precompute_trig_angles()
{
    // Precompute sine and cosine values for 0 to 2π radians
    // Using 0.001 rad increments for smooth interpolation
    constexpr float ANGLE_INCREMENT = 0.001f;
    const size_t num_angles = static_cast<size_t>(TWO_PI / ANGLE_INCREMENT) + 1;

    trig_table.resize(num_angles);
    for (size_t i = 0; i < num_angles; ++i)
    {
        float angle = i * ANGLE_INCREMENT;
        trig_table[i] = std::make_pair(std::sin(angle), std::cos(angle));
    }

    trig_table_size = trig_table.size();
}

size_t angle_to_index(float angle)
{
    if (trig_table_size == 0)
        return 0; // Safety check

    // Normalize angle to [0, 2π) range
    if (angle < 0)
        angle += TWO_PI;

    // Convert to table index
    float normalized = angle * INV_TWO_PI;
    size_t index = static_cast<size_t>(normalized *
                                       static_cast<float>(trig_table_size - 1));

    // Ensure index stays within bounds
    return std::min(index, trig_table_size - 1);
}

// ########## RECTANGLE SPAWNING ##########

void spawn_rectangles(float screen_x, float screen_y)
{
    std::vector<std::unique_ptr<obj::Rectangle>> new_rectangles;

    // Convert screen coordinates to world coordinates
    float world_x = screen_to_world_x(screen_x);
    float world_y = screen_to_world_y(screen_y);

    // Validate spawn position is within world bounds
    // if (world_x < 0.0f || world_x > world_width || world_y < 0.0f ||
    //     world_y > world_height)
    // {
    //     return;
    // }

    // Spawn configuration constants
    constexpr int SPAWN_COUNT = 200; // Number of rectangles to spawn4

    new_rectangles.reserve(SPAWN_COUNT);
    rectangle_count += SPAWN_COUNT;

    for (int i = 0; i < SPAWN_COUNT; ++i)
    {
        // Generate random color
        Color<u8> color(rand() % 256, rand() % 256, rand() % 256, 255);

        // Position rectangle center at click point (all start from the same
        // point)
        float initial_world_x = world_x - RECT_WIDTH / 2.0f;
        float initial_world_y = world_y - RECT_HEIGHT / 2.0f;

        // Create rectangle at click position
        auto rect = std::make_unique<obj::Rectangle>(
            initial_world_x, initial_world_y, RECT_WIDTH, RECT_HEIGHT, color);

        // Configure rectangle properties
        rect->should_rotate = true;
        rect->move = true;
        rect->spawn_time = static_cast<float>(glfwGetTime());
        rect->randPhase =
            random_angle(random_engine); // Random phase for flutter effect

        // Set random initial rotation angles for GPU
        rect->initial_pitch = TWO_PI * (static_cast<float>(rand()) / RAND_MAX);
        rect->initial_yaw = TWO_PI * (static_cast<float>(rand()) / RAND_MAX);
        rect->initial_roll = TWO_PI * (static_cast<float>(rand()) / RAND_MAX);

        // === PHYSICS SETUP ===
        // Set physics properties
        rect->mass = DEFAULT_MASS;
        rect->calcAirCalc();

        // Apply initial explosive impulse in random direction
        float explosion_angle =
            random_angle(random_engine); // Random angle from 0 to 2π
        float dir_x = std::cos(explosion_angle);
        float dir_y = std::sin(explosion_angle);
        obj::Vec2 impulse(dir_x, dir_y);
        impulse.normalize();
        float str = EXPLOSION_STRENGTH + random_impuls_increase(random_engine);
        // std::cout<<"str: "<<str<<"\n";
        impulse *= str;           // Scale to m/s
        rect->velocity = impulse; // Scale by mass for consistent force

        // NOTE: Legacy compatibility calls removed to avoid overriding physics
        // The physics system now handles all movement

        new_rectangles.push_back(std::move(rect));
    }

    // Add new rectangles to global storage and render order (layer 1)
    for (auto &rect : new_rectangles)
    {
        obj::Rectangle *rect_ptr = rect.get();
        rectangles.push_back(std::move(rect));
        activeRects.push_back(rect_ptr); // Add to active rectangles
        render_order[layer_rectangles].push_back(rect_ptr);
    }
}

// ########## MOUSE INPUT HANDLING ##########

void update_mouse_hold_duration(double delta_time)
{
    if (left_mouse_held || right_mouse_held || middle_mouse_held)
    {
        mouse_hold_duration += delta_time;
    }
    else
    {
        mouse_hold_duration = 0.0;
    }
}

void handle_mouse_hold_continuous()
{
    constexpr double HOLD_THRESHOLD =
        0.5; // Start continuous spawn after 0.5 seconds
    constexpr double SPAWN_INTERVAL =
        0.1; // Spawn every 0.1 seconds during hold

    static double last_spawn_time = 0.0;

    if (left_mouse_held && mouse_hold_duration > HOLD_THRESHOLD)
    {
        // Spawn rectangles continuously at intervals
        if (mouse_hold_duration - last_spawn_time > SPAWN_INTERVAL)
        {
            spawn_rectangles(mouse_current_x, mouse_current_y);

            last_spawn_time = mouse_hold_duration;
        }
    }

    // Reset spawn timer when mouse is released
    if (!left_mouse_held)
    {
        last_spawn_time = 0.0;
    }
}

// ########## WORLD COORDINATE SYSTEM ##########

void init_world_background()
{
    Color<u8> black_color(BG_COLOR_R, BG_COLOR_G, BG_COLOR_B,
                          255); // background color

    // Create rectangle at world origin covering entire world
    background = std::make_unique<obj::Rectangle>(0.0f, 0.0f, world_width,
                                                  world_height, black_color);

    // Configure background properties
    background->should_rotate = false;
    background->move = false;
    background->should_render = true;

    // Add to background layer (use the pointer)
    render_order[layer_background].push_back(background.get());
}

void update_world_transform(float screen_w, float screen_h)
{
    // Update screen dimensions
    screen_width = screen_w;
    screen_height = screen_h;

    // Since the viewport is calculated to exactly match the world's aspect
    // ratio, we can use uniform scaling and the world should fill the entire
    // viewport
    float scale_x = screen_w / world_width;
    float scale_y = screen_h / world_height;

    // These should be equal due to aspect ratio preservation in viewport
    // calculation Use the minimum to ensure no overflow (though they should be
    // equal)
    world_scale = std::min(scale_x, scale_y);

    // World should be centered in viewport
    float scaled_world_width = world_width * world_scale;
    float scaled_world_height = world_height * world_scale;
    world_offset_x = (screen_w - scaled_world_width) * 0.5f;
    world_offset_y = (screen_h - scaled_world_height) * 0.5f;
}
