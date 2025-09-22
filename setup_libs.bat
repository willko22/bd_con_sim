@echo off
echo Setting up EnTT, GLFW, ImGui, and FreeType libraries...

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

REM Setup GLAD (OpenGL loader)
echo Setting up GLAD...
echo Creating GLAD directory structure...
if exist "libs\glad" rmdir /s /q "libs\glad"
mkdir "libs\glad"
mkdir "libs\glad\include"
mkdir "libs\glad\include\glad"
mkdir "libs\glad\include\KHR"
mkdir "libs\glad\src"

echo.
echo GLAD setup requires manual download due to web service complexity.
echo.
echo Please follow these steps:
echo 1. Open your web browser
echo 2. Go to: https://glad.dav1d.de/#language=c^&specification=gl^&api=gl%%3D4.6^&api=gles1%%3Dnone^&api=gles2%%3Dnone^&api=glsc2%%3Dnone^&profile=core^&loader=on
echo 3. Click the 'Generate' button
echo 4. Download the 'glad.zip' file
echo 5. Extract the contents:
echo    - Copy include/glad/glad.h to libs\glad\include\glad\
echo    - Copy include/KHR/khrplatform.h to libs\glad\include\KHR\
echo    - Copy src/glad.c to libs\glad\src\
echo.
echo Directory structure created at: libs\glad\
echo After manual setup, you should have:
echo   libs\glad\include\glad\glad.h
echo   libs\glad\include\KHR\khrplatform.h
echo   libs\glad\src\glad.c
echo.

REM Download FreeType (replacement for stb_truetype)
echo Setting up FreeType (font rendering)...
echo Cloning FreeType repository...
if exist "libs\freetype" rmdir /s /q "libs\freetype"
git clone --depth 1 https://github.com/freetype/freetype.git libs/freetype
if %ERRORLEVEL% equ 0 (
    echo FreeType downloaded successfully
) else (
    echo Failed to clone FreeType. You can manually download it from: https://github.com/freetype/freetype
    echo Extract it to: libs\freetype\
)

echo.
echo Library setup complete!
echo You can now build the project with C++23, EnTT, GLFW, ImGui, GLAD, FreeType, and OpenGL support.

REM Check if libraries are properly set up
if exist "libs\entt\src\entt\entt.hpp" if exist "libs\glfw\CMakeLists.txt" if exist "libs\imgui\imgui.h" if exist "libs\glad\src\glad.c" if exist "libs\freetype\include\freetype\freetype.h" (
    echo All libraries are properly configured
) else (
    echo Some libraries may not be properly configured. Check the libs\ directory.
)

pause
