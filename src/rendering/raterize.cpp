#include "rendering/rasterize.h"
#include <iostream>
#include <string>
#include <cmath>
#include <GLFW/glfw3.h>

#include "rendering/vertex_shader.h"
#include "rendering/fragment_shader.h"
#include "utils/globals.h" // For trig_table access

// OpenGL function pointers for Windows
#ifdef _WIN32
    #include <windows.h>
    #include <GL/gl.h>
    #include <GL/glext.h>
    
    // Function pointers for modern OpenGL
    PFNGLCREATESHADERPROC glCreateShader = nullptr;
    PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
    PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
    PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
    PFNGLDELETESHADERPROC glDeleteShader = nullptr;
    PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
    PFNGLATTACHSHADERPROC glAttachShader = nullptr;
    PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
    PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
    PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
    PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
    PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
    PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
    PFNGLBUFFERDATAPROC glBufferData = nullptr;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
    PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor = nullptr;
    PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = nullptr;
    PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;
    PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
    PFNGLUNIFORM1IPROC glUniform1i = nullptr;
    PFNGLUNIFORM1FPROC glUniform1f = nullptr;
    PFNGLUNIFORM2FPROC glUniform2f = nullptr;  // NEW
    
    // Function to load OpenGL function pointers
    void loadOpenGLFunctions() {
        glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
        glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
        glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
        glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
        glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
        glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
        glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
        glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
        glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
        glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
        glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
        glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
        glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
        glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
        glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
        glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
        glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
        glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)wglGetProcAddress("glVertexAttribDivisor");
        glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)wglGetProcAddress("glDrawArraysInstanced");
        glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
        glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
        glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
        glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
        glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");  // NEW
    }
#endif


// Global OpenGL objects
static unsigned int shaderProgram = 0;      // Instanced rendering shader
static unsigned int batchShaderProgram = 0; // Batch rendering shader (fallback)
static unsigned int VAO = 0;
static unsigned int VBO = 0;

// Global variables for instanced rendering
static GLuint instanceVBO = 0;
static GLuint instanceVAO = 0;
static const int MAX_INSTANCES = 1000000; // Support up to 500k rectangles

// GPU trig table
static GLuint trigTableTexture = 0;
static float trigTableSize = 0.0f;

// Cached uniform locations for performance (NEW OPTIMIZATION)
static GLint uTrigTableLoc = -1;
static GLint uTrigTableSizeLoc = -1;
static GLint uTimeLoc = -1;
static GLint uRotationSpeedLoc = -1;
static GLint uScreenSizeLoc = -1;  // NEW: for GPU-side NDC conversion
static GLint uWorldScaleLoc = -1;  // NEW: for GPU-side world coordinate conversion
static GLint uWorldOffsetLoc = -1; // NEW: for GPU-side world coordinate conversion

// Cached window dimensions for performance
static int cached_width = 800;
static int cached_height = 600;

// Cached NDC conversion factors (NEW OPTIMIZATION)
static float cached_width_inv = 1.0f / 800.0f;
static float cached_height_inv = 1.0f / 600.0f;

// Reusable vertex buffer for performance
static std::vector<float> reusable_vertices;


// Helper function to compile shader
static unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    return shader;
}

// Function to upload trig table to GPU as a 1D texture
static bool uploadTrigTableToGPU() {
    if (trig_table.empty()) {
        std::cerr << "Error: Trig table is empty. Make sure to call precompute_trig_table() first." << std::endl;
        return false;
    }
    
    // Store the table size for shader
    trigTableSize = static_cast<float>(trig_table_size);
    
    // Prepare texture data (interleaved sin, cos values for RG format)
    std::vector<float> textureData;
    textureData.reserve(trig_table.size() * 2);
    for (const auto& entry : trig_table) {
        textureData.push_back(entry.first);  // sin value (R channel)
        textureData.push_back(entry.second); // cos value (G channel)
    }
    
    // Create and upload the texture
    glGenTextures(1, &trigTableTexture);
    glBindTexture(GL_TEXTURE_1D, trigTableTexture);
    
    // Upload data as RG32F texture (Red = sin, Green = cos)
    // The width should be trig_table.size(), not trig_table.size() * 2
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RG32F, trig_table.size(), 0, GL_RG, GL_FLOAT, textureData.data());
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // No interpolation for lookup table
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_1D, 0);
    
    std::cout << "Uploaded trig table to GPU with " << trig_table.size() << " entries" << std::endl;
    return true;
}

