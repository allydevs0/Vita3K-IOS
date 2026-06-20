// Vita3K emulator project - iOS Entry Point
// Copyright (C) 2026 Vita3K team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include "interface.h"

#include <app/functions.h>
#include <config/functions.h>
#include <config/state.h>
#include <emuenv/state.h>
#include <util/log.h>
#include <util/string_utils.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstring>
#include <string>
#include <thread>

// Global emulator state — lives for the duration of the app
static EmuEnvState emuenv;
static SDL_Window *sdl_window = nullptr;
static bool emulator_running = false;

// C-linkage bridge called from Swift AppDelegate
extern "C" {

/// Called once from AppDelegate after UIKit is ready.
/// @param metal_layer  Pointer to the CAMetalLayer returned by ViewController.
/// @param app_path     Path to the .vpk or installed app to launch (may be NULL
///                     to open the game browser instead).
/// @return 0 on success, non-zero on error.
int vita3k_ios_start(void *metal_layer, const char *app_path) {
    // ----------------------------------------------------------------
    // 1. Logging
    // ----------------------------------------------------------------
    logging::set_level(logging::Level::Info);
    LOG_INFO("VitaiOS starting up…");

    // ----------------------------------------------------------------
    // 2. SDL3 initialisation (audio + events; video is handled by Metal)
    // ----------------------------------------------------------------
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
        LOG_ERROR("SDL_Init failed: {}", SDL_GetError());
        return 1;
    }

    // Create a minimal SDL window that wraps the existing CAMetalLayer.
    // SDL3 on iOS accepts a pre-created Metal layer via the
    // SDL_WINDOW_METAL property so MoltenVK can use the UIKit surface.
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_METAL_VIEW_POINTER, metal_layer);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER,
        SDL_WINDOW_METAL | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    sdl_window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!sdl_window) {
        LOG_ERROR("SDL_CreateWindowWithProperties failed: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // ----------------------------------------------------------------
    // 3. Emulator core initialisation
    // ----------------------------------------------------------------
    // Resolve the iOS Documents directory as the Vita3K root path
    // (read/write, survives app updates, user-accessible via Files app).
    char *base_path = SDL_GetPrefPath("com.allydevs0", "VitaiOS");
    if (!base_path) {
        LOG_ERROR("Could not determine base path");
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return 1;
    }

    const fs::path root_path(base_path);
    SDL_free(base_path);

    // Load / create default config
    Config cfg;
    if (config::init_config(cfg, root_path) != Success) {
        LOG_WARN("Could not load config, using defaults");
    }

    // Initialise the emulator core
    if (!app::init(emuenv, cfg, root_path)) {
        LOG_ERROR("app::init failed");
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return 1;
    }

    emuenv.window = sdl_window;

    // ----------------------------------------------------------------
    // 4. Optional: auto-launch an app if a path was supplied
    // ----------------------------------------------------------------
    if (app_path && std::strlen(app_path) > 0) {
        AppLaunchRequest launch_req;
        launch_req.path = fs::path(app_path);

        int32_t main_module_id = -1;
        if (load_app(main_module_id, emuenv, launch_req) != Success) {
            LOG_ERROR("load_app failed for path: {}", app_path);
        } else if (run_app(emuenv, main_module_id, launch_req) != Success) {
            LOG_ERROR("run_app failed");
        }
    }

    emulator_running = true;
    LOG_INFO("VitaiOS initialisation complete.");
    return 0;
}

/// Pump SDL events — call this from the UIKit display link / CADisplayLink.
void vita3k_ios_pump_events(void) {
    if (!emulator_running)
        return;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            emulator_running = false;
        }
        // Additional input handling (touch → Vita ctrl, etc.) lives in
        // the existing vita3k/input subsystem, which SDL3 feeds automatically.
    }
}

/// Clean shutdown — call from applicationWillTerminate.
void vita3k_ios_shutdown(void) {
    emulator_running = false;
    if (sdl_window) {
        SDL_DestroyWindow(sdl_window);
        sdl_window = nullptr;
    }
    SDL_Quit();
    LOG_INFO("VitaiOS shut down.");
}

} // extern "C"
