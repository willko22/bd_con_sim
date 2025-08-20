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

// #include "rendering/rasterize.h"


// Error callback for GLFW
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}



int main() {
    std::cout << "Initializing GLFW and OpenGL..." << std::endl;
    
    //precompute sine and cosine values for angles
    precompute_trig_angles();

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
    double last_frame_time = last_time; // Track time for frame delta
    int frame_count = 0;
    float fps = 0.0f;
    std::vector<size_t> to_remove;


    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Calculate FPS and frame delta
        double current_time = glfwGetTime();
        double frame_delta = current_time - last_frame_time; // Time since last frame
        last_frame_time = current_time;
        
        // Update mouse hold duration and handle continuous hold behavior
        update_mouse_hold_duration(frame_delta);
        handle_mouse_hold_continuous();
        
        // Remove rectangles that are completely outside world bounds (performance optimization)
        // static double last_bounds_check = 0.0;
        // if (current_time - last_bounds_check > 0.5) { // Check bounds every 0.5 seconds
        //     remove_out_of_bounds_rectangles();
        //     last_bounds_check = current_time;
        // }
        
        frame_count++;
        double fps_delta = current_time - last_time; // Time since last FPS update
        if (fps_delta >= 1.0) {
            fps = frame_count / fps_delta;
            frame_count = 0;
            last_time = current_time;
        }

        // No longer need to update angles on CPU - GPU handles this now!
        // The GPU calculates current angles as: initial_angle + (rotation_speed * time)
        // This eliminates the bottleneck of updating thousands of rectangles on CPU
        
        float age = 0.0f;
        objects::BCircle bbox = {};
        for (size_t i = 0; i < activeRects.size(); ++i) {
            auto* rect = activeRects[i];
            if (!rect->move) continue; // Skip if rectangle is not moving
            age = current_time - rect->spawn_time;

            rect->adjustDirection(sin(age * FLUTTER_SPEED + rect->randPhase) 
                                * FLUTTER_STRENGHT * frame_delta, GRAVITY * frame_delta); // Apply gravity effect
            rect->setSpeed(RECTANGLE_SPEED + (SPAWN_SPEED - RECTANGLE_SPEED) * exp(rect->decay_rate * -age));

            rect->movePolygon(frame_delta);

            bbox = rect->bbox; // returns min/max x/y (implement if not existing)
            if (bbox.center.y + bbox.radius > world_height) 
            {
                // Option 1: stop movement
                rect->setSpeed(0.0f);

                // Option 2: clamp position to boundary
                rect->bbox.center.y = std::clamp(bbox.center.y, bbox.radius, world_height - bbox.radius);

                // Option 3: mark as "dead" for removal
                rect->stop_time = current_time; // Set stop time for GPU-side calculations
                rect->move = false;
                
                to_remove.push_back(i);
                // rect->should_rotate = false; // Disable rotation to stop GPU updates
                
            }
        }

        for (int j = static_cast<int>(to_remove.size()) - 1; j >= 0; --j) {
            size_t idx = to_remove[j];
            auto* rect = activeRects[idx];

            settledRects.push_back(rect);
            activeRects.erase(activeRects.begin() + idx);
        }
        to_remove.clear();
        
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

