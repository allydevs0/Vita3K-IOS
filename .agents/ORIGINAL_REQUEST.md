# Original User Request

## Initial Request — 2026-06-20T00:40:25-03:00

Refactor the Vita3K repository into an iOS-exclusive build called "VitaiOS". Remove all desktop (Windows, Linux, macOS) and Android code, strip out the Qt6 GUI in favor of a native SDL3 iOS entry point, and simplify the CMake build system to only target iOS, preserving Dynarmic JIT with an optional Unicorn fallback.

Working directory: C:\Users\denise\Downloads\Vita3K
Integrity mode: development

**Reference Material**: Please strictly follow the step-by-step refactoring architecture defined in `C:/Users/denise/.gemini/antigravity-cli/brain/5d5161ba-ab19-484b-9c1c-e1fe858d3681/implementation_plan.md`.

## Requirements

### R1. Remove Non-iOS Platforms
Strip out all OS-specific code, CMake conditional blocks, and GitHub Actions workflows for Windows, Linux, Android, and macOS desktop. 

### R2. Replace GUI
Remove the existing Qt6 frontend (`gui-qt`) and `main.cpp`. Create a new `main_ios.cpp` that initializes the emulator core via SDL3 (which has native iOS support) and interacts directly with `interface.h`.

### R3. Update Build System
Clean up `CMakeLists.txt` and `external/CMakeLists.txt` to remove incompatible desktop dependencies (like Discord RPC, MoltenVK-macos, Qt6, etc.) and configure MoltenVK for iOS. 

## Acceptance Criteria

### iOS Build Success
- [ ] You must push your code to the `ios-port` branch.
- [ ] After pushing, you must trigger a build on Codemagic using their API (or ask the user to trigger it) and verify that the build succeeds and outputs a valid `.app` bundle.

### Clean Codebase (Objective Verification)
- [ ] A programmatic search (e.g., via `grep`) across all `CMakeLists.txt` files MUST return 0 results for `if(WIN32)`, `if(ANDROID)`, and `if(APPLE)`.
- [ ] The `vita3k/gui-qt/` directory must no longer exist.
- [ ] The `vita3k/android/` directory must no longer exist.
