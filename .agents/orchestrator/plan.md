# Refactoring Plan: VitaiOS

We are following the step-by-step refactoring architecture defined in `implementation_plan.md`.

## Execution Topology
We will run as the Project Orchestrator and delegate tasks to subagents.

## Milestones

### Milestone 1: Delete non-iOS files and directories
- **Files/dirs to delete**:
  - `vita3k/main.cpp`
  - `vita3k/gui-qt/`
  - `vita3k/android/`
  - `vita3k/Vita3K.rc`, `vita3k/resource.h`, `vita3k/Windows.manifest`
  - `cmake/qt6.cmake`
  - `appimage/`
  - `.ci/`
  - `.github/`
  - `i18n/qt/`
- **Verification**: Run `dir` or file search checks to confirm deletion.

### Milestone 2: Update vcpkg config & root CMakeLists.txt
- **File**: `vcpkg.json`
  - Remove any desktop-only dependencies.
- **File**: `CMakeLists.txt` (root)
  - Remove ANDROID block, Discord, AppImage, MSVC/Windows blocks.
  - Simplify and config iOS as default target (enable OBJC/Swift if needed).
- **Verification**: Verify syntax and cmake configure target definitions.

### Milestone 3: Update external/CMakeLists.txt
- **File**: `external/CMakeLists.txt`
  - Remove `discord-rpc`, `libadrenotools`, `winsock`, `MoltenVK-macos.tar`, Vulkan validation layers download, `nativefiledialog-extended`, `googletest`, etc.
  - Keep `capstone`, `dynarmic`, etc.

### Milestone 4: Update vita3k/CMakeLists.txt
- **File**: `vita3k/CMakeLists.txt`
  - Remove ANDROID block, desktop block, WIN32 block, macOS desktop block, Linux block.
  - Remove guards for `VITA3K_BUILD_IOS`.
  - Remove `qt6.cmake`, `gui-qt` subdirectory/link, Discord, Tracy.
  - Target `vita3k` should compile `main_ios.cpp` and `interface.cpp`.

### Milestone 5: Create main_ios.cpp and integration
- **File**: `vita3k/main_ios.cpp`
  - Create entry point using SDL3 that initializes emu core via `app::init()`, calls `load_app()`, `run_app()` from `interface.h`.
- **File**: `vita3k/ios/ViewController.swift` / `AppDelegate.swift`
  - Ensure compatibility with SDL windowing / app lifecycle.

### Milestone 6: Push to `ios-port` and run build
- **Actions**:
  - Push changes to `ios-port` branch.
  - Trigger Codemagic build (or instruct user/task).
  - Verify build outputs valid `.app` bundle.
