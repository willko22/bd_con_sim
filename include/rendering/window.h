#pragma once

#include <GLFW/glfw3.h>
#include "entities/objects.h"

#ifdef _WIN32
// Force dedicated GPU usage on Windows systems with hybrid graphics
extern "C" {
    // NVIDIA Optimus
    extern __declspec(dllexport) unsigned long NvOptimusEnablement;
    
    // AMD PowerXpress
    extern __declspec(dllexport) int AmdPowerXpressRequestHighPerformance;
}
#endif





std::pair<bool, GLFWwindow*> window_init();

// Render function that handles the entire frame rendering
void render_frame(float& fps);

// Cleanup function to properly dispose of rendering resources
void window_cleanup();