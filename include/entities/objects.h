#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>
#include <unordered_set>
#include <memory>
#include <cmath>


// Instead of size_t (64-bit), use smaller types:
using u16 = uint16_t;  // 0-65k range - fine for dimensions, colors
using u8 = uint8_t;    // 0-255 range - perfect for counters, colors
using u32 = uint32_t;  // 0-4B range - for calculations that might overflow u16
using i32 = int32_t;   // Signed 32-bit for offset calculations
using i16 = int16_t;   // -32k to +32k range - fine for screen coordinates
constexpr float inv255 = 1.0f / 255.0f;

template<typename T>
struct Color {
    // vector of 4 elements specifically for RGBA colors
    // represented by default as (r, g, b, a)
    // each element can be accessed separately
    T r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(255) {}
    Color(T red, T green, T blue, T alpha = 255) : r(red), g(green), b(blue), a(alpha) {}
    
    // Convert to OpenGL format (0.0 to 1.0)
    Color<float> toGL() const {
        return Color<float>(static_cast<float>(r) * inv255,
                          static_cast<float>(g) * inv255,
                          static_cast<float>(b) * inv255,
                          static_cast<float>(a) * inv255);
    }
    
    // SIMD-friendly operations
    Color<T> operator+(const Color<T>& other) const {
        return Color<T>(r + other.r, g + other.g, b + other.b, a + other.a);
    }
    
    Color<T> operator*(float factor) const {
        return Color<T>(static_cast<T>(r * factor), 
                       static_cast<T>(g * factor),
                       static_cast<T>(b * factor), 
                       static_cast<T>(a * factor));
    }
    
    // Direct memory access for batch operations
    T* data() { return &r; }
    const T* data() const { return &r; }
};

namespace objects {

struct Vec2 {
    i16 x, y;
    constexpr Vec2() noexcept : x(0), y(0) {}
    constexpr Vec2(i16 x, i16 y) noexcept : x(x), y(y) {}
    
    // Array-style access: [0] = x, [1] = y
    constexpr i16& operator[](size_t index) noexcept {
        return index == 0 ? x : y;
    }
    
    constexpr const i16& operator[](size_t index) const noexcept {
        return index == 0 ? x : y;
    }
    
    // For use in unordered_set
    constexpr bool operator==(const Vec2& other) const noexcept {
        return x == other.x && y == other.y;
    }
    
    // Vector operations for game math
    constexpr Vec2 operator+(const Vec2& other) const noexcept {
        return Vec2(x + other.x, y + other.y);
    }
    
    constexpr Vec2 operator-(const Vec2& other) const noexcept {
        return Vec2(x - other.x, y - other.y);
    }
    
    constexpr Vec2 operator*(i16 scalar) const noexcept {
        return Vec2(x * scalar, y * scalar);
    }
    
    // Dot product for distance calculations
    constexpr i32 dot(const Vec2& other) const noexcept {
        return static_cast<i32>(x) * other.x + static_cast<i32>(y) * other.y;
    }
    
    // Squared magnitude (avoids sqrt for distance comparisons)
    constexpr i32 magnitudeSquared() const noexcept {
        return static_cast<i32>(x) * x + static_cast<i32>(y) * y;
    }
    
    // Magnitude (actual length)
    float magnitude() const noexcept {
        return std::sqrt(static_cast<float>(magnitudeSquared()));
    }
    
    // Normalize vector to unit length
    Vec2 normalize() const noexcept {
        float mag = magnitude();
        if (mag > 0.0f) {
            float inv_mag = 1.0f / mag;
            return Vec2(static_cast<i16>(x * inv_mag), static_cast<i16>(y * inv_mag));
        }
        return Vec2(0, 0);
    }
};

// Hash function for Vec2 to use in unordered_set
struct Vec2Hash {
    constexpr u16 operator()(const Vec2& v) const noexcept {
        // Morton encoding for better spatial locality
        // Combines both coordinates into a single hash more efficiently
        u16 ux = static_cast<u16>(v.x + 32768); // Shift to unsigned range
        u16 uy = static_cast<u16>(v.y + 32768);
        return static_cast<u16>((ux << 8) | uy) ^ ((uy << 4) | (ux >> 4));
    }
};

struct Vec2Set {
    // Pre-allocates memory for expected number of points
    // as unordered set of points
    std::unordered_set<Vec2, Vec2Hash> points;
    