bool rasterize_init() {
#ifdef _WIN32
    // Load OpenGL function pointers
    loadOpenGLFunctions();
    
    // Check if functions were loaded successfully
    if (!glCreateShader || !glShaderSource || !glCompileShader) {
        std::cerr << "Failed to load OpenGL function pointers!" << std::endl;
        return false;
    }
#endif
    
    // Compile shaders for instanced rendering
    unsigned int instancedVertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    // // Compile shaders for batch rendering (fallback)
    // unsigned int batchVertexShader = compileShader(GL_VERTEX_SHADER, batchVertexShaderSource);
    
    // Create instanced shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, instancedVertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::INSTANCED_SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }
    
    // Create batch shader program
    // batchShaderProgram = glCreateProgram();
    // glAttachShader(batchShaderProgram, batchVertexShader);
    // glAttachShader(batchShaderProgram, fragmentShader);
    // glLinkProgram(batchShaderProgram);
    
    // glGetProgramiv(batchShaderProgram, GL_LINK_STATUS, &success);
    // if (!success) {
    //     glGetProgramInfoLog(batchShaderProgram, 512, nullptr, infoLog);
    //     std::cerr << "ERROR::BATCH_SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    //     return false;
    // }
    
    // Clean up individual shaders
    glDeleteShader(instancedVertexShader);
    // glDeleteShader(batchVertexShader);
    glDeleteShader(fragmentShader);
    
    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    // Upload trig table to GPU
    if (!uploadTrigTableToGPU()) {
        std::cerr << "Warning: Failed to upload trig table to GPU. Performance may be reduced." << std::endl;
    }
    
    // Cache uniform locations for performance optimization (NEW)
    uTrigTableLoc = glGetUniformLocation(shaderProgram, "uTrigTable");
    uTrigTableSizeLoc = glGetUniformLocation(shaderProgram, "uTrigTableSize"); 
    uTimeLoc = glGetUniformLocation(shaderProgram, "uTime");
    uRotationSpeedLoc = glGetUniformLocation(shaderProgram, "uRotationSpeed");
    uScreenSizeLoc = glGetUniformLocation(shaderProgram, "uScreenSize");  // NEW
    uWorldScaleLoc = glGetUniformLocation(shaderProgram, "uWorldScale");  // NEW
    uWorldOffsetLoc = glGetUniformLocation(shaderProgram, "uWorldOffset"); // NEW
    
    std::cout << "Rasterizer initialized successfully" << std::endl;
    return true;
}

void rasterize_cleanup() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    if (batchShaderProgram != 0) {
        glDeleteProgram(batchShaderProgram);
        batchShaderProgram = 0;
    }
    // Clean up instanced rendering resources
    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
        instanceVBO = 0;
    }
    if (instanceVAO != 0) {
        glDeleteVertexArrays(1, &instanceVAO);
        instanceVAO = 0;
    }
    if (trigTableTexture != 0) {
        glDeleteTextures(1, &trigTableTexture);
        trigTableTexture = 0;
    }
    std::cout << "Rasterizer cleaned up" << std::endl;
}

void update_viewport_cache(int width, int height) {
    cached_width = width;
    cached_height = height;
    
    // Cache inverse values to avoid divisions in rendering loop (NEW OPTIMIZATION)
    cached_width_inv = 1.0f / static_cast<float>(width);
    cached_height_inv = 1.0f / static_cast<float>(height);
}

void begin_batch_render() {
    if (shaderProgram == 0) {
        std::cerr << "Rasterizer not initialized! Call rasterize_init() first." << std::endl;
        return;
    }
    
    // Setup OpenGL state once for all polygons (MAJOR OPTIMIZATION!)
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glUseProgram(shaderProgram);
}

void end_batch_render() {
    // Unbind after all drawing
    glBindVertexArray(0);
}

