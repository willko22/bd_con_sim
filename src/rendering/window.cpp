
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "utils/globals.h"
#include "utils/key_captures.h"
#include "rendering/rasterize.h"

#include "rendering/window.h"

#ifdef _WIN32
// Force dedicated GPU usage on Windows systems with hybrid graphics
extern "C" {
    // NVIDIA Optimus
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    
    // AMD PowerXpress
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif


std::pair<bool, GLFWwindow*> window_init(){
    // Configure GLFW for high performance and NVIDIA overlay compatibility
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Request dedicated GPU context (if available)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);  // 4x MSAA for better quality
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    // Additional hints for better overlay detection
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);


    // Create window
    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "GLFW + OpenGL Game", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return {false, nullptr};
    }
    
    // Make the window's context current
    glfwMakeContextCurrent(window);
    
    // Set V-sync based on global setting
    glfwSwapInterval(enable_vsync);
    
    // Set key callback
    glfwSetKeyCallback(window, key_callback);
    
    // Set mouse button callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    // Set mouse position callback for tracking movement during holds
    glfwSetCursorPosCallback(window, mouse_position_callback);
    
    // Set framebuffer size callback for viewport updates
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Initialize viewport cache with current window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    update_viewport_cache(width, height);
    screen_width = static_cast<float>(width);
    screen_height = static_cast<float>(height);
    
    // Initialize world coordinate transform for resolution independence
    update_world_transform(static_cast<float>(width), static_cast<float>(height));

    // Initialize the rasterizer after OpenGL context is created
    if (!rasterize_init()) {
        std::cerr << "Failed to initialize rasterizer" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return {false, nullptr};
    }

    return {true, window};
}

void render_frame(float& fps) {
    // Clear the screen with black background when world is centered
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // INSTANCED RENDERING OPTIMIZATION: Draw all rectangles with maximum efficiency
    for (auto& layer : render_order) {
        if (!layer.empty()) {
            // Draw entire layer with instanced rendering - maximum performance!
            instanced_draw_rectangles(layer);
            // Draw red center dots for debugging rotation centers
            // draw_center_dots(layer);
        }
    }

    // begin_batch_render();
    // for (const auto& layer : render_order) {
    //     if (!layer.empty()) {
    //         // Draw entire layer with batch rendering - good performance!
    //         batch_draw_rectangles(layer);
    //     }
    // }

    end_batch_render();
    
    // Conditional ImGui rendering for better performance (NEW OPTIMIZATION)
    static bool show_ui = true; // Toggle with 'U' key or similar
    
    if (show_ui) {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    
    // Simple FPS window
    ImGui::Begin("FPS");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / fps);
    ImGui::Text("VSync: %s", enable_vsync ? "ON" : "OFF");
    ImGui::Text("Rectangle Count: %d", rectangle_count);
    
    // // Mouse hold state information
    // ImGui::Separator();
    // ImGui::Text("Mouse State:");
    // ImGui::Text("Position: (%.1f, %.1f)", mouse_current_x, mouse_current_y);
    // ImGui::Text("Left: %s", left_mouse_held ? "HELD" : "Released");
    // ImGui::Text("Right: %s", right_mouse_held ? "HELD" : "Released");
    // ImGui::Text("Middle: %s", middle_mouse_held ? "HELD" : "Released");
    
    // if (left_mouse_held || right_mouse_held || middle_mouse_held) {
    //     ImGui::Text("Hold Duration: %.2f s", mouse_hold_duration);
    // }
    
    ImGui::End();
    
    // Rendering
    ImGui::Render();
    
    // Render ImGui on top
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    // Note: Add 'U' key handler to toggle show_ui in key_captures.cpp for runtime control
}

void window_cleanup() {
    // Clean up rasterizer resources
    rasterize_cleanup();
}