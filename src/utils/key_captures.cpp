
#include <iostream>

#include "entities/objects.h"    // Include full definition for Rectangle
#include "rendering/rasterize.h" // For update_viewport_cache
#include "utils/globals.h"
#include "utils/key_captures.h"
#include <algorithm>

// Key callback for GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods)
{
    // Suppress unused parameter warnings
    (void)scancode;
    (void)mods;

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            enable_vsync = !enable_vsync;
            glfwSwapInterval(enable_vsync);
            break;
        case GLFW_KEY_R:
            activeRects.clear();
            settledRects.clear();
            rectangle_count = 0;
            rectangles.clear();
            render_order[0].clear();
            // Re-add background rectangle
            break;
        case GLFW_KEY_G:
            apply_gravity = !apply_gravity;
            break;
        }
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    (void)window; // Suppress unused parameter warning
    (void)mods;   // Suppress unused parameter warning

    if (action == GLFW_PRESS)
    {
        mouse_hold_duration = 0.0;

        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            left_mouse_held = true;
            std::cout << "Left mouse button PRESSED at (" << mouse_current_x
                      << ", " << mouse_current_y << ")" << std::endl;

            // Immediate click behavior - use already tracked mouse position
            spawn_rectangles(mouse_current_x, mouse_current_y);
            break;
        }

        case GLFW_MOUSE_BUTTON_RIGHT:
            right_mouse_held = true;
            std::cout << "Right mouse button PRESSED at (" << mouse_current_x
                      << ", " << mouse_current_y << ")" << std::endl;
            // Add your right click press logic here
            break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            middle_mouse_held = true;
            std::cout << "Middle mouse button PRESSED at (" << mouse_current_x
                      << ", " << mouse_current_y << ")" << std::endl;
            // Add your middle click press logic here
            break;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            left_mouse_held = false;
            std::cout << "Left mouse button RELEASED at (" << mouse_current_x
                      << ", " << mouse_current_y << ")";

            if (mouse_hold_duration > 0.1)
            {
                std::cout << " - was HELD for " << mouse_hold_duration
                          << " seconds";
            }
            std::cout << std::endl;
            break;
        }

        case GLFW_MOUSE_BUTTON_RIGHT:
            right_mouse_held = false;
            std::cout << "Right mouse button RELEASED at (" << mouse_current_x
                      << ", " << mouse_current_y << ")" << std::endl;
            // Add your right click release logic here
            break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            middle_mouse_held = false;
            std::cout << "Middle mouse button RELEASED at (" << mouse_current_x
                      << ", " << mouse_current_y << ")" << std::endl;
            // Add your middle click release logic here
            break;
        }

        // Reset hold duration when any button is released
        mouse_hold_duration = 0.0;
    }
}

// Mouse position callback to track cursor movement during holds
void mouse_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    (void)window; // Suppress unused parameter warning

    // Update current mouse position
    mouse_current_x = static_cast<float>(xpos);
    mouse_current_y = static_cast<float>(ypos);

    mouse_world_x_prev = mouse_world_x;
    mouse_world_y_prev = mouse_world_y;

    // Convert to world coordinates
    mouse_world_x = screen_to_world_x(mouse_current_x);
    mouse_world_y = screen_to_world_y(mouse_current_y);

    mouse_last_t = mouse_current_t;
    mouse_current_t = glfwGetTime();

    // Optional: Add drag behavior here if needed
    // if (is_mouse_dragging()) {
    //     // Handle drag behavior
    // }
}

// Framebuffer size callback to update viewport cache
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window; // Suppress unused parameter warning
    glViewport(0, 0, width, height);
    update_viewport_cache(width, height);

    // Update world coordinate transform for resolution independence
    update_world_transform(static_cast<float>(width),
                           static_cast<float>(height));
}