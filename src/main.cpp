
#include "utils/globals.h"

#include <iostream>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "rendering/window.h"

// Error callback for GLFW
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

    bool error;
    GLFWwindow *window;

    std::tie(error, window) = window_init();

    if (!error || !window)
    {
        std::cerr << "Failed to initialize window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // GLAD is initialized in window_init() after creating the OpenGL context

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

    // Load fonts before initializing backends
    g_DefaultFont = io.Fonts->AddFontDefault();
    g_TitleFont =
        io.Fonts->AddFontFromFileTTF(TITLE_FONT_PATH, TITLE_FONT_SIZE);

    // Check if custom font loaded successfully, fallback to default if not
    if (!g_TitleFont)
    {
        std::cerr << "Warning: Failed to load custom font from "
                  << TITLE_FONT_PATH << ", using default font" << std::endl;
        g_TitleFont = g_DefaultFont;
    }

    // Setup
    // Platform/Renderer
    // backends
    const char *glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::cout << "ImGui initialized successfully" << std::endl;

    // Initialize title layout after fonts are loaded
    update_title_layout();

    // Initialize world background after OpenGL context is ready
    init_world_background();

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

    // Simple
    // FPS
    // tracking
    double last_time = glfwGetTime();
    double last_frame_time = last_time; // Track time for frame delta
    int frame_count = 0;
    float fps = 0.0f;
    std::vector<obj::Rectangle *> to_remove; // move to settled
    std::vector<obj::Rectangle *> to_add;    // move to active
    std::vector<obj::Rectangle *>
        to_remove_active_only; // drop from active only
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
        float cx, cy, seg_vx, seg_vy, seg_len2;
        float dt_mouse = mouse_current_t - mouse_last_t;
        float vx_mouse = (mouse_world_x - mouse_world_x_prev) / dt_mouse;
        float vy_mouse = (mouse_world_y - mouse_world_y_prev) / dt_mouse;
        float speed_mouse =
            std::sqrt(vx_mouse * vx_mouse + vy_mouse * vy_mouse);
        const float OUT_OFFSET = 1.0f;
        const float OFFSET_TIME =
            1.0f; // Time in seconds to smoothly push rectangle out
        const float EPS = 1e-6f;

        for (size_t i = 0; i < settledRects.size(); ++i)
        {
            rect = settledRects[i];
            if (rect->move)
                continue; // Skip if rectangle is already moving

            closest_point_on_segment(mouse_world_x_prev, mouse_world_y_prev,
                                     mouse_world_x, mouse_world_y,
                                     rect->bbox.center.x, rect->bbox.center.y,
                                     cx, cy, seg_vx, seg_vy, seg_len2);

            float dx = rect->bbox.center.x - cx;
            float dy = rect->bbox.center.y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float radius = MOUSE_RADIUS + rect->bbox.radius;

            // only correct if inside the swept circle
            if (dist < radius && rect->spawn_time + 1.f < current_time)
            {
                float nx, ny;
                if (dist > EPS)
                {
                    // normal from closest point to particle
                    nx = dx / dist;
                    ny = dy / dist;

                    // Calculate required velocity to smoothly push rectangle
                    // out of mouse radius, scaled by mouse speed for fast
                    // movements
                    float target_distance = radius + OUT_OFFSET;
                    float current_distance = dist;
                    float distance_to_travel =
                        target_distance - current_distance;

                    // Base velocity needed to reach target in OFFSET_TIME
                    float base_velocity = distance_to_travel / OFFSET_TIME;

                    // Scale the pushing force based on mouse speed to handle
                    // fast movements, but cap it to prevent skyrocketing
                    float mouse_speed_factor =
                        speed_mouse * 0.05f; // Reduced sensitivity
                    float mouse_speed_multiplier =
                        1.0f + std::min(mouse_speed_factor,
                                        RECT_SIM_WIDTH); // Cap at 3x max
                    float required_velocity =
                        base_velocity * mouse_speed_multiplier;

                    // Apply the velocity in the normal direction (away from
                    // mouse)
                    rect->velocity.x += nx * required_velocity;
                    rect->velocity.y += ny * required_velocity;
                }

                // Only apply push force to rectangles that are "in front" of
                // mouse movement
                if (speed_mouse > EPS) // Only if mouse is actually moving
                {
                    float penetration = (radius - dist) / radius; // 0..1

                    // Normalize mouse velocity vector
                    float mvx = vx_mouse / speed_mouse;
                    float mvy = vy_mouse / speed_mouse;

                    // Vector from mouse position to rectangle center
                    float to_rect_x = rect->bbox.center.x - mouse_world_x;
                    float to_rect_y = rect->bbox.center.y - mouse_world_y;
                    float to_rect_len = std::sqrt(to_rect_x * to_rect_x +
                                                  to_rect_y * to_rect_y);

                    if (to_rect_len > EPS)
                    {
                        // Normalize vector to rectangle
                        to_rect_x /= to_rect_len;
                        to_rect_y /= to_rect_len;

                        // Calculate dot product: positive means rectangle is
                        // "in front" of mouse movement
                        float dot_product = mvx * to_rect_x + mvy * to_rect_y;

                        // Only apply force if rectangle is in front
                        // (dot_product > 0) Use the dot product as a multiplier
                        // to scale force based on alignment
                        if (dot_product > 0.0f)
                        {
                            float force_magnitude = speed_mouse * dt_mouse *
                                                    penetration * MOUSE_MASS *
                                                    dot_product;

                            rect->velocity.x += mvx * force_magnitude;
                            rect->velocity.y += mvy * force_magnitude;
                            float randFactor =
                                0.01f + (rand() / (float)RAND_MAX) * 0.5f;

                            float multi = speed_mouse * dt_mouse * MOUSE_MASS;

                            rect->velocity.y -= multi * randFactor;
                            rect->velocity.x += mvx * multi * 0.5f;
                        }
                    }
                }

                // move rectangle back to active list
                rect->move = true;
                rect->stop_time = 0.0f;
                rect->spawn_time = current_time;
                to_add.push_back(rect);
            }
        }

        // Move selected rectangles from settled -> active by pointer
        if (!to_add.empty())
        {
            for (auto *r : to_add)
            {
                activeRects.push_back(r);
            }
            // Erase by pointer to avoid index invalidation issues
            for (auto *r : to_add)
            {
                auto it =
                    std::remove(settledRects.begin(), settledRects.end(), r);
                if (it != settledRects.end())
                    settledRects.erase(it, settledRects.end());
            }
            to_add.clear();
        }

        for (size_t i = 0; i < activeRects.size(); ++i)
        {
            rect = activeRects[i];
            if (!rect->move)
                continue; // Skip if rectangle is not moving

            // rect->k = 0.5 * rho * Cd * A / mass
            float damping = std::exp(-rect->k * dt);

            rect->velocity *= damping;

            // Gravity in m/s²
            if (apply_gravity)
                rect->velocity.y +=
                    GRAVITY_ACCELERATION * (RECT_WIDTH + 1) * dt;

            closest_point_on_segment(mouse_world_x_prev, mouse_world_y_prev,
                                     mouse_world_x, mouse_world_y,
                                     rect->bbox.center.x, rect->bbox.center.y,
                                     cx, cy, seg_vx, seg_vy, seg_len2);

            float dx = rect->bbox.center.x - cx;
            float dy = rect->bbox.center.y - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            float radius = MOUSE_RADIUS + rect->bbox.radius;

            // only correct if inside the swept circle
            if (dist < radius && rect->spawn_time + 1.f < current_time)
            {
                float nx, ny;
                if (dist > EPS)
                {
                    // normal from closest point to particle
                    nx = dx / dist;
                    ny = dy / dist;

                    // Calculate required velocity to smoothly push rectangle
                    // out of mouse radius, scaled by mouse speed for fast
                    // movements
                    float target_distance = radius + OUT_OFFSET;
                    float current_distance = dist;
                    float distance_to_travel =
                        target_distance - current_distance;

                    // Base velocity needed to reach target in OFFSET_TIME
                    float base_velocity = distance_to_travel / OFFSET_TIME;

                    // Scale the pushing force based on mouse speed to handle
                    // fast movements, but cap it to prevent skyrocketing
                    float mouse_speed_factor =
                        speed_mouse * 0.05f; // Reduced sensitivity
                    float mouse_speed_multiplier =
                        1.0f + std::min(mouse_speed_factor,
                                        RECT_SIM_WIDTH); // Cap at 3x max
                    float required_velocity =
                        base_velocity * mouse_speed_multiplier;

                    // Apply the velocity in the normal direction (away from
                    // mouse)
                    rect->velocity.x += nx * required_velocity;
                    rect->velocity.y += ny * required_velocity;
                }

                // Only apply push force to rectangles that are "in front" of
                // mouse movement
                if (speed_mouse > EPS) // Only if mouse is actually moving
                {
                    float penetration = (radius - dist) / radius; // 0..1

                    // Normalize mouse velocity vector
                    float mvx = vx_mouse / speed_mouse;
                    float mvy = vy_mouse / speed_mouse;

                    // Vector from mouse position to rectangle center
                    float to_rect_x = rect->bbox.center.x - mouse_world_x;
                    float to_rect_y = rect->bbox.center.y - mouse_world_y;
                    float to_rect_len = std::sqrt(to_rect_x * to_rect_x +
                                                  to_rect_y * to_rect_y);

                    if (to_rect_len > EPS)
                    {
                        // Normalize vector to rectangle
                        to_rect_x /= to_rect_len;
                        to_rect_y /= to_rect_len;

                        // Calculate dot product: positive means rectangle is
                        // "in front" of mouse movement
                        float dot_product = mvx * to_rect_x + mvy * to_rect_y;

                        // Only apply force if rectangle is in front
                        // (dot_product > 0) Use the dot product as a multiplier
                        // to scale force based on alignment
                        if (dot_product > 0.0f)
                        {
                            float force_magnitude = speed_mouse * dt_mouse *
                                                    penetration * MOUSE_MASS *
                                                    dot_product;

                            rect->velocity.x += mvx * force_magnitude;
                            rect->velocity.y += mvy * force_magnitude;
                        }
                    }
                }
            }

            rect->updatePhysics(dt);

            bbox = rect->bbox; // returns min/max x/y (implement if
                               // not existing)
            if (bbox.center.y + bbox.radius > world_height)
            {
                rect->setVelocity(0.0f, 0.0f);
                rect->bbox.center.y = std::clamp(bbox.center.y, bbox.radius,
                                                 world_height - bbox.radius);
                rect->position.y = rect->bbox.center.y;
                rect->stop_time = current_time;
                rect->move = false;
                to_remove.push_back(rect);
                // Avoid further processing on this rect in this iteration
                continue;
            }

            if (bbox.center.x + bbox.radius < 0 ||
                bbox.center.x - bbox.radius > world_width)
            {
                // Defer erase until after loop to keep indices stable
                to_remove_active_only.push_back(rect);
                continue;
            }
        }

        // Move selected rectangles from active -> settled by pointer
        if (!to_remove.empty())
        {
            for (auto *r : to_remove)
            {
                settledRects.push_back(r);
            }
            for (auto *r : to_remove)
            {
                auto it =
                    std::remove(activeRects.begin(), activeRects.end(), r);
                if (it != activeRects.end())
                    activeRects.erase(it, activeRects.end());
            }
            to_remove.clear();
        }

        // Remove any active-only indices (objects that left horizontal bounds)
        if (!to_remove_active_only.empty())
        {
            for (auto *r : to_remove_active_only)
            {
                auto it =
                    std::remove(activeRects.begin(), activeRects.end(), r);
                if (it != activeRects.end())
                    activeRects.erase(it, activeRects.end());
            }
            to_remove_active_only.clear();
        }

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
