const char* vertexShaderSource = R"(
#version 460 core
layout (location = 0) in vec2 aPos;        // Base rectangle vertex position (0-1 range)
layout (location = 1) in vec2 aOffset;     // Per-instance position offset (screen coordinates)
layout (location = 2) in vec2 aSize;       // Per-instance size (screen coordinates)
layout (location = 3) in vec4 aColor;      // Per-instance color (RGBA)
layout (location = 4) in vec3 aAngles;     // Per-instance INITIAL rotation angles (pitch, yaw, roll in radians)

// Trig table texture and size
uniform sampler1D uTrigTable;
uniform float uTrigTableSize;

// Time and rotation speed for GPU-side angle calculation
uniform float uTime;           // Current time in seconds
uniform float uRotationSpeed;  // Rotation speed in radians per second

// Screen dimensions for GPU-side NDC conversion (NEW OPTIMIZATION)
uniform vec2 uScreenSize;     // Screen width and height

out vec4 vertexColor;

// Function to look up sin/cos from the trig table
vec2 lookupTrig(float angle) {
    // Normalize angle to [0, 2π] range
    float normalized = mod(angle, 6.28318530718); // 2*PI
    if (normalized < 0.0) normalized += 6.28318530718;
    
    // Convert to texture coordinate [0, 1]
    float texCoord = normalized / 6.28318530718;
    
    // Lookup in texture (Red = sin, Green = cos)
    return texture(uTrigTable, texCoord).rg;
}

void main()
{

    // Center the base vertex position around (0,0) before rotation
    // aPos goes from (0,0) to (1,1), so center it to (-0.5,-0.5) to (0.5,0.5)
    vec2 centeredPos = aPos - vec2(0.5);
    
    // Convert screen coordinates to NDC on GPU (NEW OPTIMIZATION)
    vec2 ndcOffset = (aOffset / uScreenSize) * 2.0 - 1.0;
    ndcOffset.y = -ndcOffset.y; // Flip Y coordinate
    vec2 ndcSize = aSize / uScreenSize * 2.0;
    
    // Scale first using NDC size
    vec2 scaledPos = centeredPos * ndcSize;
    
    // Calculate current angles based on initial angles + (rotation_speed * time)
    // This moves the angle increment calculation to the GPU!
    float currentPitch = aAngles.x + (uRotationSpeed * uTime);
    float currentYaw = aAngles.y + (uRotationSpeed * uTime);
    float currentRoll = aAngles.z + (uRotationSpeed * uTime);
    
    // Look up trigonometric values from GPU table using current angles
    vec2 pitchTrig = lookupTrig(currentPitch);  // pitch (X-axis rotation)
    vec2 yawTrig = lookupTrig(currentYaw);      // yaw   (Y-axis rotation)  
    vec2 rollTrig = lookupTrig(currentRoll);    // roll  (Z-axis rotation)
    
    float sp = pitchTrig.x, cp = pitchTrig.y;  // sin/cos pitch
    float sy = yawTrig.x,   cy = yawTrig.y;    // sin/cos yaw
    float sr = rollTrig.x,  cr = rollTrig.y;   // sin/cos roll
    
    // Build complete 3D rotation matrix for ZYX order
    // For a point (x, y, 0), we only need the first two rows of the matrix
    float m00 = cy * cr;
    float m01 = -cy * sr;
    float m10 = sp * sy * cr + cp * sr;
    float m11 = -sp * sy * sr + cp * cr;
    
    // Apply rotation to 2D position (treating z=0)
    vec2 pos = scaledPos;
    float x = pos.x;
    float y = pos.y;
    
    // Apply the complete rotation matrix
    vec2 rotatedPos = vec2(
        x * m00 + y * m01,
        x * m10 + y * m11
    );
    
    // Translate to final position using NDC offset
    gl_Position = vec4(ndcOffset + rotatedPos, 0.0, 1.0);
    vertexColor = aColor;
}
)";

// // Fallback vertex shader for non-instanced rendering (batch mode)
// const char* batchVertexShaderSource = R"(
// #version 460 core
// layout (location = 0) in vec2 aPos;
// layout (location = 1) in vec4 aColor;

// out vec4 vertexColor;

// void main()
// {
//     gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
//     vertexColor = aColor;
// }
// )";

