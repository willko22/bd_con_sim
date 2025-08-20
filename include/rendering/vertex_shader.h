const char* vertexShaderSource = R"(
#version 460 core
layout (location = 0) in vec2 aPos;        // Base rectangle vertex position (0-1 range)
layout (location = 1) in vec2 aOffset;     // Per-instance INITIAL position offset (world coordinates)
layout (location = 2) in vec2 aSize;       // Per-instance size (world coordinates)
layout (location = 3) in vec4 aColor;      // Per-instance color (RGBA)
layout (location = 4) in vec3 aAngles;     // Per-instance INITIAL rotation angles (pitch, yaw, roll in radians)
layout (location = 5) in vec2 aVelocity;   // Per-instance velocity (world units per second)
layout (location = 6) in float aSpawnTime; // Per-instance spawn time (seconds)
layout (location = 7) in float aStopTime; // Per-instance spawn time (seconds)
layout (location = 8) in vec2 aFlags;      // Per-instance flags (x=should_rotate, y=move)

// Trig table texture and size
uniform sampler1D uTrigTable;
uniform float uTrigTableSize;

// Time and rotation speed for GPU-side angle calculation
uniform float uTime;           // Current time in seconds
uniform float uRotationSpeed;  // Rotation speed in radians per second

uniform float uVelocityChange; // Gravity or other velocity change factor

// World coordinate system uniforms for GPU-side conversion
uniform vec2 uScreenSize;     // Screen width and height
uniform float uWorldScale;    // Scale factor from world to screen coordinates
uniform vec2 uWorldOffset;    // Offset for centering world in screen

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
    // Calculate elapsed time since spawn for this specific rectangle
    float dt = 0.0;
    if (aStopTime > 0.0) {
        dt = aStopTime - aSpawnTime; // Time since spawn until stop
    } else {
        // If not stopped, use current time
        dt = uTime - aSpawnTime; // Time since spawn
    }

    // Center the base vertex position around (0,0) before rotation
    // aPos goes from (0,0) to (1,1), so center it to (-0.5,-0.5) to (0.5,0.5)
    vec2 centeredPos = aPos - vec2(0.5);
    
    // Calculate current position using initial position + (velocity * elapsed_time)
    // This moves the movement calculation to the GPU based on spawn time!
    // Only apply movement if the move flag is enabled (aFlags.y > 0.5)
    vec2 currentWorldPos = aOffset;
    // if (aFlags.y > 0.5) {
    //     vec2 aVelocity = aVelocity + vec2(0.0f, uVelocityChange * dt); // Apply velocity change factor (e.g., gravity)
    //     currentWorldPos += aVelocity * dt;
    // }
    
    // Convert world coordinates to screen coordinates
    vec2 screenPos = (currentWorldPos * uWorldScale) + uWorldOffset;
    
    // Convert screen coordinates to NDC on GPU 
    vec2 ndcOffset = (screenPos / uScreenSize) * 2.0 - 1.0;
    ndcOffset.y = -ndcOffset.y; // Flip Y coordinate for screen space
    
    // Convert world size to screen size, then to NDC size
    vec2 worldSizeScaled = aSize * uWorldScale; // World size to screen size
    vec2 ndcSize = (worldSizeScaled / uScreenSize) * 2.0; // Screen size to NDC size
    
    // Scale first using NDC size
    vec2 scaledPos = centeredPos * ndcSize;
    
    // Calculate rotation only if should_rotate flag is enabled (aFlags.x > 0.5)
    vec2 finalPos = scaledPos;
    if (aFlags.x > 0.5) {
        // Calculate current angles based on initial angles + (rotation_speed * elapsed_time)
        // This moves the angle increment calculation to the GPU based on spawn time!
        float currentPitch = aAngles.x + (uRotationSpeed * dt);
        float currentYaw = aAngles.y + (uRotationSpeed * dt);
        float currentRoll = aAngles.z + (uRotationSpeed * dt);
        
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
        float x = scaledPos.x;
        float y = scaledPos.y;
        
        // Apply the complete rotation matrix
        finalPos = vec2(
            x * m00 + y * m01,
            x * m10 + y * m11
        );
    }
    
    // Translate to final position using NDC offset
    gl_Position = vec4(ndcOffset + finalPos, 0.0, 1.0);
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

