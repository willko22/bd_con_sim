cmake_minimum_required(VERSION 3.16)

# Get the project name from the folder name
get_filename_component(PROJECT_NAME_FROM_FOLDER ${CMAKE_SOURCE_DIR} NAME)
# Project name and version
project(${PROJECT_NAME_FROM_FOLDER} VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set C standard (for GLAD)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Generate compile_commands.json for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Configure different build types
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wno-unknown-pragmas -Wno-cast-function-type")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -s")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/libs)

# Find packages
find_package(PkgConfig QUIET)

# EnTT - Header-only ECS library
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/entt")
    message(STATUS "EnTT not found in libs/entt. Please add EnTT to libs/entt or install via package manager")
else()
    include_directories(${CMAKE_SOURCE_DIR}/libs/entt/src)
    message(STATUS "EnTT found at: ${CMAKE_SOURCE_DIR}/libs/entt")
endif()

# GLFW - Window and input handling
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/glfw")
    message(STATUS "GLFW not found in libs/glfw. Please add GLFW to libs/glfw or install via package manager")
else()
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/glfw)
    message(STATUS "GLFW found at: ${CMAKE_SOURCE_DIR}/libs/glfw")
endif()

# ImGui - Immediate mode GUI library
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/imgui")
    message(STATUS "ImGui not found in libs/imgui. Please add ImGui to libs/imgui or install via package manager")
else()
    include_directories(${CMAKE_SOURCE_DIR}/libs/imgui)
    include_directories(${CMAKE_SOURCE_DIR}/libs/imgui/backends)
    
    # ImGui source files
    set(IMGUI_SOURCES
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp
    )
    message(STATUS "ImGui found at: ${CMAKE_SOURCE_DIR}/libs/imgui")
endif()

# FreeType - High-quality font rendering library (replacement for stb_truetype)
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/freetype")
    message(STATUS "FreeType not found in libs/freetype. Please add FreeType to libs/freetype or run setup_libs.bat")
else()
    # Build FreeType from source in libs/freetype
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/freetype)
    # Include directory containing ft2build.h and freetype headers
    include_directories(${CMAKE_SOURCE_DIR}/libs/freetype/include)
    message(STATUS "FreeType found at: ${CMAKE_SOURCE_DIR}/libs/freetype")
endif()


# GLAD - OpenGL function loader
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/glad")
    message(STATUS "GLAD not found in libs/glad. Please add GLAD to libs/glad")
else()
    # Build GLAD as a static library
    add_library(glad STATIC ${CMAKE_SOURCE_DIR}/libs/glad/src/glad.c)
    target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/libs/glad/include)
    message(STATUS "GLAD found at: ${CMAKE_SOURCE_DIR}/libs/glad")
endif()


# OpenGL - Graphics API
find_package(OpenGL REQUIRED)

# Platform-specific libraries for GLFW + OpenGL
if(WIN32)
    set(GRAPHICS_LIBS 
        opengl32
        ${OPENGL_LIBRARIES}
    )
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    set(GRAPHICS_LIBS 
        ${OPENGL_LIBRARIES}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
elseif(UNIX)
    find_package(X11 REQUIRED)
    set(GRAPHICS_LIBS 
        ${X11_LIBRARIES}
        ${OPENGL_LIBRARIES}
        m
        dl
        pthread
    )
endif()

# Find all source files
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/**/*.cpp"
)

# Add ImGui sources if available
if(EXISTS "${CMAKE_SOURCE_DIR}/libs/imgui")
    list(APPEND SOURCES ${IMGUI_SOURCES})
endif()

# Find all header files (for IDE support)
file(GLOB_RECURSE HEADERS
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_SOURCE_DIR}/include/**/*.hpp"
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/include/**/*.h"
)


# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${CMAKE_PROJECT_NAME}"
    DEBUG_POSTFIX "_debug"

)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -s>
    )
    # Add size optimization flags for release
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-s -Wl,--gc-sections>
    )
    
    # Static linking for self-contained executable
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-static-libgcc -static-libstdc++ -static>)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2>
    )
endif()

# GLFW compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    # Prevent GLFW from including OpenGL headers (we use GLAD)
    GLFW_INCLUDE_NONE
)

# ImGui compile definitions
# ImGui will use its built-in OpenGL loader by default unless we specify otherwise
if(EXISTS "${CMAKE_SOURCE_DIR}/libs/imgui" AND EXISTS "${CMAKE_SOURCE_DIR}/libs/glad")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
endif()

# Link libraries
if(EXISTS "${CMAKE_SOURCE_DIR}/libs/glfw")
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${GRAPHICS_LIBS})
    message(STATUS "Linking GLFW and OpenGL libraries: glfw ${GRAPHICS_LIBS}")
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GRAPHICS_LIBS})
    message(STATUS "GLFW not found - linking only OpenGL libraries: ${GRAPHICS_LIBS}")
endif()

# Link GLAD if available
if(EXISTS "${CMAKE_SOURCE_DIR}/libs/glad")
    target_link_libraries(${PROJECT_NAME} PRIVATE glad)
    message(STATUS "Linking GLAD library")
endif()

# Link FreeType if available
if(EXISTS "${CMAKE_SOURCE_DIR}/libs/freetype")
    # The FreeType CMake build defines a target named 'freetype'
    target_link_libraries(${PROJECT_NAME} PRIVATE freetype)
    message(STATUS "Linking FreeType library")
endif()

# Additional libraries can be added here as needed
# target_link_libraries(${PROJECT_NAME} PRIVATE additional_library)

# Custom target to copy assets
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets to build directory"
)

# Make sure assets are copied before building the executable
add_dependencies(${PROJECT_NAME} copy_assets)

# Print configuration info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source files found: ${SOURCES}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
