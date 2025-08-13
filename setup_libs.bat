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

REM Download ImGui
echo Setting up ImGui...
echo Cloning ImGui repository...
if exist "libs\imgui" rmdir /s /q "libs\imgui"
git clone --depth 1 https://github.com/ocornut/imgui.git libs/imgui
if %ERRORLEVEL% equ 0 (
    echo ImGui downloaded successfully
) else (
    echo Failed to clone ImGui. You can manually download it from: https://github.com/ocornut/imgui
    echo Extract it to: libs\imgui\
)

REM Download stb (includes stb_truetype)
echo Setting up stb (includes stb_truetype)...
echo Cloning stb repository...
if exist "libs\stb" rmdir /s /q "libs\stb"
git clone --depth 1 https://github.com/nothings/stb.git libs/stb
if %ERRORLEVEL% equ 0 (
    echo stb downloaded successfully
) else (
    echo Failed to clone stb. You can manually download it from: https://github.com/nothings/stb
    echo Extract it to: libs\stb\
)

echo.
echo Library setup complete!
echo You can now build the project with C++23, EnTT, GLFW, ImGui, stb_truetype, and OpenGL support.

REM Check if libraries are properly set up
if exist "libs\entt\src\entt\entt.hpp" if exist "libs\glfw\CMakeLists.txt" if exist "libs\imgui\imgui.h" if exist "libs\stb\stb_truetype.h" (
    echo All libraries are properly configured
) else (
    echo Some libraries may not be properly configured. Check the libs\ directory.
)

pause
)

pause
