// VitaiOS-Bridging-Header.h
// Exposes the C++ entry points from main_ios.cpp to Swift code.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/// Start the Vita3K emulator engine.
/// @param metal_layer   Pointer to the CAMetalLayer from ViewController.
/// @param app_path      Path to game/app to launch, or NULL for game browser.
/// @return 0 on success, non-zero on failure.
int vita3k_ios_start(void *metal_layer, const char *app_path);

/// Pump SDL events. Call once per display frame from a CADisplayLink.
void vita3k_ios_pump_events(void);

/// Cleanly shut down the emulator. Call from applicationWillTerminate.
void vita3k_ios_shutdown(void);

#ifdef __cplusplus
}
#endif
