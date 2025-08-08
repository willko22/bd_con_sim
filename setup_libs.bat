@echo off
echo Setting up EnTT and Sokol libraries...

REM Create libs directory if it doesn't exist
if not exist "libs" (
    mkdir "libs"
    echo Created libs directory
)

REM Download EnTT
echo Setting up EnTT...
echo Cloning EnTT repository...
git clone --depth 1 --force https://github.com/skypjack/entt.git libs/entt
if %ERRORLEVEL% equ 0 (
    echo EnTT downloaded successfully
) else (
    echo Failed to clone EnTT. You can manually download it from: https://github.com/skypjack/entt
    echo Extract it to: libs\entt\
)

REM Download Sokol  
echo Setting up Sokol...
echo Cloning Sokol repository...
git clone --depth 1 --force https://github.com/floooh/sokol.git libs/sokol
if %ERRORLEVEL% equ 0 (
    echo Sokol downloaded successfully
) else (
    echo Failed to clone Sokol. You can manually download it from: https://github.com/floooh/sokol
    echo Extract it to: libs\sokol\
)

echo.
echo Library setup complete!
echo You can now build the project with C++23, EnTT, and Sokol support.

REM Check if libraries are properly set up
if exist "libs\entt\src\entt\entt.hpp" if exist "libs\sokol" (
    echo All libraries are properly configured
) else (
    echo Some libraries may not be properly configured. Check the libs\ directory.
)

pause
REM Check if libraries are properly set up
if exist "libs\entt\src\entt\entt.hpp" if exist "libs\sokol" (
    echo All libraries are properly configured
) else (
    echo Some libraries may not be properly configured. Check the libs\ directory.
)

pause