    Vec2Set() = default;
    
    // Constructor with capacity hint for better performance
    explicit Vec2Set(size_t capacity) {
        points.reserve(capacity);
    }
    
    void add(i16 x, i16 y) noexcept {
        points.emplace(x, y);
    }
    
    void add(const Vec2& v) noexcept {
        points.insert(v);
    }
    
    bool has(const Vec2& v) const noexcept {
        return points.find(v) != points.end();
    }
    
    size_t size() const noexcept {
        return points.size();
    }
    
    void reserve(size_t capacity) {
        points.reserve(capacity);
    }
};

struct Vec2List {
    // Dynamic vector-based storage with pre-allocation
    std::vector<Vec2> points;
    
    Vec2List() = default;
    
    // Constructor with capacity for better performance
    explicit Vec2List(size_t capacity) {
        points.reserve(capacity);
    }
    
    // Constructor with fixed size (replaces Vec2Fixed functionality)
    static Vec2List withFixedSize(size_t size) {
        Vec2List list;
        list.points.reserve(size);
        return list;
    }
    
    void add(i16 x, i16 y) {
        points.emplace_back(x, y);
    }
    
    void add(const Vec2& v) {
        points.push_back(v);
    }
    
    Vec2& operator[](size_t index) noexcept {
        return points[index];
    }
    
    const Vec2& operator[](size_t index) const noexcept {
        return points[index];
    }
    
    size_t size() const noexcept {
        return points.size();
    }
    
    size_t capacity() const noexcept {
        return points.capacity();
    }
    
    bool empty() const noexcept {
        return points.empty();
    }
    
    void clear() noexcept {
        points.clear();
    }
    
    void reserve(size_t capacity) {
        points.reserve(capacity);
    }
    
    // Pre-allocate exact size and fill (useful for fixed-size scenarios)
    void resize(size_t size) {
        points.resize(size);
    }
    
    void resize(size_t size, const Vec2& value) {
        points.resize(size, value);
    }
    
    // Iterator support for range-based loops
    auto begin() noexcept { return points.begin(); }
    auto end() noexcept { return points.end(); }
    auto begin() const noexcept { return points.begin(); }
    auto end() const noexcept { return points.end(); }
};

struct BBox {
    i16 x, y; // left top - can be negative
    u16 width, height; // dimensions - always positive
    
    BBox() : x(0), y(0), width(0), height(0) {}
    BBox(i16 x, i16 y, u16 width, u16 height) : x(x), y(y), width(width), height(height) {}
};


/**
 * Polygon structure with 3D rotation support
 * 
 * Rotation System Explanation:
 * ---------------------------
 * The polygon supports three types of rotation using standard aviation terminology:
 * 
 * 1. PITCH (rotation around X-axis):
 *    - Positive pitch: "nose up" motion - front of object tilts upward
 *    - Negative pitch: "nose down" motion - front of object tilts downward
 *    - In 2D view: affects the Y-coordinate more dramatically than X
 * 
 * 2. YAW (rotation around Y-axis):
 *    - Positive yaw: "turn left" motion - object rotates counterclockwise when viewed from above
 *    - Negative yaw: "turn right" motion - object rotates clockwise when viewed from above
 *    - In 2D view: affects the X-coordinate more dramatically than Y
 * 
 * 3. ROLL (rotation around Z-axis):
 *    - Positive roll: "bank right" motion - right side tilts down, left side tilts up
 *    - Negative roll: "bank left" motion - left side tilts down, right side tilts up
 *    - In 2D view: traditional 2D rotation, both X and Y coordinates change equally
 * 
 * Rotation Order: Pitch -> Yaw -> Roll (Tait-Bryan angles)
 * 
 * Point Storage:
 * --------------
 * - points_original: Non-rotated reference points
 * - points_rotated: Points after applying all current rotations (used for rendering)
 * 
 * Point Addition Modes:
 * ---------------------
 * Mode 0: Add point to original list, compute rotated version automatically
 * Mode 1: Add point to rotated list, compute original version automatically  
 * Mode 2: Add point to both lists at current position (no rotation applied)
 */
struct Polygon {
    Color<u8> color;        // 4 bytes
    Vec2List points_original;   // 24 bytes (vector) - non-rotated points
    Vec2List points_rotated;    // 24 bytes (vector) - rotated points
    BBox bbox;              // 8 bytes
    bool filled = true;     // 1 byte (default to filled)
    float pitch = 0.0f;     // 4 bytes (rotation around x-axis - nodding up/down)
    float yaw = 0.0f;       // 4 bytes (rotation around y-axis - turning left/right)
    float roll = 0.0f;      // 4 bytes (rotation around z-axis - tilting left/right)
    Vec2 center;            // 4 bytes (center point for rotation)

