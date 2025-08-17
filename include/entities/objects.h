#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <unordered_set>
#include <memory>
#include <cmath>
#include <limits>
#include <functional>

#include "utils/globals.h"

// Instead of size_t (64-bit), use smaller types:

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
    float x, y;
    constexpr Vec2() noexcept : x(0.0f), y(0.0f) {}
    constexpr Vec2(float x, float y) noexcept : x(x), y(y) {}
    
    // Convenience constructors for integer inputs (common in games)
    constexpr Vec2(int x, int y) noexcept : x(static_cast<float>(x)), y(static_cast<float>(y)) {}
    constexpr Vec2(i16 x, i16 y) noexcept : x(static_cast<float>(x)), y(static_cast<float>(y)) {}
    
    // Array-style access: [0] = x, [1] = y
    constexpr float& operator[](size_t index) noexcept {
        return index == 0 ? x : y;
    }
    
    constexpr const float& operator[](size_t index) const noexcept {
        return index == 0 ? x : y;
    }
    
    // For use in unordered_set (with epsilon comparison for floats)
    constexpr bool operator==(const Vec2& other) const noexcept {
        constexpr float epsilon = 0.001f;
        return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon;
    }
    
    // Vector operations for game math
    constexpr Vec2 operator+(const Vec2& other) const noexcept {
        return Vec2(x + other.x, y + other.y);
    }
    
    constexpr Vec2 operator-(const Vec2& other) const noexcept {
        return Vec2(x - other.x, y - other.y);
    }
    
    constexpr Vec2 operator*(float scalar) const noexcept {
        return Vec2(x * scalar, y * scalar);
    }
    
    constexpr Vec2 operator/(float scalar) const noexcept {
        return Vec2(x / scalar, y / scalar);
    }
    
    // Compound assignment operators
    Vec2& operator+=(const Vec2& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    Vec2& operator-=(const Vec2& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    
    Vec2& operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    
    Vec2& operator/=(float scalar) noexcept {
        x /= scalar;
        y /= scalar;
        return *this;
    }
    
    // Dot product for distance calculations
    constexpr float dot(const Vec2& other) const noexcept {
        return x * other.x + y * other.y;
    }
    
    // Cross product (returns scalar for 2D)
    constexpr float cross(const Vec2& other) const noexcept {
        return x * other.y - y * other.x;
    }
    
    // Squared magnitude (avoids sqrt for distance comparisons)
    constexpr float magnitudeSquared() const noexcept {
        return x * x + y * y;
    }
    
    // Magnitude (actual length)
    float magnitude() const noexcept {
        return std::sqrt(magnitudeSquared());
    }
    
    // Normalize vector to unit length
    Vec2 normalize() const noexcept {
        const float mag = magnitude();
        if (mag > 0.0f) {
            const float inv_mag = 1.0f / mag;
            return Vec2(x * inv_mag, y * inv_mag);
        }
        return Vec2(0.0f, 0.0f);
    }
    
    // Linear interpolation between two vectors
    static Vec2 lerp(const Vec2& a, const Vec2& b, float t) noexcept {
        return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
    }
    
    // Distance between two points
    float distanceTo(const Vec2& other) const noexcept {
        return (*this - other).magnitude();
    }
    
    // Distance squared (faster for comparisons)
    float distanceSquaredTo(const Vec2& other) const noexcept {
        return (*this - other).magnitudeSquared();
    }
    
    // Convert to integer coordinates (for pixel-perfect rendering)
    Vec2 toInt() const noexcept {
        return Vec2(std::round(x), std::round(y));
    }
    
    // Floor to integer coordinates
    Vec2 floor() const noexcept {
        return Vec2(std::floor(x), std::floor(y));
    }
    
    // Ceiling to integer coordinates
    Vec2 ceil() const noexcept {
        return Vec2(std::ceil(x), std::ceil(y));
    }
    
    // Get integer x and y (for compatibility with existing code)
    i16 getIntX() const noexcept { return static_cast<i16>(std::round(x)); }
    i16 getIntY() const noexcept { return static_cast<i16>(std::round(y)); }
};

// Hash function for Vec2 to use in unordered_set
struct Vec2Hash {
    constexpr u32 operator()(const Vec2& v) const noexcept {
        // Convert floats to integers for hashing (with reasonable precision)
        const u32 x_hash = std::hash<float>{}(std::round(v.x * 1000.0f) / 1000.0f);
        const u32 y_hash = std::hash<float>{}(std::round(v.y * 1000.0f) / 1000.0f);
        
        // Combine hashes using a good mixing function
        return x_hash ^ (y_hash + 0x9e3779b9 + (x_hash << 6) + (x_hash >> 2));
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
    
    void add(float x, float y) noexcept {
        points.emplace(x, y);
    }
    
    void add(i16 x, i16 y) noexcept {
        points.emplace(static_cast<float>(x), static_cast<float>(y));
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
    
    void add(float x, float y) {
        points.emplace_back(x, y);
    }
    
    void add(i16 x, i16 y) {
        points.emplace_back(static_cast<float>(x), static_cast<float>(y));
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
    float x, y; // left top - can be negative
    float width, height; // dimensions - always positive
    
    BBox() : x(0.0f), y(0.0f), width(0.0f), height(0.0f) {}
    BBox(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}
    
    // Convenience constructors for integer inputs
    BBox(i16 x, i16 y, u16 width, u16 height) 
        : x(static_cast<float>(x)), y(static_cast<float>(y)), 
          width(static_cast<float>(width)), height(static_cast<float>(height)) {}
    
    // Check if a point is inside the bounding box
    bool contains(const Vec2& point) const noexcept {
        return point.x >= x && point.x <= x + width && 
               point.y >= y && point.y <= y + height;
    }
    
    // Check if another bounding box intersects with this one
    bool intersects(const BBox& other) const noexcept {
        return x < other.x + other.width && 
               x + width > other.x && 
               y < other.y + other.height && 
               y + height > other.y;
    }
    
    // Get center point of the bounding box
    Vec2 getCenter() const noexcept {
        return Vec2(x + width * 0.5f, y + height * 0.5f);
    }
    
    // Get area of the bounding box
    float getArea() const noexcept {
        return width * height;
    }
    
    // Expand bounding box by a margin
    BBox expand(float margin) const noexcept {
        return BBox(x - margin, y - margin, width + 2 * margin, height + 2 * margin);
    }
    
    // Convert to integer bounding box (for pixel-perfect operations)
    BBox toInt() const noexcept {
        return BBox(std::floor(x), std::floor(y), 
                   std::ceil(width), std::ceil(height));
    }
    
    // Get integer coordinates (for compatibility)
    i16 getIntX() const noexcept { return static_cast<i16>(std::round(x)); }
    i16 getIntY() const noexcept { return static_cast<i16>(std::round(y)); }
    u16 getIntWidth() const noexcept { return static_cast<u16>(std::round(width)); }
    u16 getIntHeight() const noexcept { return static_cast<u16>(std::round(height)); }
};

struct BCircle {
    Vec2 center; // Center of the circle
    float radius; // Radius of the circle (6 decimal precision)
    
    // Cache radius squared to avoid repeated multiplication
    mutable float radius_squared;
    
    BCircle() noexcept : center(0, 0), radius(0.0f), radius_squared(0.0f) {}
    BCircle(const Vec2& center, float radius) noexcept : center(center), radius(radius) {
        setRadius(radius);
    }
    BCircle(i16 x, i16 y, float radius) noexcept : center(x, y), radius(radius) {
        setRadius(radius);
    }

    // Update radius and precompute radius squared
    void setRadius(float new_radius) noexcept {
        radius = new_radius;
        radius_squared = new_radius * new_radius;
    }
    
    // Fast distance squared calculation - optimized with float precision
    inline float distanceSquaredTo(const Vec2& point) const noexcept {
        const float dx = point.x - center.x;
        const float dy = point.y - center.y;
        return dx * dx + dy * dy;
    }

    // Fast distance squared to another circle's center
    inline float distanceSquaredTo(const BCircle& other) const noexcept {
        return distanceSquaredTo(other.center);
    }

    // Optimized point containment check
    inline bool contains(const Vec2& point) const noexcept {
        return distanceSquaredTo(point) <= radius_squared;
    }
    
    // Optimized circle intersection with early exits and branch prediction hints
    bool intersects(const BCircle& other) const noexcept {
        // Pre-calculate combined radius once
        const float combined_radius = radius + other.radius;
        
        // Early exit: Manhattan distance check (very fast, eliminates ~50% of cases)
        const float dx_abs = std::abs(other.center.x - center.x);
        const float dy_abs = std::abs(other.center.y - center.y);
        
        // Quick rejection: if Manhattan distance > combined radius * sqrt(2), definitely no intersection
        if (dx_abs + dy_abs > combined_radius * 1.414213f) {
            return false;
        }
        
        // Quick acceptance: if both dx and dy are small, definitely intersecting
        if (dx_abs <= radius && dy_abs <= radius) {
            return true;
        }
        
        // Precise check only when needed
        const float distanceSquared = dx_abs * dx_abs + dy_abs * dy_abs;
        const float combinedRadiusSquared = combined_radius * combined_radius;
        return distanceSquared <= combinedRadiusSquared;
    }
    
    // Fast overlap distance calculation with early exit
    inline float overlapDistance(const BCircle& other) const noexcept {
        const float distanceSquared = distanceSquaredTo(other);
        const float combined_radius = radius + other.radius;
        const float combinedRadiusSquared = combined_radius * combined_radius;
        
        // Early exit for no overlap
        if (distanceSquared >= combinedRadiusSquared) {
            return 0.0f;
        }
        
        // Calculate actual overlap (expensive sqrt only when needed)
        const float distance = std::sqrt(distanceSquared);
        return combined_radius - distance;
    }
    
    // Check containment of another circle with optimized early exits
    inline bool contains(const BCircle& other) const noexcept {
        // Early exit: can't contain a larger or equal circle
        if (radius <= other.radius) {
            return false;
        }

        const float distanceSquared = distanceSquaredTo(other);
        const float radius_diff = radius - other.radius;
        const float containmentThresholdSquared = radius_diff * radius_diff;
        
        return distanceSquared <= containmentThresholdSquared;
    }
    
    // Additional utility methods for common operations
    
    // Check if circle is completely within screen bounds (fast screen culling)
    inline bool isOnScreen(u16 screen_width, u16 screen_height) const noexcept {
        return center.x >= radius && 
               center.y >= radius && 
               center.x + radius <= static_cast<float>(screen_width) && 
               center.y + radius <= static_cast<float>(screen_height);
    }
    
    // Move circle by offset (faster than recreating)
    inline void moveBy(float dx, float dy) noexcept {
        center.x += dx;
        center.y += dy;
    }
    
    // Move circle to new position
    inline void moveTo(float x, float y) noexcept {
        center.x = x;
        center.y = y;
    }
    
    // Scale circle by factor (maintains center)
    inline void scale(float factor) noexcept {
        setRadius(radius * factor);
    }
    
    // Get bounding box for this circle (useful for spatial partitioning)
    inline BBox getBBox() const noexcept {
        return BBox(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);
    }
    
    // Fast area calculation (π * r²) - returns float for precision
    inline float getArea() const noexcept {
        return 3.141592653589793f * radius_squared;
    }
    
    // Check if point is on circle edge (within tolerance)
    inline bool isOnEdge(const Vec2& point, float tolerance = 1.0f) const noexcept {
        const float distSquared = distanceSquaredTo(point);
        const float inner_radius = (radius > tolerance) ? radius - tolerance : 0.0f;
        const float outer_radius = radius + tolerance;
        const float inner_radius_sq = inner_radius * inner_radius;
        const float outer_radius_sq = outer_radius * outer_radius;
        
        return distSquared >= inner_radius_sq && distSquared <= outer_radius_sq;
    }
    
    // Get circumference
    inline float getCircumference() const noexcept {
        return 2.0f * 3.141592653589793f * radius;
    }
    
    // Check if circle intersects with a rectangle (useful for collision with game objects)
    inline bool intersects(const BBox& bbox) const noexcept {
        // Find the closest point on the rectangle to the circle center
        const float closest_x = std::max(bbox.x, std::min(center.x, bbox.x + bbox.width));
        const float closest_y = std::max(bbox.y, std::min(center.y, bbox.y + bbox.height));
        
        // Calculate distance from circle center to closest point
        const float dx = center.x - closest_x;
        const float dy = center.y - closest_y;
        const float distanceSquared = dx * dx + dy * dy;
        
        return distanceSquared <= radius_squared;
    }
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
    BCircle bbox;           // Bounding circle instead of box
    bool filled = true;     // 1 byte (default to filled)
    float pitch = 0.0f;     // 4 bytes (rotation around x-axis - nodding up/down)
    float yaw = 0.0f;       // 4 bytes (rotation around y-axis - turning left/right)
    float roll = 0.0f;      // 4 bytes (rotation around z-axis - tilting left/right)
    Vec2 center;            // 4 bytes (center point for rotation)
    bool move = false; // 1 byte (indicates if polygon is moving, used for physics)
    Vec2 velocity; // 8 bytes (velocity vector for physics, if needed)

    // Initial angles for GPU-side rotation calculation (NEW)
    float initial_pitch = 0.0f;  // 4 bytes - base rotation angles
    float initial_yaw = 0.0f;    // 4 bytes
    float initial_roll = 0.0f;   // 4 bytes

    // Cached trigonometric values - computed once, used many times
    mutable float pitch_sin;
    mutable float pitch_cos;
    mutable float yaw_sin;
    mutable float yaw_cos;
    mutable float roll_sin;
    mutable float roll_cos;


    Polygon() = default;
    
    // Constructor with position - BCircle is calculated automatically
    Polygon(const Color<u8>& c, i16 x = 0, i16 y = 0) 
        : color(c), bbox(Vec2(static_cast<float>(x), static_cast<float>(y)), 0.0f), 
          center(static_cast<float>(x), static_cast<float>(y)) {}
    
    // Constructor with points - BCircle is calculated from points
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


    void setVelocity(float vx, float vy) {
        velocity.x = vx;
        velocity.y = vy;
    }

    void setVelocity(const Vec2& v) {
        velocity = v;
    }

    void adjustVelocity(float vx, float vy) {
        velocity.x += vx;
        velocity.y += vy;
    }

    void adjustVelocity(const Vec2& v) {
        velocity += v;
    }

    void movePolygon(float dt) {
        if (move) {
            // Update position based on velocity and time delta
            center.x += velocity.x * dt;
            center.y += velocity.y * dt;
            // Update bounding box and points based on new center
            _apply_velocity_to_points(dt);
        }
    }


    Color<float> getGLColor() const {
        return color.toGL();
    }
    
    // Add point and recalculate BCircle and center
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
                _calculateRotationVariables(pitch, yaw, roll);
                Vec2 rotated_point = _applyRotations(point);
                points_rotated.add(rotated_point);
                break;
            }
            case 1: {
                // Add to rotated as current position
                points_rotated.add(point);
                // Add to original as current position rotated in reverse
                _calculateRotationVariablesReverse(pitch, yaw, roll);
                Vec2 original_point = _applyRotations(point);
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
    
    // Get the current bounding circle
    const BCircle& getBBox() const {
        return bbox;
    }
    
    // Get the center point
    const Vec2& getCenter() const {
        return center;
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
    
    
    // Recalculate BCircle and center from current rotated points
    void updateBBox() {
        _calculateBBox();
    }
    
    // Move polygon so its BCircle center is at specified position
    void moveTo(i16 x, i16 y) {
        _movePoly(x, y);
    }
    
    // Main rotation functions - optimized and consolidated
    // ====================================================
    // 
    // setRotation(pitch, yaw, roll) - Sets ABSOLUTE rotation angles for all axes
    // rotate(pitch, yaw, roll) - ADDS rotation amounts to current rotation
    // 
    // All parameters default to 0.0f, so you can specify only the axes you want.
    // Examples: 
    //   setRotation(0.5f) sets pitch to 0.5 radians, yaw and roll to 0
    //   rotate(0.0f, 0.0f, 0.2f) adds 0.2 radians to current roll
    
    // Set absolute rotation values for all axes (sets specific angles)
    void setRotation(float pitch_rad = 0.0f, float yaw_rad = 0.0f, float roll_rad = 0.0f) {
        pitch = pitch_rad;
        yaw = yaw_rad;
        roll = roll_rad;
        _calculateRotationVariables(pitch, yaw, roll);
        _updateRotatedPoints();
        _calculateBBox(true);
    }
    
    // Add to current rotation values for all axes (rotates by specified amounts)
    void rotate(float pitch_delta = 0.0f, float yaw_delta = 0.0f, float roll_delta = 0.0f) {
        pitch += pitch_delta;
        yaw += yaw_delta;
        roll += roll_delta;
        
        // // Normalize angles to prevent them from growing too large
        pitch = std::fmod(pitch, TWO_PI);
        yaw = std::fmod(yaw, TWO_PI);
        roll = std::fmod(roll, TWO_PI);
        
        _calculateRotationVariables(pitch, yaw, roll);
        // _updateRotatedPoints();
        // _calculateBBox(true);
    }
    
    // Individual axis convenience methods - simplified and consistent
    void setPitch(float angle_radians) { setRotation(angle_radians, yaw, roll); }
    void setYaw(float angle_radians) { setRotation(pitch, angle_radians, roll); }
    void setRoll(float angle_radians) { setRotation(pitch, yaw, angle_radians); }
    
    void rotatePitch(float angle_radians) { rotate(angle_radians, 0.0f, 0.0f); }
    void rotateYaw(float angle_radians) { rotate(0.0f, angle_radians, 0.0f); }
    void rotateRoll(float angle_radians) { rotate(0.0f, 0.0f, angle_radians); }

protected:

    
    // Cached rotation matrix elements (arranged for potential SIMD optimization)
    float _m00, _m01, _m02;
    float _m10, _m11, _m12;
    float _m20, _m21, _m22;
    
    // Cached center coordinates
    float _center_x, _center_y;
    
    // Temporary calculation variables (reused to avoid allocations)
    float _x, _y;
    float _rotated_x, _rotated_y;



    // Move all points so the bounding circle center is at target position
    void _movePoly(i16 target_x, i16 target_y) {
        if (points_original.empty()) {
            return;
        }
        
        float current_x = bbox.center.x;
        float current_y = bbox.center.y;
        float target_x_f = static_cast<float>(target_x);
        float target_y_f = static_cast<float>(target_y);

        // Calculate offset needed to move polygon to target position
        float offset_x = target_x_f - current_x;
        float offset_y = target_y_f - current_y;
        
        // Apply offset to all original points
        for (size_t i = 0; i < points_original.size(); ++i) {
            points_original[i].x += offset_x;
            points_original[i].y += offset_y;

            points_rotated[i].x += offset_x;
            points_rotated[i].y += offset_y;
        }
        
        // Update center and bounding circle center
        center.x += offset_x;
        center.y += offset_y;
        bbox.center.x = target_x_f;
        bbox.center.y = target_y_f;
    }


    void _apply_velocity_to_points(float dt) {
        // Move all points based on velocity and time delta
        float velocity_x = velocity.x * dt;
        float velocity_y = velocity.y * dt;
        for (auto& p : points_rotated) {
            p.x += velocity_x;
            p.y += velocity_y;
        }

    }

    void _calculateBBox(bool rotating = false) {
        if (points_rotated.empty()) {
            bbox = BCircle(center, 0.0f);
            return;
        }
        
        float min_x = std::numeric_limits<float>::max();
        float min_y = std::numeric_limits<float>::max();
        float max_x = std::numeric_limits<float>::lowest();
        float max_y = std::numeric_limits<float>::lowest();
        
        // Find bounds of all rotated points (these are what's actually rendered)
        for (const auto& p : points_rotated) {
            min_x = std::min(min_x, p.x);
            min_y = std::min(min_y, p.y);
            max_x = std::max(max_x, p.x);
            max_y = std::max(max_y, p.y);
        }
        
        // Calculate center of bounding area
        if (!rotating) {
            center.x = min_x + (max_x - min_x) * 0.5f;
            center.y = min_y + (max_y - min_y) * 0.5f;
        }
        
        // Calculate radius as the maximum distance from center to any point
        float max_distance_squared = 0.0f;
        for (const auto& p : points_rotated) {
            float dx = p.x - center.x;
            float dy = p.y - center.y;
            float distance_squared = dx * dx + dy * dy;
            max_distance_squared = std::max(max_distance_squared, distance_squared);
        }
        
        // Create bounding circle
        bbox = BCircle(center, std::sqrt(max_distance_squared));
    }

    void _calculateRotationVariables(float pitch_val, float yaw_val, float roll_val) {
        size_t pitch_index = angle_to_index(pitch_val);
        size_t yaw_index = angle_to_index(yaw_val);
        size_t roll_index = angle_to_index(roll_val);
        
        pitch_sin = trig_table[pitch_index].first;
        pitch_cos = trig_table[pitch_index].second;
        yaw_sin = trig_table[yaw_index].first;
        yaw_cos = trig_table[yaw_index].second;
        roll_sin = trig_table[roll_index].first;
        roll_cos = trig_table[roll_index].second;
    }


    void _calculateRotationVariablesReverse(float pitch_val, float yaw_val, float roll_val) {
        // Reverse multiplier for reverse rotation
        const float multiplier = -1.0f;
        const float effective_pitch = pitch_val * multiplier;
        const float effective_yaw = yaw_val * multiplier;
        const float effective_roll = roll_val * multiplier;

        _calculateRotationVariables(effective_pitch, effective_yaw, effective_roll);
    }

        

        

    // Function 1: Calculate all rotation variables once
    // Sets up trigonometric values and rotation matrix elements
    // void _calculateRotationVariables(float pitch_val, float yaw_val, float roll_val, bool reverse = false) {
    //     // Apply reverse multiplier if needed
    //     const float multiplier = reverse ? -1.0f : 1.0f;
    //     const float effective_pitch = pitch_val * multiplier;
    //     const float effective_yaw = yaw_val * multiplier;
    //     const float effective_roll = roll_val * multiplier;
        
    //     // Pre-calculate all trigonometric values once
    //     _pitch_cos = std::cos(effective_pitch);
    //     _pitch_sin = std::sin(effective_pitch);
    //     _yaw_cos = std::cos(effective_yaw);
    //     _yaw_sin = std::sin(effective_yaw);
    //     _roll_cos = std::cos(effective_roll);
    //     _roll_sin = std::sin(effective_roll);
        
    //     // Pre-calculate rotation matrix elements based on rotation order
    //     if (reverse) {
    //         // Reverse order: Roll -> Yaw -> Pitch (transpose of forward matrix)
    //         _m00 = _yaw_cos * _roll_cos;
    //         _m01 = _pitch_sin * _yaw_sin * _roll_cos + _pitch_cos * _roll_sin;
    //         _m02 = -_pitch_cos * _yaw_sin * _roll_cos + _pitch_sin * _roll_sin;
    //         _m10 = -_yaw_cos * _roll_sin;
    //         _m11 = -_pitch_sin * _yaw_sin * _roll_sin + _pitch_cos * _roll_cos;
    //         _m12 = _pitch_cos * _yaw_sin * _roll_sin + _pitch_sin * _roll_cos;
    //         _m20 = _yaw_sin;
    //         _m21 = -_pitch_sin * _yaw_cos;
    //         _m22 = _pitch_cos * _yaw_cos;
    //     } else {
    //         // Forward order: Pitch -> Yaw -> Roll
    //         _m00 = _yaw_cos * _roll_cos;
    //         _m01 = -_yaw_cos * _roll_sin;
    //         _m02 = _yaw_sin;
    //         _m10 = _pitch_sin * _yaw_sin * _roll_cos + _pitch_cos * _roll_sin;
    //         _m11 = -_pitch_sin * _yaw_sin * _roll_sin + _pitch_cos * _roll_cos;
    //         _m12 = -_pitch_sin * _yaw_cos;
    //         _m20 = -_pitch_cos * _yaw_sin * _roll_cos + _pitch_sin * _roll_sin;
    //         _m21 = _pitch_cos * _yaw_sin * _roll_sin + _pitch_sin * _roll_cos;
    //         _m22 = _pitch_cos * _yaw_cos;
    //     }
        
    //     // Cache center coordinates
    //     _center_x = center.x;
    //     _center_y = center.y;
    // }
    
    // Function 2: Apply pre-calculated rotations to all points
    // Uses the variables set by _calculateRotationVariables
    void _applyRotationsToAllPoints() {
        points_rotated.clear();
        points_rotated.reserve(points_original.size());
        
        // Apply pre-calculated matrix to all points in optimized loop (inlined for max performance)
        for (const auto& original_point : points_original) {
            // Inline the transformation for maximum performance
            points_rotated.add(_applyRotations(original_point));
        }
    }
    
    // Optimized rotation application using pre-calculated matrices
    // Now uses the two-function approach with early exit for no-rotation
    void _updateRotatedPoints() {
        // Early exit optimization: if no rotation, just copy original points
        if (pitch == 0.0f && yaw == 0.0f && roll == 0.0f) {
            points_rotated = points_original;
            return;
        }
        
        _calculateRotationVariables(pitch, yaw, roll);
        _applyRotationsToAllPoints();
    }
    
    
    // Apply rotation transformation to a single point (used in addPoint modes)
    inline Vec2 _applyRotations(const Vec2& point) {
        // Apply transformation to single point (optimized for 2D)
        _x = point.x - _center_x;
        _y = point.y - _center_y;
        
        // Skip z calculations since z=0 for 2D points
        _rotated_x = _m00 * _x + _m01 * _y; // Removed _m02 * _z 
        _rotated_y = _m10 * _x + _m11 * _y; // Removed _m12 * _z
        
        return Vec2(_rotated_x + _center_x, _rotated_y + _center_y);
    }
    

};

// Rectangle class - inherits from Polygon with specific constraints
struct Rectangle : public Polygon {
    float width, height;  // Rectangle dimensions (always positive, float for sub-pixel precision)
    
    // Constructor with position, dimensions, color, and optional rotation
    Rectangle(float x, float y, float w, float h, const Color<u8>& c, float rotation_radians = 0.0f) 
        : Polygon(c), width(w), height(h) {
        _createRectanglePoints(x, y, w, h);
        _calculateBBox();
        
        // Set initial angles for GPU-side rotation calculation
        initial_pitch = rotation_radians;
        initial_yaw = rotation_radians;  
        initial_roll = rotation_radians;
        
        if (rotation_radians != 0.0f) {
            rotate(0.0f, 0.0f, rotation_radians); // Use the new 3-axis rotate function
        }
    }
    
    // Constructor with just dimensions and color (positioned at origin)
    Rectangle(float w, float h, const Color<u8>& c, float rotation_radians = 0.0f) 
        : Rectangle(0.0f, 0.0f, w, h, c, rotation_radians) {}
    
    // Legacy constructor for backward compatibility
    Rectangle(i16 x, i16 y, u16 w, u16 h, const Color<u8>& c, float rotation_radians = 0.0f) 
        : Rectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), c, rotation_radians) {}
    
    // Resize rectangle (maintains position and rotation)
    void resize(float new_width, float new_height) {
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
        _createRectanglePoints(0.0f, 0.0f, width, height);
        _calculateBBox();
        
        // Restore rotations
        if (current_pitch != 0.0f || current_yaw != 0.0f || current_roll != 0.0f) {
            pitch = current_pitch;
            yaw = current_yaw;
            roll = current_roll;
            _updateRotatedPoints();
        }
        
        // Restore position
        moveTo(static_cast<i16>(current_center.x) - static_cast<i16>(width / 2), 
               static_cast<i16>(current_center.y) - static_cast<i16>(height / 2));
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
    void _createRectanglePoints(float x, float y, float w, float h) {
        points_original.clear();
        points_original.reserve(4);
        points_rotated.clear();
        points_rotated.reserve(4);
        
        // Add 4 corners: top-left, top-right, bottom-right, bottom-left (no conversions needed!)
        points_original.add(x, y);                    // Top-left
        points_original.add(x + w, y);               // Top-right  
        points_original.add(x + w, y + h);          // Bottom-right
        points_original.add(x, y + h);               // Bottom-left
        
        // Initially, rotated points are the same as original
        points_rotated = points_original;
    }
};

} // namespace objects

