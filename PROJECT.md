# Project: VitaiOS

## Architecture
- Refactoring of Vita3K to target iOS exclusively.
- GUI is completely stripped out and replaced with a native SDL3 entry point `main_ios.cpp`.
- Core emulator integration runs via `interface.h` (`load_app` and `run_app`).
- Build system simplified to target iOS using CMake and vcpkg.

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|---|---|---|---|
| 1 | Delete non-iOS files | Delete desktop UI, Android entry point, Windows assets, old workflows | None | DONE |
| 2 | Root build cleanup | Update vcpkg.json, simplify root CMakeLists.txt for iOS target | M1 | DONE |
| 3 | External deps cleanup | Update external/CMakeLists.txt to remove desktop-only/Android-only libraries | M2 | DONE |
| 4 | Executable target simplification | Update vita3k/CMakeLists.txt to build iOS target only (no Qt6, no desktop) | M3 | DONE |
| 5 | iOS Entry Point | Implement main_ios.cpp using SDL3 and connect to emulator core | M4 | DONE |
| 6 | CI Build & Verification | Push to `ios-port` and run Codemagic iOS build, verifying .app creation | M5 | PLANNED |

## Interface Contracts
### main_ios.cpp ↔ Emulation Core (interface.h)
- `app::init()` initializes emulator core application/settings.
- `load_app(path)` loads a PS Vita app/game path.
- `run_app()` starts the emulator execution loop.

## Code Layout
- `vita3k/main_ios.cpp` - iOS SDL3 entry point.
- `vita3k/ios/` - iOS platform bundle assets/classes (AppDelegate, ViewController).
- `CMakeLists.txt` - root CMake configuration.
- `vita3k/CMakeLists.txt` - executable target and iOS bundle settings.
- `external/CMakeLists.txt` - third-party dependencies configuration.