    Polygon() = default;
    
    // Constructor with position - BBox is calculated automatically
    Polygon(const Color<u8>& c, i16 x = 0, i16 y = 0) 
        : color(c), bbox(x, y, 0, 0), center(x, y) {}
    
    // Constructor with points - BBox is calculated from points
    Polygon(const Color<u8>& c, const Vec2List& pts) 
        : color(c), points_original(pts), points_rotated(pts) {
        _calculateBBox();
    }
    
    // Constructor with points and position
    Polygon(const Color<u8>& c, const Vec2List& pts, i16 x, i16 y) 
        : color(c), points_original(pts), points_rotated(pts) {
        _calculateBBox();
        _movePoly(x, y);
    }

    Color<float> getGLColor() const {
        return color.toGL();
    }
    
    // Add point and recalculate BBox and center
    void addPoint(i16 x, i16 y) {
        addPoint(x, y, 0); // Default mode: add to current position into both lists
    }
    
    void addPoint(const Vec2& point) {
        addPoint(point.x, point.y, 0); // Default mode
    }
    
    // Add point with mode:
    // Mode 0: Add to current position into original and into rotated version as rotated by current rotation
    // Mode 1: Add to rotated as current position and into original as rotated in reverse
    // Mode 2: Add to both as current position
    void addPoint(i16 x, i16 y, int mode) {
        Vec2 point(x, y);
        
        switch(mode) {
            case 0: {
                // Add to original as current position
                points_original.add(point);
                // Add to rotated as current position rotated by current rotations
                Vec2 rotated_point = _applyRotations(point);
                points_rotated.add(rotated_point);
                break;
            }
            case 1: {
                // Add to rotated as current position
                points_rotated.add(point);
                // Add to original as current position rotated in reverse
                Vec2 original_point = _applyReverseRotations(point);
                points_original.add(original_point);
                break;
            }
            case 2: {
                // Add to both as current position
                points_original.add(point);
                points_rotated.add(point);
                break;
            }
        }
        _calculateBBox();
    }
    
    // Get the current bounding box
    const BBox& getBBox() const {
        return bbox;
    }
    
    // Get the center point
    const Vec2& getCenter() const {
        return center;
    }
    
    // Get current rotation in radians for roll (backward compatibility with Z-axis rotation)
    float getRotation() const {
        return roll;
    }
    
    // Get current rotations for all axes with descriptive names
    // Pitch: Rotation around X-axis (nodding up/down) - positive pitch tilts front up
    // Yaw: Rotation around Y-axis (turning left/right) - positive yaw turns left
    // Roll: Rotation around Z-axis (tilting left/right) - positive roll tilts right side down
    float getPitch() const { return pitch; }
    float getYaw() const { return yaw; }
    float getRoll() const { return roll; }
    

    
    // Get the original (non-rotated) points
    const Vec2List& getOriginalPoints() const {
        return points_original;
    }
    
