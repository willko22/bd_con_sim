
#include <iostream>
#include <string>

#include "utils/key_captures.h"

#include "rendering/rasterize.h"
#include "utils/globals.h"

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "rendering/window.h"

#ifdef _WIN32
// Force dedicated GPU usage on Windows systems with hybrid graphics
extern "C"
{
    // NVIDIA Optimus
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

    // AMD PowerXpress
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

std::pair<bool, GLFWwindow *> window_init()
{
    // Configure GLFW for high performance and NVIDIA overlay compatibility
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Request dedicated GPU context (if available)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA for better quality
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    // Additional hints for better overlay detection
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    // Create window
    GLFWwindow *window = glfwCreateWindow(
        screen_width, screen_height, "GLFW + OpenGL Game", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return {false, nullptr};
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLAD before any OpenGL calls
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return {false, nullptr};
    }

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

    // Initialize viewport cache with current window size using aspect ratio
    // preservation
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // Calculate aspect ratio preserving viewport
    const float target_aspect =
        world_width / world_height; // Target aspect ratio from world dimensions
    const float window_aspect =
        static_cast<float>(width) / static_cast<float>(height);

    if (window_aspect > target_aspect)
    {
        // Window is wider than target aspect ratio - pillarbox (black bars on
        // sides)
        viewport_height = height;
        viewport_width = static_cast<int>(height * target_aspect);
        viewport_x = (width - viewport_width) / 2;
        viewport_y = 0;
    }
    else
    {
        // Window is taller than target aspect ratio - letterbox (black bars on
        // top/bottom)
        viewport_width = width;
        viewport_height = static_cast<int>(width / target_aspect);
        viewport_x = 0;
        viewport_y = (height - viewport_height) / 2;
    }

    // Set viewport to maintain aspect ratio
    glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    update_viewport_cache(viewport_width, viewport_height);
    screen_width = static_cast<float>(viewport_width);
    screen_height = static_cast<float>(viewport_height);

    // Initialize world coordinate transform for resolution independence
    update_world_transform(static_cast<float>(viewport_width),
                           static_cast<float>(viewport_height));

    // Initialize the rasterizer after OpenGL context is created
    if (!rasterize_init())
    {
        std::cerr << "Failed to initialize rasterizer" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return {false, nullptr};
    }

    return {true, window};
}

void render_frame(float &fps)
{
    // Clear the screen with transparent/black (the world background rectangle
    // will handle the black)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // INSTANCED RENDERING OPTIMIZATION: Draw all rectangles with maximum
    // efficiency
    for (size_t i = 0; i < render_order.size(); ++i)
    {
        auto &layer = render_order[i];
        if (!layer.empty())
        {

            instanced_draw_rectangles(layer, i == layer_background);

            // Draw red center dots for debugging rotation centers
            // draw_center_dots(layer);
        }

        if (i == layer_text)
        {
            // Render title text with custom font
            if (g_TitleFont != nullptr)
            {
                ImGui::PushFont(g_TitleFont);

                // Calculate text size with the current font at original size
                ImVec2 textSize = ImGui::CalcTextSize(TITLE_TEXT);

                // Calculate dynamic scale factor to make title 60% of viewport
                // width
                float desired_width = viewport_width * 0.6f;
                float scale_factor = desired_width / textSize.x;

                // Update title dimensions in globals (scaled dimensions)
                title_width = static_cast<int>(textSize.x * scale_factor);
                title_height = static_cast<int>(textSize.y * scale_factor);

                // Create invisible window for text rendering
                ImGui::SetNextWindowPos(
                    ImVec2(title_position_x, title_position_y),
                    ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::SetNextWindowSize(ImVec2(0, 0)); // Auto-size
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                ImGui::PushStyleColor(
                    ImGuiCol_WindowBg,
                    ImVec4(0, 0, 0, 0)); // Transparent background
                ImGui::PushStyleColor(ImGuiCol_Border,
                                      ImVec4(0, 0, 0, 0)); // No border

                ImGui::Begin(
                    "TitleText", nullptr,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse |
                        ImGuiWindowFlags_NoInputs |
                        ImGuiWindowFlags_AlwaysAutoResize);

                // Apply scaling using ImGui's built-in scaling
                ImGui::SetWindowFontScale(scale_factor);

                // Convert color from u8 to float and render text
                Color<float> titleColorGL = TITLE_FONT_COLOR.toGL();
                ImGui::TextColored(ImVec4(titleColorGL.r, titleColorGL.g,
                                          titleColorGL.b, titleColorGL.a),
                                   TITLE_TEXT);

                ImGui::End();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);
                ImGui::PopFont();
            }
        }
    }

    // Conditional ImGui rendering for better performance (NEW OPTIMIZATION)
    static bool show_ui = true; // Toggle with 'U' key or similar

    if (show_ui)
    {

        // Start the Dear ImGui frame

        // Simple FPS window
        ImGui::Begin("FPS");
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / fps);
        ImGui::Text("VSync: %s", enable_vsync ? "ON" : "OFF");
        ImGui::Text("Rectangle Count: %lld", activeRects.size());
        ImGui::Separator();
        ImGui::Text("Title Position: (%d, %d)", title_position_x,
                    title_position_y);
        ImGui::Text("Title Size: %dx%d", title_width, title_height);

        ImGui::End();

        // Rendering
        ImGui::Render();

        // Render ImGui on top
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    // Note: Add 'U' key handler to toggle show_ui in key_captures.cpp for
    // runtime control
}

void window_cleanup()
{
    // Clean up rasterizer resources
    rasterize_cleanup();
}