int _buffer_size = 16; // Updated: position(2) + size(2) + color(4) + angles(3) + velocity(2) + spawn_time(1) + flags(2) = 16 floats per instance
void instanced_draw_rectangles(const std::vector<objects::Rectangle*>& rectangles) {
    if (rectangles.empty()) return;

    
    // Initialize instanced rendering resources if needed
    static bool instanced_initialized = false;
    if (!instanced_initialized) {
            
        // Create instance data buffer
        glGenBuffers(1, &instanceVBO);
        glGenVertexArrays(1, &instanceVAO);
        
        // Set up the base rectangle geometry (unit square)
        constexpr static float base_vertices[] = {
            // Triangle 1
            0.0f, 0.0f,  // Bottom-left
            1.0f, 0.0f,  // Bottom-right  
            0.0f, 1.0f,  // Top-left
            
            // Triangle 2
            1.0f, 0.0f,  // Bottom-right
            1.0f, 1.0f,  // Top-right
            0.0f, 1.0f   // Top-left
        };
        
        glBindVertexArray(instanceVAO);
        
        // Upload base geometry
        GLuint geometryVBO;
        glGenBuffers(1, &geometryVBO);
        glBindBuffer(GL_ARRAY_BUFFER, geometryVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(base_vertices), base_vertices, GL_STATIC_DRAW);
        
        // Set vertex attribute for position (location 0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Set up instance data buffer (empty for now, will be filled each frame)
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * _buffer_size * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        
        // Instance attributes: position(2) + size(2) + color(4) + angles(3) + velocity(2) + spawn_time(1) + flags(2) = 16 floats per instance
        // Position offset (location 1) - now world coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, _buffer_size * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribDivisor(1, 1); // Advance once per instance
        
        // Size (location 2) - now world coordinates
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, _buffer_size * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);
        
        // Color (location 3)
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, _buffer_size * sizeof(float), (void*)(4 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);
        
        // Rotation angles (location 4) - 3 components for pitch, yaw, roll
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, _buffer_size * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4, 1);
        
        // Velocity (location 5) - 2 components for velocity X and Y in world units per second
        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, _buffer_size * sizeof(float), (void*)(11 * sizeof(float)));
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1);
        
        // Spawn time (location 6) - 1 component for spawn time in seconds
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, _buffer_size * sizeof(float), (void*)(13 * sizeof(float)));
        glEnableVertexAttribArray(6);
        glVertexAttribDivisor(6, 1);
        
        // Flags (location 7) - 2 components for should_rotate and move flags
        glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, _buffer_size * sizeof(float), (void*)(14 * sizeof(float)));
        glEnableVertexAttribArray(7);
        glVertexAttribDivisor(7, 1);
        
        // Persistent OpenGL state setup for better performance (NEW OPTIMIZATION)
        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, trigTableTexture);
        
        instanced_initialized = true;
        
        std::cout << "Instanced rendering initialized with support for " << MAX_INSTANCES << " rectangles" << std::endl;
    }
    
    // Prepare instance data - use static vector to avoid allocations (OPTIMIZED)
    static std::vector<float> instance_data;
    instance_data.resize(rectangles.size() * _buffer_size); // 16 floats per instance now (added flags)
    
    size_t data_index = 0;
    for (const auto* rect : rectangles) {
        if (!rect) continue;
        // Send world coordinates to GPU (GPU will convert to screen coordinates)
        float x = rect->center.x;
        float y = rect->center.y;
        float w = rect->width;
        float h = rect->height;
        
        // Convert color to float [0,1]
        Color<float> glColor = rect->color.toGL();
        
        // Get velocity from the rectangle's move_offset (which contains velocity * speed)
        float velocity_x = rect->move_offset.x;
        float velocity_y = rect->move_offset.y;
        
        // Direct array access instead of insert() for better performance
        instance_data[data_index++] = x;                    // Initial world position X
        instance_data[data_index++] = y;                    // Initial world position Y
        instance_data[data_index++] = w;                    // World width
        instance_data[data_index++] = h;                    // World height
        instance_data[data_index++] = glColor.r;            // Color R
        instance_data[data_index++] = glColor.g;            // Color G
        instance_data[data_index++] = glColor.b;            // Color B
        instance_data[data_index++] = glColor.a;            // Color A
        instance_data[data_index++] = rect->initial_pitch;  // Initial pitch angle
        instance_data[data_index++] = rect->initial_yaw;    // Initial yaw angle
        instance_data[data_index++] = rect->initial_roll;   // Initial roll angle
        instance_data[data_index++] = velocity_x;           // Velocity X (world units per second)
        instance_data[data_index++] = velocity_y;           // Velocity Y (world units per second)
        instance_data[data_index++] = rect->spawn_time;     // Spawn time (seconds)
        instance_data[data_index++] = rect->should_rotate ? 1.0f : 0.0f; // Should rotate flag
        instance_data[data_index++] = rect->move ? 1.0f : 0.0f;          // Move flag
    }
    
    if (instance_data.empty()) return;
    
    // Upload instance data
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instance_data.size() * sizeof(float), instance_data.data());
    
    // Minimal state changes - OpenGL state is persistent from initialization (OPTIMIZED)
    glBindVertexArray(instanceVAO);
    
    // Set uniforms using cached locations (PERFORMANCE OPTIMIZATION)
    glUniform1i(uTrigTableLoc, 0); // Texture unit 0
    glUniform1f(uTrigTableSizeLoc, trigTableSize);
    glUniform2f(uScreenSizeLoc, static_cast<float>(cached_width), static_cast<float>(cached_height)); // NEW
    
    // NEW: Pass world coordinate system parameters to GPU
    glUniform1f(uWorldScaleLoc, world_scale);
    glUniform2f(uWorldOffsetLoc, world_offset_x, world_offset_y);
    
    // NEW: Pass time and rotation speed to GPU for angle calculation
    glUniform1f(uTimeLoc, static_cast<float>(glfwGetTime()));
    glUniform1f(uRotationSpeedLoc, rotation_speed);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, rectangles.size()); // 6 vertices per rectangle, N instances
    
    glBindVertexArray(0);
}



