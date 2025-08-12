#include "rendering/Render.hpp"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>

RenderSystem g_render;

// Vertex and fragment shader source code
const char* RenderSystem::vs_source = R"(
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
out vec4 frag_color;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    frag_color = color;
}
)";

const char* RenderSystem::fs_source = R"(
#version 330 core
in vec4 frag_color;
out vec4 FragColor;
void main() {
    FragColor = frag_color;
}
)";

void RenderSystem::init() {
    std::cout << "Setting up Sokol..." << std::endl;
    sg_setup(sg_desc{});
    std::cout << "Sokol setup complete!" << std::endl;
    
    std::cout << "Creating vertex buffer..." << std::endl;
    // Create dynamic vertex buffer with explicit initialization
    sg_buffer_desc buf_desc = {};
    buf_desc.size = 65536; // 64KB for dynamic vertex data
    buf_desc.usage.vertex_buffer = true;
    buf_desc.usage.dynamic_update = true;
    vertex_buffer = sg_make_buffer(buf_desc).id;
    std::cout << "Vertex buffer created: " << vertex_buffer << std::endl;
    
    std::cout << "Creating shader..." << std::endl;
    // Create shader
    sg_shader_desc shd_desc = {};
    shd_desc.vertex_func.source = vs_source;
    shd_desc.fragment_func.source = fs_source;
    shader = sg_make_shader(shd_desc).id;
    std::cout << "Shader created: " << shader << std::endl;
    
    std::cout << "Creating pipeline..." << std::endl;
    // Create pipeline
    sg_pipeline_desc pip_desc = {};
    pip_desc.shader.id = shader;
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2; // position
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4; // color
    pipeline = sg_make_pipeline(pip_desc).id;
    std::cout << "Pipeline created: " << pipeline << std::endl;
    std::cout << "RenderSystem initialization complete!" << std::endl;
}

void RenderSystem::frame() {
    static int frame_count = 0;
    frame_count++;
    if (frame_count % 60 == 0) {  // Print every 60 frames (roughly 1 second at 60fps)
        std::cout << "Frame " << frame_count << ", rectangles in layers: ";
        for (auto& [layer, rectangles] : layers) {
            std::cout << "Layer " << layer << ":" << rectangles.size() << " ";
        }
        std::cout << std::endl;
    }
    
    try {
        sg_pass_action pass_action = {};
        pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
        pass_action.colors[0].clear_value = { 0.0f, 0.2f, 0.3f, 1.0f };
        
        sg_pass pass = {};
        pass.action = pass_action;
        pass.swapchain.width = sapp_width();
        pass.swapchain.height = sapp_height();
        pass.swapchain.sample_count = 1;
        pass.swapchain.color_format = SG_PIXELFORMAT_RGBA8;
        pass.swapchain.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        
        sg_begin_pass(&pass);
        
        // Render all layers in order (if we have a valid vertex buffer)
        if (vertex_buffer != 0) {
            for (auto& [layer, rectangles] : layers) {
                if (!rectangles.empty()) {
                    renderLayer(layer);
                }
            }
        }
        
        sg_end_pass();
        sg_commit();
    } catch (...) {
        std::cout << "Error in frame rendering!" << std::endl;
    }
}

void RenderSystem::cleanup() {
    sg_destroy_pipeline({pipeline});
    sg_destroy_shader({shader});
    sg_destroy_buffer({vertex_buffer});
    sg_shutdown();
}

void RenderSystem::addRectangle(int layer, const RRectangle& rect) {
    layers[layer].push_back(rect);
}

void RenderSystem::clearLayer(int layer) {
    layers[layer].clear();
}

void RenderSystem::clearAll() {
    layers.clear();
}

void RenderSystem::generateRectangleVertices(const RRectangle& rect, std::vector<float>& vertices) {
    // Calculate RRectangle corners relative to center
    float half_w = rect.width * 0.5f;
    float half_h = rect.height * 0.5f;
    
    // Corner positions (before rotation)
    float corners[4][2] = {
        {-half_w, -half_h}, // bottom-left
        { half_w, -half_h}, // bottom-right
        { half_w,  half_h}, // top-right
        {-half_w,  half_h}  // top-left
    };
    
    // Apply rotation and translation
    float cos_r = std::cos(rect.rotation);
    float sin_r = std::sin(rect.rotation);
    
    float rotated_corners[4][2];
    for (int i = 0; i < 4; i++) {
        rotated_corners[i][0] = corners[i][0] * cos_r - corners[i][1] * sin_r + rect.x;
        rotated_corners[i][1] = corners[i][0] * sin_r + corners[i][1] * cos_r + rect.y;
    }
    
    // Create two triangles (6 vertices) for the RRectangle
    // Triangle 1: bottom-left, bottom-right, top-right
    // Triangle 2: bottom-left, top-right, top-left
    
    int triangle_indices[6] = {0, 1, 2, 0, 2, 3};
    
    for (int i = 0; i < 6; i++) {
        int corner_idx = triangle_indices[i];
        // Position (x, y)
        vertices.push_back(rotated_corners[corner_idx][0]);
        vertices.push_back(rotated_corners[corner_idx][1]);
        // Color (r, g, b, a)
        vertices.push_back(rect.r);
        vertices.push_back(rect.g);
        vertices.push_back(rect.b);
        vertices.push_back(rect.a);
    }
}

void RenderSystem::renderLayer(int layer) {
    if (layers[layer].empty()) return;
    
    // Generate vertices for all rectangles in this layer
    vertex_data.clear();
    for (const auto& rect : layers[layer]) {
        generateRectangleVertices(rect, vertex_data);
    }
    
    if (vertex_data.empty()) return;
    
    static int render_count = 0;
    render_count++;
    if (render_count % 60 == 0) {
        std::cout << "Rendering layer " << layer << " with " << layers[layer].size() 
                  << " rectangles, " << vertex_data.size() << " vertex floats" << std::endl;
    }
    
    // Update vertex buffer
    sg_range data_range;
    data_range.ptr = vertex_data.data();
    data_range.size = vertex_data.size() * sizeof(float);
    sg_update_buffer(sg_buffer{vertex_buffer}, data_range);
    
    // Draw rectangles
    sg_apply_pipeline(sg_pipeline{pipeline});
    sg_bindings bindings = {};
    bindings.vertex_buffers[0] = sg_buffer{vertex_buffer};
    sg_apply_bindings(&bindings);
    
    int vertex_count = vertex_data.size() / 6; // 6 floats per vertex (2 pos + 4 color)
    sg_draw(0, vertex_count, 1);
}
