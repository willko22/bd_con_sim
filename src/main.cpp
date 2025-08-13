#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// STB TrueType for font rendering
#include "stb_truetype.h"
#include "rendering/window.h"
#include "utils/globals.h"


// Error callback for GLFW
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Global variable definitions with default values


int main() {
    std::cout << "Initializing GLFW and OpenGL..." << std::endl;
    
    // Set error callback
    glfwSetErrorCallback(error_callback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
   
    
    // Disable v-sync to see maximum FPS (set to 1 to enable v-sync for 60fps cap)


    bool error;
    GLFWwindow* window;

    std::tie(error, window) = window_init();

    if (!error || !window) {
        std::cerr << "Failed to initialize window" << std::endl;
        glfwTerminate();
        return -1;
    }

    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    const char* glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    std::cout << "ImGui initialized successfully" << std::endl;
    
    std::cout << "=== GPU Information ===" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;
    
    // Check if we're using a dedicated GPU
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    if (renderer) {
        std::string gpu_name = std::string(renderer);
        if (gpu_name.find("NVIDIA") != std::string::npos || 
            gpu_name.find("GeForce") != std::string::npos ||
            gpu_name.find("RTX") != std::string::npos ||
            gpu_name.find("GTX") != std::string::npos) {
            std::cout << "✓ Using NVIDIA dedicated GPU" << std::endl;
        } else if (gpu_name.find("AMD") != std::string::npos || 
                   gpu_name.find("Radeon") != std::string::npos) {
            std::cout << "✓ Using AMD dedicated GPU" << std::endl;
        } else if (gpu_name.find("Intel") != std::string::npos && 
                   gpu_name.find("Arc") != std::string::npos) {
            std::cout << "✓ Using Intel Arc dedicated GPU" << std::endl;
        } else if (gpu_name.find("Intel") != std::string::npos) {
            std::cout << "⚠ Using Intel integrated GPU" << std::endl;
        } else {
            std::cout << "? Unknown GPU type" << std::endl;
        }
    }

    
    std::cout << "========================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC   - Close the window" << std::endl;
    std::cout << "  V     - Toggle VsyncW" << std::endl;
    std::cout << "Note: GPU switching requires application restart!" << std::endl;
    
    // Simple FPS tracking
    double last_time = glfwGetTime();
    int frame_count = 0;
    float fps = 0.0f;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Calculate FPS
        double current_time = glfwGetTime();
        frame_count++;
        if (current_time - last_time >= 1.0) {
            fps = frame_count / (current_time - last_time);
            frame_count = 0;
            last_time = current_time;
        }
        
        // Render the frame
        render_frame(fps);
        
        // Swap front and back buffers
        glfwSwapBuffers(window);
        
    }
    
    std::cout << "Shutting down..." << std::endl;
    
    // Custom cleanup
    window_cleanup();
    
    // ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // GLFW cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}