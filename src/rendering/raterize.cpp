#include "rendering/rasterize.h"
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

#include "rendering/vertex_shader.h"
#include "rendering/fragment_shader.h"

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
    }
#endif


// Global OpenGL objects
static unsigned int shaderProgram = 0;
static unsigned int VAO = 0;
static unsigned int VBO = 0;


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
    
    // Compile shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    // Create shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }
    
    // Clean up individual shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
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
    std::cout << "Rasterizer cleaned up" << std::endl;
}

void draw_test_triangle() {
    if (shaderProgram == 0) {
        std::cerr << "Rasterizer not initialized! Call rasterize_init() first." << std::endl;
        return;
    }
    
    // Triangle vertices with positions and colors
    // Format: x, y, r, g, b, a
    float vertices[] = {
        // Top vertex (red)
         0.0f,  0.5f,   1.0f, 0.0f, 0.0f, 1.0f, // Position (0, 0.5) and color red
        // Bottom left vertex (green)  
        -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, 1.0f,
        // Bottom right vertex (blue)
         0.5f, -0.5f,   0.0f, 0.0f, 1.0f, 1.0f,
    };
    
    // Bind VAO
    glBindVertexArray(VAO);
    
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    // Set vertex attribute pointers
    // Position attribute (location 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (location 1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Use shader program and draw
    glUseProgram(shaderProgram);
    glDrawArrays(GL_TRIANGLES, 0, 4);
    
    // Unbind
    glBindVertexArray(0);
}

void draw_polygon(const objects::Polygon& polygon) {
    if (shaderProgram == 0) {
        std::cerr << "Rasterizer not initialized! Call rasterize_init() first." << std::endl;
        return;
    }
    
    const objects::Vec2List& points = polygon.getRotatedPoints();
    if (points.size() < 3) {
        return; // Need at least 3 points for a polygon
    }
    
    // Get polygon color in OpenGL format
    Color<float> glColor = polygon.getGLColor();
    
    // Create vertices array (position + color for each vertex)
    std::vector<float> vertices;
    vertices.reserve(points.size() * 5); // 2 pos + 3 color per vertex
    
    // Get window size for coordinate conversion
    int width, height;
    GLFWwindow* window = glfwGetCurrentContext();
    glfwGetWindowSize(window, &width, &height);
    
    for (const auto& point : points) {
        // Convert from pixel coordinates to normalized device coordinates (-1 to 1)
        float x = (2.0f * point.x / width) - 1.0f;
        float y = 1.0f - (2.0f * point.y / height); // Flip Y axis
        
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(glColor.r);
        vertices.push_back(glColor.g);
        vertices.push_back(glColor.b);
    }
    
    // Bind VAO
    glBindVertexArray(VAO);
    
    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    
    // Set vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Use shader program and draw
    glUseProgram(shaderProgram);
    
    if (polygon.filled) {
        // Draw filled polygon using triangle fan
        glDrawArrays(GL_TRIANGLE_FAN, 0, points.size());
    } else {
        // Draw outline using line loop
        glDrawArrays(GL_LINE_LOOP, 0, points.size());
    }
    
    // Unbind
    glBindVertexArray(0);
}

void draw_rectangle(const objects::Rectangle& rectangle) {
    // Rectangle inherits from Polygon, so we can just call draw_polygon
    draw_polygon(rectangle);
}
