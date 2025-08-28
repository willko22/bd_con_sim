#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

// ImGui
// includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// STB
// TrueType
// for
// font
// rendering
// #include
// "stb_truetype.h"
#include "rendering/window.h"
#include "utils/globals.h"

// #include
// "rendering/rasterize.h"

// Error
// callback
// for
// GLFW
void error_callback(int error, const char *description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main()
{
    std::cout << "Initializing GLFW and OpenGL..." << std::endl;

    // precompute
    // sine
    // and
    // cosine
    // values
    // for
    // angles
    precompute_trig_angles();

    // Set
    // error
    // callback
    glfwSetErrorCallback(error_callback);

    // Initialize
    // GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Disable
    // v-sync
    // to
    // see
    // maximum
    // FPS
    // (set
    // to
    // 1
    // to
    // enable
    // v-sync
    // for
    // 60fps
    // cap)

    bool error;
    GLFWwindow *window;

    std::tie(error, window) = window_init();

    if (!error || !window)
    {
        std::cerr << "Failed to initialize window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initialize
    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard
                                                          // Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup
    // ImGui
    // style
    ImGui::StyleColorsDark();

    // Setup
    // Platform/Renderer
    // backends
    const char *glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::cout << "ImGui initialized successfully" << std::endl;

    std::cout << "=== GPU Information ===" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;

    // Check
    // if
    // we're
    // using
    // a
    // dedicated
    // GPU
    const char *renderer = (const char *)glGetString(GL_RENDERER);
    if (renderer)
    {
        std::string gpu_name = std::string(renderer);
        if (gpu_name.find("NVIDIA") != std::string::npos ||
            gpu_name.find("GeForce") != std::string::npos ||
            gpu_name.find("RTX") != std::string::npos ||
            gpu_name.find("GTX") != std::string::npos)
        {
            std::cout << "✓ Using NVIDIA dedicated GPU" << std::endl;
        }
        else if (gpu_name.find("AMD") != std::string::npos ||
                 gpu_name.find("Radeon") != std::string::npos)
        {
            std::cout << "✓ Using AMD dedicated GPU" << std::endl;
        }
        else if (gpu_name.find("Intel") != std::string::npos &&
                 gpu_name.find("Arc") != std::string::npos)
        {
            std::cout << "✓ Using Intel Arc dedicated GPU" << std::endl;
        }
        else if (gpu_name.find("Intel") != std::string::npos)
        {
            std::cout << "⚠ Using Intel integrated GPU" << std::endl;
        }
        else
        {
            std::cout << "? Unknown GPU type" << std::endl;
        }
    }

    std::cout << "========================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC   - Close the window" << std::endl;
    std::cout << "  V     - Toggle VsyncW" << std::endl;
    std::cout << "Note: GPU switching requires application restart!"
              << std::endl;

    // Simple
    // FPS
    // tracking
    double last_time = glfwGetTime();
    double last_frame_time = last_time; // Track time for frame delta
    int frame_count = 0;
    float fps = 0.0f;
    std::vector<size_t> to_remove;
    // float
    // age
    // =
    // 0.0f;

    // Main
    // loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Calculate FPS and frame delta
        double current_time = glfwGetTime();
        double dt = current_time - last_frame_time; // Time since last frame
        last_frame_time = current_time;

        // Update mouse hold duration and handle continuous hold
        // behavior
        update_mouse_hold_duration(dt);
        handle_mouse_hold_continuous();

        // Remove rectangles that are completely outside world bounds
        // (performance optimization) static double last_bounds_check
        // = 0.0; if (current_time - last_bounds_check > 0.5) { //
        // Check bounds every 0.5 seconds
        //     remove_out_of_bounds_rectangles();
        //     last_bounds_check = current_time;
        // }

        frame_count++;
        double fps_delta =
            current_time - last_time; // Time since last FPS update
        if (fps_delta >= 1.0)
        {
            fps = frame_count / fps_delta;
            frame_count = 0;
            last_time = current_time;
        }

        // === PHYSICS-BASED SIMULATION ===
        obj::BCircle bbox = {};
        obj::Rectangle *rect = nullptr;
        for (size_t i = 0; i < activeRects.size(); ++i)
        {
            rect = activeRects[i];
            if (!rect->move)
                continue; // Skip if rectangle is not moving

            // age = current_time - rect->spawn_time;

            // === Apply Physics Forces ===

            // // Apply gravity force: F = ma, so F = m * g
            // obj::Vec2 gravity_force(0.0f, GRAVITY_ACCELERATION);
            // // rect->applyForce(gravity_force);
            // rect->acceleration += gravity_force ; // Directly add
            // to acceleration

            // // Apply flutter effect as a horizontal force
            // // float flutter_force_x = sin(age * FLUTTER_SPEED +
            // rect->randPhase) * FLUTTER_STRENGTH * rect->mass;
            // // obj::Vec2 flutter_force(flutter_force_x, 0.0f);
            // // rect->applyForce(flutter_force);

            // float v2 = rect->velocity.magnitudeSquared();
            // if (v2 > 0.0f)
            // {
            //     obj::Vec2 drag_acc = rect->velocity.normalized() /
            //     rect->air_calc * v2; rect->acceleration -=
            //     drag_acc;
            //     // rect->applyForce(drag_acc);
            // }

            // obj::Vec2 gravityForce(0.0f, rect->mass *
            // GRAVITY_ACCELERATION); rect->applyForce(gravityForce);

            // float v_mag = rect->velocity.magnitude();
            // if (v_mag > 1e-6f) {
            //     float dragMag = DRAG_COEFF * v_mag * v_mag;
            //     obj::Vec2 dragForce = rect->velocity.normalized() *
            //     -dragMag; rect->applyForce(dragForce);
            // }

            // if (rect->speed > 0.001f) {  // Avoid division by zero
            //     float damping = std::exp(-rect->k * dt);
            //     obj::Vec2 dragAccel = rect->velocity * damping *
            //     rect->speed
            //     * METERS_TO_WORLD / rect->mass; float vdot =
            //     rect->velocity.dot(dragAccel); // should be
            //     negative float maxDecel = rect->speed / dt; // max
            //     allowed decel magnitude if (-vdot > maxDecel) {
            //         // clamp so velocity goes exactly to zero, not
            //         negative dragAccel = rect->velocity * (-1.0f /
            //         dt);
            //     }

            //     rect->acceleration += dragAccel;
            // }

            // rect->k = 0.5 * rho * Cd * A / mass
            float damping = std::exp(-rect->k * dt);

            rect->velocity *= damping;

            // Gravity in m/s²
            rect->velocity.y += GRAVITY_ACCELERATION * 4 * dt;

            float x_dist = rect->bbox.center.x - mouse_world_x;
            float y_dist = rect->bbox.center.y - mouse_world_y;

            // avoid div by zero with small offset
            float dist_squared = x_dist * x_dist + y_dist * y_dist;
            float dist = std::sqrt(dist_squared);
            if (dist < MOUSE_RADIUS + rect->bbox.radius && dist > 1.0f &&
                rect->spawn_time + 0.5f < current_time)
            {
                // Repulsion force inversely proportional to distance
                // squared
                float force_magnitude = EXPLOSION_STRENGTH;

                // Apply force away from mouse position
                rect->velocity.x += (x_dist / dist) * force_magnitude * dt;
                rect->velocity.y += (y_dist / dist) * force_magnitude * dt;
            }

            rect->updatePhysics(dt);

            bbox = rect->bbox; // returns min/max x/y (implement if
                               // not existing)
            if (bbox.center.y + bbox.radius > world_height)
            {
                // Option 1: stop movement
                rect->setVelocity(0.0f, 0.0f);

                // Option 2: clamp position to boundary
                rect->bbox.center.y = std::clamp(bbox.center.y, bbox.radius,
                                                 world_height - bbox.radius);
                rect->position.y =
                    rect->bbox.center.y; // Keep position synchronized

                // Option 3: mark as "dead" for removal
                rect->stop_time = current_time; // Set stop time for GPU-side
                                                // calculations
                rect->move = false;

                to_remove.push_back(i);
                // rect->should_rotate = false; // Disable rotation to
                // stop GPU updates
            }
        }

        for (int j = static_cast<int>(to_remove.size()) - 1; j >= 0; --j)
        {
            size_t idx = to_remove[j];
            auto *rect = activeRects[idx];

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

    // Custom
    // cleanup
    window_cleanup();

    // ImGui
    // cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // GLFW
    // cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
