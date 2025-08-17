# Coding Standards - BD Construction Simulator

## C++ Version
- **C++23** - Use latest features and standards

## Naming Conventions

### Constants
```cpp
const int MAX_UNITS = 100;
const float PI_VALUE = 3.14159f;
```

### Classes
```cpp
class GameEngine {};
class RenderSystem {};
class EntityManager {};
```

### Functions
```cpp
void updateGame();
bool checkCollision();
int calculateDamage();
```

### Variables
```cpp
int player_health;
float movement_speed;
bool is_active;
```

### Private Members
```cpp
class Player {
private:
    int _health;
    float _position_x;
    bool _is_alive;
};
```

## Comment Styles

### Major Section Headers
```cpp
########## GAME SYSTEMS ##########
```

### Section Headers
```cpp
========== Entity Management ==========
```

### Subsection Headers
```cpp
---------- collision detection ----------
```

### Regular Comments
```cpp
// Calculate new position based on velocity
// Check if entity is within bounds
```

## File Organization
- Headers: `include/` directory with subdirectories by category
- Sources: `src/` directory mirroring include structure
- Third-party: `libs/` directory
- Assets: `assets/` directory (textures, sounds, maps, etc.)

## Best Practices
- Use RAII for resource management
- Prefer `enum class` over plain enums
- Use `auto` for type deduction when type is obvious
- Prefer range-based for loops
- Use smart pointers for dynamic memory management
