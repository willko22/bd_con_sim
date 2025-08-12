#include "game/Game.hpp"
#include "rendering/Render.hpp"
#include <iostream>

Game::Game() 
    : rng(std::chrono::steady_clock::now().time_since_epoch().count())
    , pos_dist(-0.8f, 0.8f)
    , size_dist(0.05f, 0.3f)
    , color_dist(0.0f, 1.0f)
    , rotation_dist(0.0f, 6.28318f) // 0 to 2*PI
    , layer_dist(0, 5)
    , spawn_interval(1.0f) // spawn every 0.5 seconds
    , running(false)
{
    last_spawn_time = std::chrono::steady_clock::now();
}

Game::~Game() {
    std::cout << "Game destructor called" << std::endl;
    cleanup();
}

void Game::initialize() {
    std::cout << "Initializing game..." << std::endl;
    g_render.init();
    running = true;
    
    // Spawn an initial rectangle for testing
    std::cout << "Spawning initial test rectangle..." << std::endl;
    spawnRandomRectangle();
    std::cout << "Game initialization complete!" << std::endl;
}

void Game::update() {
    if (!running) return;
    
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<float>(current_time - last_spawn_time).count();
    
    if (elapsed >= spawn_interval) {
        spawnRandomRectangle();
        last_spawn_time = current_time;
    }
}

void Game::render() {
    if (!running) return;
    g_render.frame();
}

void Game::cleanup() {
    std::cout << "Cleaning up game..." << std::endl;
    g_render.cleanup();
    running = false;
}

void Game::drawRectangle(float x, float y, float width, float height, 
                        float rotation, float r, float g, float b, float a, int layer) {
    RRectangle rect(x, y, width, height, rotation, r, g, b, a);
    g_render.addRectangle(layer, rect);
}

void Game::spawnRandomRectangle() {
    float x = pos_dist(rng);
    float y = pos_dist(rng);
    float width = size_dist(rng);
    float height = size_dist(rng);
    float rotation = rotation_dist(rng);
    float r = color_dist(rng);
    float g = color_dist(rng);
    float b = color_dist(rng);
    float a = 0.8f; // slightly transparent
    int layer = 0;
    
    drawRectangle(x, y, width, height, rotation, r, g, b, a, layer);
    
    std::cout << "Spawned RRectangle at (" << x << ", " << y << ") with size (" 
              << width << "x" << height << ") on layer " << layer << std::endl;
}

// void Game::initialize() {
//     std::cout << "Initializing game..." << std::endl;

//     // Set up the window descriptor properly
//     _window = {};
//     _window.width = 640;
//     _window.height = 480;
//     _window.window_title = "BD Con Sim";
//     _window.init_cb = []() {
//         std::cout << "Sokol window initialized!" << std::endl;
//     };
//     _window.frame_cb = []() {
//         // This gets called every frame
//     };
//     _window.cleanup_cb = []() {
//         std::cout << "Sokol window cleanup" << std::endl;
//     };
//     _window.event_cb = [](const sapp_event* event) {
//         // Handle events
//         (void)event; // Suppress unused parameter warning
//     };

//     // Initialize SDL, SFML, or other libraries here
// }

// void Game::update() {
//     // Game logic updates
// }

// void Game::render() {

//     // Rendering code
// }

// void Game::cleanup() {
//     std::cout << "Cleaning up..." << std::endl;
//     // Cleanup resources
// }
