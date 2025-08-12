@echo off
echo Setting up EnTT and GLFW libraries...

REM Create libs directory if it doesn't exist
if not exist "libs" (
    mkdir "libs"
    echo Created libs directory
)

REM Download EnTT
echo Setting up EnTT...
echo Cloning EnTT repository...
if exist "libs\entt" rmdir /s /q "libs\entt"
git clone --depth 1 https://github.com/skypjack/entt.git libs/entt
if %ERRORLEVEL% equ 0 (
    echo EnTT downloaded successfully
) else (
    echo Failed to clone EnTT. You can manually download it from: https://github.com/skypjack/entt
    echo Extract it to: libs\entt\
)

REM Download GLFW  
echo Setting up GLFW...
echo Cloning GLFW repository...
if exist "libs\glfw" rmdir /s /q "libs\glfw"
git clone --depth 1 https://github.com/glfw/glfw.git libs/glfw
if %ERRORLEVEL% equ 0 (
    echo GLFW downloaded successfully
) else (
    echo Failed to clone GLFW. You can manually download it from: https://github.com/glfw/glfw
    echo Extract it to: libs\glfw\
)

echo.
echo Library setup complete!
echo You can now build the project with C++23, EnTT, GLFW, and OpenGL support.

REM Check if libraries are properly set up
if exist "libs\entt\src\entt\entt.hpp" if exist "libs\glfw\CMakeLists.txt" (
    echo All libraries are properly configured
) else (
    echo Some libraries may not be properly configured. Check the libs\ directory.
)

pause
)

pause
