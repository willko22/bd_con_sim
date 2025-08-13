
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW + OpenGL Game", nullptr, nullptr);
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

    // Initialize the rasterizer after OpenGL context is created
    if (!rasterize_init()) {
        std::cerr << "Failed to initialize rasterizer" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return {false, nullptr};
    }

    return {true, window};
}

void render_frame(float fps) {
    // Clear the screen with a simple color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw our test triangle with different colors at each corner
    draw_test_triangle();
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Simple FPS window
    ImGui::Begin("FPS");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / fps);
    ImGui::Text("VSync: %s", enable_vsync ? "ON" : "OFF");
    ImGui::End();
    
    // Rendering
    ImGui::Render();
    
    // Render ImGui on top
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void window_cleanup() {
    // Clean up rasterizer resources
    rasterize_cleanup();
}