    // Get the rotated points (these are what should be rendered)
    const Vec2List& getRotatedPoints() const {
        return points_rotated;
    }
    
    // Get the points for rendering (alias for getRotatedPoints for clarity)
    const Vec2List& getPoints() const {
        return points_rotated;
    }
    
    // Recalculate BBox and center from current rotated points
    void updateBBox() {
        _calculateBBox();
    }
    
    // Move polygon so its BBox starts at specified position
    void moveTo(i16 x, i16 y) {
        _movePoly(x, y);
    }
    
    // Main rotation functions - clean and consistent
    // ===============================================
    // 
    // These are the primary rotation functions with a clean, consistent interface:
    // 
    // setRotation(pitch, yaw, roll) - Sets ABSOLUTE rotation angles for all axes
    //   - All parameters default to 0.0f, so you can specify only the axes you want
    //   - Example: setRotation(0.5f) sets pitch to 0.5 radians, yaw and roll to 0
    //   - Example: setRotation(0.0f, 1.0f) sets yaw to 1.0 radians, pitch and roll to 0
    // 
    // rotate(pitch, yaw, roll) - ADDS rotation amounts to current rotation
    //   - All parameters default to 0.0f, so you can specify only the axes you want
    //   - Example: rotate(0.1f) adds 0.1 radians to current pitch
    //   - Example: rotate(0.0f, 0.0f, 0.2f) adds 0.2 radians to current roll
    
    // Set absolute rotation values for all axes (sets specific angles)
    void setRotation(float pitch_rad = 0.0f, float yaw_rad = 0.0f, float roll_rad = 0.0f) {
        pitch = pitch_rad;
        yaw = yaw_rad;
        roll = roll_rad;
        _updateRotatedPoints();
        _calculateBBox();
    }
    
    // Add to current rotation values for all axes (rotates by specified amounts)
    void rotate(float pitch_delta = 0.0f, float yaw_delta = 0.0f, float roll_delta = 0.0f) {
        pitch += pitch_delta;
        yaw += yaw_delta;
        roll += roll_delta;
        _updateRotatedPoints();
        _calculateBBox();
    }
    
    
    // Individual axis convenience methods
    // ===================================
    
    void setPitch(float angle_radians) {
        setRotation(angle_radians, yaw, roll);
    }
    
    void setYaw(float angle_radians) {
        setRotation(pitch, angle_radians, roll);
    }
    
    void setRoll(float angle_radians) {
        setRotation(pitch, yaw, angle_radians);
    }
    
    void rotatePitch(float angle_radians) {
        rotate(angle_radians, 0.0f, 0.0f);
    }
    
    void rotateYaw(float angle_radians) {
        rotate(0.0f, angle_radians, 0.0f);
    }
    
    void rotateRoll(float angle_radians) {
        rotate(0.0f, 0.0f, angle_radians);
    }

protected:
    // Move all points so the bounding box starts at target position
    void _movePoly(i16 target_x, i16 target_y) {
        if (points_original.empty()) {
            return;
        }
        
        i16 current_x = bbox.x, current_y = bbox.y;

        bbox.x = target_x;
        bbox.y = target_y;
        
        // Calculate offset needed to move polygon to target position
        i16 offset_x = target_x - current_x;
        i16 offset_y = target_y - current_y;
        
        // Apply offset to all original points
        for (auto& p : points_original) {
            p.x += offset_x;
            p.y += offset_y;
        }
        
        // Apply offset to all rotated points
        for (auto& p : points_rotated) {
            p.x += offset_x;
            p.y += offset_y;
        }
        
        // Update center as well
        center.x += offset_x;
        center.y += offset_y;
    }

