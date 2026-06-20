# Deletion Plan Report: Removing Non-iOS Assets

This report analyzes the Vita3K repository and outlines a comprehensive plan for removing non-iOS platform-specific files, build configurations, and assets in preparation for targeting iOS exclusively (Milestone 1).

---

## 1. Verification of Specified Deletion Targets

All 9 target paths specified in the request have been verified to exist in the repository:

| Target Path | Type | Existence | Description / Purpose |
|---|---|---|---|
| `vita3k/main.cpp` | File | Verified | Main entry point for desktop platforms (uses Qt6). |
| `vita3k/gui-qt/` | Directory | Verified | Contains the Qt6-based desktop user interface code. |
| `vita3k/android/` | Directory | Verified | Contains Android-specific JNI C++ code wrappers and JNI binders. |
| `vita3k/Vita3K.rc` | File | Verified | Windows executable resource compiler script. |
| `vita3k/resource.h` | File | Verified | Windows resource header containing resource IDs. |
| `vita3k/Windows.manifest` | File | Verified | Windows application manifest for visual styles and privileges. |
| `cmake/qt6.cmake` | File | Verified | CMake script for locating and configuring Qt6 library. |
| `appimage/` | Directory | Verified | Configuration scripts for packaging Linux AppImages. |
| `.ci/` | Directory | Verified | Old continuous integration shell and PowerShell scripts. |
| `.github/` | Directory | Verified | GitHub actions workflows, issue templates, and metadata. |
| `i18n/qt/` | Directory | Verified | Qt-specific translation source files (`.ts`). |

---

## 2. Additional Platforms, Build Files, and Assets Identified for Cleanup

In addition to the requested files, the following platform-specific or desktop-only components have been identified for removal to ensure a clean codebase:

### Root and CMake Files
- **`android/` (Directory at root)**: The root Gradle-based Android build configuration, prebuilts, wrapper scripts (`gradlew`, `gradlew.bat`), and Gradle configuration. Since iOS is targeted exclusively, this root directory is completely obsolete.
- **`cmake/vcpkg_android.cmake` (File)**: Android-specific vcpkg configuration and toolchain setup.
- **`gen-linux.sh` (File)**: Bash helper script to run CMake with the `linux-ninja-clang` preset.
- **`gen-windows.bat` (File)**: Windows batch helper script to generate Visual Studio project files.

### Desktop UI Assets inside `vita3k/`
- **`vita3k/resources.qrc` (File)**: Qt Resource Collection configuration file. Refers entirely to icons, SVG layouts, and QSS style files used by the Qt GUI.
- **`vita3k/Vita3K.ico` (File)**: Windows desktop application icon asset.
- **`vita3k/Vita3K.png` (File)**: Desktop icon graphic used to generate macOS `.icns` or shown on Linux desktops.

### Third-Party Libraries in `external/`
- **`external/libadrenotools/` (Directory)**: Android-only library used for sideloading/hooking Adreno GPU drivers.
- **`external/nativefiledialog-extended/` (Directory)**: Desktop-only library providing OS-native file dialogs.
- **`external/googletest/` (Directory)**: Testing framework not needed for the iOS target compilation.
- **`external/tracy/` (Directory)**: Tracy Profiler library source (removed for iOS port target).
- **`external/dirent/` (Directory)**: Compatibility header for MSVC compiler on Windows. Since iOS uses Clang/POSIX where `<dirent.h>` is native, this is obsolete.

---

## 3. Dependency Verification for `vita3k/icons/`

The folder `vita3k/icons/` contains UI and emulator assets. A dependency scan shows that we must **NOT** delete the folder entirely, but selectively clean up:

1. **Required Core Assets (DO NOT DELETE)**:
   - `bronze.png`, `silver.png`, `gold.png`, `platinum.png`: The emulation core (`vita3k/app/src/app_init.cpp:650`) dynamically loads these files at runtime from the `icons` subfolder under `static_assets_path` for trophy unlock notifications.
   - `cross.png`, `doublearrow.png`, `info.png`: The in-game SDL3 overlay interface (`vita3k/overlay/src/common_dialog.cpp:134-136`) loads these to render dialog close buttons, back buttons, and info prompts.
2. **Obsolete Assets (SAFE TO DELETE)**:
   - All SVG files (`PSV_Layout.svg`, `check_dark.svg`, `check_light.svg`, `chevron_down_dark.svg`, `chevron_down_light.svg`, `chevron_up_dark.svg`, `chevron_up_light.svg`, `cross_dark.svg`, `cross_light.svg`, `radio_dot_dark.svg`, `radio_dot_light.svg`).
   - Desktop-specific PNG icons (`configure.png`, `controllers.png`, `cross.png` *[Wait, keep cross.png for overlay]*, `exit_fullscreen.png`, `fullscreen.png`, `open.png`, `pause.png`, `play.png`, `refresh.png`, `stop.png`).

---

## 4. Sequential Deletion Plan

To ensure clean execution, the implementer should perform deletions in the following sequence:

1. **Root Platforms & Workflows Cleanup**:
   - Delete `.github/`
   - Delete `.ci/`
   - Delete `android/`
   - Delete `appimage/`
2. **Platform Source & Translations Cleanup**:
   - Delete `vita3k/android/`
   - Delete `vita3k/gui-qt/`
   - Delete `i18n/qt/`
3. **Build System & Generation Scripts Cleanup**:
   - Delete `cmake/qt6.cmake`
   - Delete `cmake/vcpkg_android.cmake`
   - Delete `gen-linux.sh`
   - Delete `gen-windows.bat`
4. **Desktop Windows Resource Cleanup**:
   - Delete `vita3k/main.cpp`
   - Delete `vita3k/Vita3K.rc`
   - Delete `vita3k/resource.h`
   - Delete `vita3k/Windows.manifest`
   - Delete `vita3k/resources.qrc`
   - Delete `vita3k/Vita3K.ico`
   - Delete `vita3k/Vita3K.png`
5. **Obsolete Submodule/External Directory Cleanup**:
   - Delete `external/libadrenotools/`
   - Delete `external/nativefiledialog-extended/`
   - Delete `external/googletest/`
   - Delete `external/tracy/`
   - Delete `external/dirent/`
6. **Selective Icon Cleanup**:
   - Inside `vita3k/icons/`, delete all files **except**:
     - `bronze.png`
     - `silver.png`
     - `gold.png`
     - `platinum.png`
     - `cross.png`
     - `doublearrow.png`
     - `info.png`
