#pragma once

#include "rendering/Render.hpp"
#include <random>
#include <chrono>

class Game {
public:
    Game();
    ~Game();
    
    void initialize();
    void update();
    void render();
    void cleanup();
    
    void drawRectangle(float x, float y, float width, float height, 
                      float rotation = 0.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f, int layer = 0);
    
    void spawnRandomRectangle();
    
private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> pos_dist;
    std::uniform_real_distribution<float> size_dist;
    std::uniform_real_distribution<float> color_dist;
    std::uniform_real_distribution<float> rotation_dist;
    std::uniform_int_distribution<int> layer_dist;
    
    std::chrono::steady_clock::time_point last_spawn_time;
    float spawn_interval; // seconds between spawns
    
    bool running;
};
// };