    void _calculateBBox(i16 offset_x = 0, i16 offset_y = 0) {
        if (points_rotated.empty()) {
            bbox = BBox(offset_x, offset_y, 0, 0);
            return;
        }
        
        i16 min_x = INT16_MAX, min_y = INT16_MAX;
        i16 max_x = INT16_MIN, max_y = INT16_MIN;
        
        // Find bounds of all rotated points (these are what's actually rendered)
        for (const auto& p : points_rotated) {
            min_x = std::min(min_x, p.x);
            min_y = std::min(min_y, p.y);
            max_x = std::max(max_x, p.x);
            max_y = std::max(max_y, p.y);
        }
        
        bbox = BBox(min_x, min_y, static_cast<u16>(max_x - min_x), static_cast<u16>(max_y - min_y));
        
        center.x = bbox.x + static_cast<i16>(bbox.width / 2);
        center.y = bbox.y + static_cast<i16>(bbox.height / 2);
    }
 
    // Apply current rotations to a single point
    Vec2 _applyRotations(const Vec2& point) {
        Vec2 result = point;
        
        // For 2D representation, we simulate 3D rotations by applying transformations
        // Rotation order: Pitch (X) -> Yaw (Y) -> Roll (Z) - Standard Tait-Bryan angles
        
        float x = static_cast<float>(result.x - center.x);
        float y = static_cast<float>(result.y - center.y);
        float z = 0.0f; // Assume z = 0 for 2D points
        
        // Apply Pitch rotation (rotation around x-axis - nodding up/down)
        // Positive pitch rotates the front up (nose up in aviation)
        if (pitch != 0.0f) {
            float cos_pitch = std::cos(pitch);
            float sin_pitch = std::sin(pitch);
            float new_y = y * cos_pitch - z * sin_pitch;
            float new_z = y * sin_pitch + z * cos_pitch;
            y = new_y;
            z = new_z;
        }
        
        // Apply Yaw rotation (rotation around y-axis - turning left/right)
        // Positive yaw rotates left (turning left in aviation)
        if (yaw != 0.0f) {
            float cos_yaw = std::cos(yaw);
            float sin_yaw = std::sin(yaw);
            float new_x = x * cos_yaw + z * sin_yaw;
            float new_z = -x * sin_yaw + z * cos_yaw;
            x = new_x;
            z = new_z;
        }
        
        // Apply Roll rotation (rotation around z-axis - tilting left/right)
        // Positive roll tilts the right side down (banking right in aviation)
        if (roll != 0.0f) {
            float cos_roll = std::cos(roll);
            float sin_roll = std::sin(roll);
            float new_x = x * cos_roll - y * sin_roll;
            float new_y = x * sin_roll + y * cos_roll;
            x = new_x;
            y = new_y;
        }
        
        result.x = static_cast<i16>(x + center.x);
        result.y = static_cast<i16>(y + center.y);
        
        return result;
    }
    
    // Apply reverse rotations to a single point
    Vec2 _applyReverseRotations(const Vec2& point) {
        Vec2 result = point;
        
        float x = static_cast<float>(result.x - center.x);
        float y = static_cast<float>(result.y - center.y);
        float z = 0.0f;
        
        // Apply rotations in reverse order with negative angles
        // Reverse order: Roll -> Yaw -> Pitch
        
        // Reverse Roll rotation
        if (roll != 0.0f) {
            float cos_roll = std::cos(-roll);
            float sin_roll = std::sin(-roll);
            float new_x = x * cos_roll - y * sin_roll;
            float new_y = x * sin_roll + y * cos_roll;
            x = new_x;
            y = new_y;
        }
        
        // Reverse Yaw rotation
        if (yaw != 0.0f) {
            float cos_yaw = std::cos(-yaw);
            float sin_yaw = std::sin(-yaw);
            float new_x = x * cos_yaw + z * sin_yaw;
            float new_z = -x * sin_yaw + z * cos_yaw;
            x = new_x;
            z = new_z;
        }
        
        // Reverse Pitch rotation
        if (pitch != 0.0f) {
            float cos_pitch = std::cos(-pitch);
            float sin_pitch = std::sin(-pitch);
            float new_y = y * cos_pitch - z * sin_pitch;
            float new_z = y * sin_pitch + z * cos_pitch;
            y = new_y;
            z = new_z;
        }
        
        result.x = static_cast<i16>(x + center.x);
        result.y = static_cast<i16>(y + center.y);
        
        return result;
    }
    
