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
if exist "libs\glad\src\glad.c" (
    echo GLAD already present at libs\glad
) else (
    echo GLAD not found; attempting to generate via Python glad2...
    where python >nul 2>nul
    if %ERRORLEVEL% neq 0 (
        echo Python not found. Skipping GLAD generation.
        echo Please generate GLAD manually (C, OpenGL core 3.3+, include gl loader) and place it in libs\glad
        echo Web generator: https://glad.dav1d.de/
    ) else (
        python -m pip install --user --quiet glad2
        if %ERRORLEVEL% equ 0 (
            python -m glad --api gl:core=3.3 --generator c --out-path libs/glad --extensions ''
            if %ERRORLEVEL% equ 0 (
                echo GLAD generated successfully into libs\glad
            ) else (
                echo Failed to generate GLAD with glad2. You can try the web generator and copy files to libs\glad
            )
        ) else (
            echo Failed to install glad2 via pip. Please install it manually or use the web generator.
        )
    )
)

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
