
#include <iostream>

#include "utils/globals.h"
#include "utils/key_captures.h"


// Key callback for GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Suppress unused parameter warnings
    (void)scancode;
    (void)mods;
    
    if (action == GLFW_PRESS){
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            enable_vsync = !enable_vsync;
            glfwSwapInterval(enable_vsync);
            break;
        
        }
    }
   
}