    // Update all rotated points based on current rotations
    void _updateRotatedPoints() {
        points_rotated.clear();
        points_rotated.reserve(points_original.size());
        
        for (const auto& original_point : points_original) {
            Vec2 rotated_point = _applyRotations(original_point);
            points_rotated.add(rotated_point);
        }
    }
    
    // Legacy method - kept for backward compatibility with Rectangle class
    void _rotatePoints(float angle_radians, const Vec2& pivot) {
        float cos_a = std::cos(angle_radians);
        float sin_a = std::sin(angle_radians);
        
        for (auto& p : points_rotated) {
            // Translate to pivot origin
            float x = static_cast<float>(p.x - pivot.x);
            float y = static_cast<float>(p.y - pivot.y);
            
            // Rotate
            float rotated_x = x * cos_a - y * sin_a;
            float rotated_y = x * sin_a + y * cos_a;
            
            // Translate back and update point
            p.x = static_cast<i16>(rotated_x + pivot.x);
            p.y = static_cast<i16>(rotated_y + pivot.y);
        }
    }
};

// Rectangle class - inherits from Polygon with specific constraints
struct Rectangle : public Polygon {
    u16 width, height;  // Rectangle dimensions (always positive)
    
    // Constructor with position, dimensions, color, and optional rotation
    Rectangle(i16 x, i16 y, u16 w, u16 h, const Color<u8>& c, float rotation_radians = 0.0f) 
        : Polygon(c), width(w), height(h) {
        _createRectanglePoints(x, y, w, h);
        _calculateBBox();
        
        if (rotation_radians != 0.0f) {
            rotate(0.0f, 0.0f, rotation_radians); // Use the new 3-axis rotate function
        }
    }
    
    // Constructor with just dimensions and color (positioned at origin)
    Rectangle(u16 w, u16 h, const Color<u8>& c, float rotation_radians = 0.0f) 
        : Rectangle(0, 0, w, h, c, rotation_radians) {}
    
    // Resize rectangle (maintains position and rotation)
    void resize(u16 new_width, u16 new_height) {
        if (new_width == width && new_height == height) return;
        
        width = new_width;
        height = new_height;
        
        // Save current state
        Vec2 current_center = center;
        float current_pitch = pitch;
        float current_yaw = yaw;
        float current_roll = roll;
        
        // Recreate rectangle at origin with no rotation
        pitch = yaw = roll = 0.0f;
        _createRectanglePoints(0, 0, width, height);
        _calculateBBox();
        
        // Restore rotations
        if (current_pitch != 0.0f || current_yaw != 0.0f || current_roll != 0.0f) {
            pitch = current_pitch;
            yaw = current_yaw;
            roll = current_roll;
            _updateRotatedPoints();
        }
        
        // Restore position
        moveTo(current_center.x - static_cast<i16>(width / 2), 
               current_center.y - static_cast<i16>(height / 2));
    }
    
    // Get rectangle dimensions
    u16 getWidth() const { return width; }
    u16 getHeight() const { return height; }
    
    // Override addPoint to prevent modification (rectangles must stay rectangles)
    void addPoint(i16 x, i16 y) = delete;
    void addPoint(const Vec2& point) = delete;
    void addPoint(i16 x, i16 y, int mode) = delete;

private:
    // Create the 4 corner points of a rectangle
    void _createRectanglePoints(i16 x, i16 y, u16 w, u16 h) {
        points_original.clear();
        points_original.reserve(4);
        points_rotated.clear();
        points_rotated.reserve(4);
        
        // Add 4 corners: top-left, top-right, bottom-right, bottom-left
        points_original.add(x, y);                    // Top-left
        points_original.add(x + w, y);                // Top-right  
        points_original.add(x + w, y + h);            // Bottom-right
        points_original.add(x, y + h);                // Bottom-left
        
        // Initially, rotated points are the same as original
        points_rotated = points_original;
    }
};

} // namespace objects

