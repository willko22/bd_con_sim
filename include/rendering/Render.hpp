#pragma once

#include <vector>
#include <map>

struct RRectangle {
    float x, y;           // position
    float width, height;  // size
    float rotation;       // rotation in radians
    float r, g, b, a;     // color (RGBA)
    
    RRectangle(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f, 
              float rotation = 0.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
        : x(x), y(y), width(width), height(height), rotation(rotation), r(r), g(g), b(b), a(a) {}
};

class RenderSystem {
public:
    void init();
    void frame();
    void cleanup();
    
    void addRectangle(int layer, const RRectangle& rect);
    void clearLayer(int layer);
    void clearAll();
    
private:
    void generateRectangleVertices(const RRectangle& rect, std::vector<float>& vertices);
    void renderLayer(int layer);
    
    // Sokol resources - forward declare to avoid including sokol headers here
    unsigned int vertex_buffer; // sg_buffer
    unsigned int shader;        // sg_shader  
    unsigned int pipeline;      // sg_pipeline
    
    // Render data
    std::map<int, std::vector<RRectangle>> layers;
    std::vector<float> vertex_data; // temp buffer for vertices
    
    // Shader sources
    static const char* vs_source;
    static const char* fs_source;
};

extern RenderSystem g_render;