// ############# DEPRECATED #############

void draw_center_dots(const std::vector<objects::Rectangle*>& rectangles) {
    if (rectangles.empty()) return;
    
    // Use the batch shader for simple rendering
    glUseProgram(batchShaderProgram);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Enable point rendering and set point size
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(3.0f);
    
    // Reuse static vector to avoid allocations
    static std::vector<float> dot_vertices;
    dot_vertices.clear();
    dot_vertices.reserve(rectangles.size() * 6); // 6 floats per dot (2 pos + 4 color)
    
    for (const auto* rect : rectangles) {
        if (!rect) continue;
        
        // Convert to normalized device coordinates
        float x = (rect->center.x / cached_width) * 2.0f - 1.0f;
        float y = 1.0f - (rect->center.y / cached_height) * 2.0f;
        
        // Add vertex data: position + bright red color
        dot_vertices.insert(dot_vertices.end(), {
            x, y,                           // Position
            1.0f, 0.0f, 0.0f, 1.0f         // Bright red color (RGBA)
        });
    }
    
    if (dot_vertices.empty()) return;
    
    // Upload vertex data
    glBufferData(GL_ARRAY_BUFFER, dot_vertices.size() * sizeof(float), dot_vertices.data(), GL_DYNAMIC_DRAW);
    
    // Set vertex attribute pointers for position and color
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    
    // Draw points
    glDrawArrays(GL_POINTS, 0, dot_vertices.size() / 6);
    
    // Cleanup
    glDisable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(0);
}

void batch_draw_rectangles(const std::vector<objects::Rectangle*>& rectangles) {
    if (shaderProgram == 0 || rectangles.empty()) {
        return;
    }
    
    // Calculate total vertices needed (6 vertices per rectangle for 2 triangles)
    size_t total_vertices = rectangles.size() * 6;
    std::vector<float> batch_vertices;
    batch_vertices.reserve(total_vertices * 5); // 5 floats per vertex (2 pos + 3 color)
    
    // Build vertex data for all rectangles
    for (const auto* rect : rectangles) {
        const objects::Vec2List& points = rect->getRotatedPoints();
        if (points.size() != 4) continue; // Skip non-rectangle polygons
        
        // Get rectangle color in OpenGL format
        Color<float> glColor = rect->getGLColor();
        
        // Convert rectangle (4 vertices) to 2 triangles (6 vertices)
        // Triangle 1: points[0], points[1], points[2]
        // Triangle 2: points[0], points[2], points[3]
        
        const auto& p0 = points[0];
        const auto& p1 = points[1];
        const auto& p2 = points[2];
        const auto& p3 = points[3];
        
        // Convert to normalized device coordinates
        float x0 = (2.0f * p0.x / cached_width) - 1.0f;
        float y0 = 1.0f - (2.0f * p0.y / cached_height);
        float x1 = (2.0f * p1.x / cached_width) - 1.0f;
        float y1 = 1.0f - (2.0f * p1.y / cached_height);
        float x2 = (2.0f * p2.x / cached_width) - 1.0f;
        float y2 = 1.0f - (2.0f * p2.y / cached_height);
        float x3 = (2.0f * p3.x / cached_width) - 1.0f;
        float y3 = 1.0f - (2.0f * p3.y / cached_height);
        
        // Triangle 1: p0, p1, p2
        batch_vertices.insert(batch_vertices.end(), {
            x0, y0, glColor.r, glColor.g, glColor.b,
            x1, y1, glColor.r, glColor.g, glColor.b,
            x2, y2, glColor.r, glColor.g, glColor.b
        });
        
        // Triangle 2: p0, p2, p3
        batch_vertices.insert(batch_vertices.end(), {
            x0, y0, glColor.r, glColor.g, glColor.b,
            x2, y2, glColor.r, glColor.g, glColor.b,
            x3, y3, glColor.r, glColor.g, glColor.b
        });
    }
    
    if (batch_vertices.empty()) return;
    
    // Upload all vertex data at once
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, batch_vertices.size() * sizeof(float), batch_vertices.data(), GL_DYNAMIC_DRAW);
    
    // Set vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Draw all rectangles in a single draw call!
    glUseProgram(batchShaderProgram);
    glDrawArrays(GL_TRIANGLES, 0, batch_vertices.size() / 5);
    
    // Unbind
    glBindVertexArray(0);
}