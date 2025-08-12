// main.cpp
#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "game/Game.hpp"
#include <iostream>

// Global game instance
static Game* g_game = nullptr;

static void init() {
    std::cout << "Main init() called" << std::endl;
    g_game = new Game();
    g_game->initialize();
    std::cout << "Main init() complete" << std::endl;
}

static void frame() {
    static int main_frame_count = 0;
    main_frame_count++;
    if (main_frame_count % 60 == 0) {
        std::cout << "Main frame() " << main_frame_count << std::endl;
    }
    
    if (g_game) {
        g_game->update();
        g_game->render();
    }
}

static void cleanup() {
    std::cout << "Main cleanup() called - game is exiting" << std::endl;
    if (g_game) {
        g_game->cleanup();
        delete g_game;
        g_game = nullptr;
    }
    std::cout << "Main cleanup() complete" << std::endl;
}

static void event(const sapp_event* e) {
    std::cout << "Event received: type=" << e->type << std::endl;
    if (e->type == SAPP_EVENTTYPE_QUIT_REQUESTED) {
        std::cout << "Quit requested!" << std::endl;
    }
    if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
        std::cout << "Key pressed: " << e->key_code << std::endl;
        if (e->key_code == SAPP_KEYCODE_ESCAPE) {
            std::cout << "Escape key pressed - requesting quit" << std::endl;
            sapp_request_quit();
        }
    }
    // Handle events here if needed
}

sapp_desc sokol_main(int /*argc*/, char* /*argv*/[]) {
    std::cout << "sokol_main() called" << std::endl;
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.event_cb = event;
    desc.width = 800;
    desc.height = 600;
    desc.window_title = "Rectangle Spawner Game";
    desc.high_dpi = false;
    desc.sample_count = 1;
    std::cout << "sokol_main() returning descriptor" << std::endl;
    return desc;
}
