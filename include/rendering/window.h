#pragma once
#include <GLFW/glfw3.h>

#ifdef _WIN32
// Force dedicated GPU usage on Windows systems with hybrid graphics
extern "C" {
    // NVIDIA Optimus
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    
    // AMD PowerXpress
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif


std::pair<bool, GLFWwindow*> window_init();

// Render function that handles the entire frame rendering
void render_frame(float fps);

// Cleanup function to properly dispose of rendering resources
void window_cleanup();