# Deletion Plan Report: Removing Non-iOS Assets

This report outlines the plan for removing non-iOS platforms, build configurations, CI workflows, and desktop-specific GUIs from the Vita3K repository in preparation for targeting iOS exclusively.

---

## 1. Verification of Specified Deletion Targets

The following 9 paths requested for deletion have been verified to exist in the repository.

| Target Path | Type | Details / Size | Purpose | Action |
|---|---|---|---|---|
| `vita3k/main.cpp` | File | `10008 bytes` | Main entry point for desktop environments. | **Delete** |
| `vita3k/gui-qt/` | Directory | `4 subdirs / files` | Qt6-based desktop graphical user interface. | **Delete** |
| `vita3k/android/` | Directory | `JNI source files` | Android JNI native code wrapper & binders. | **Delete** |
| `vita3k/Vita3K.rc` | File | `3866 bytes` | Windows executable resource script. | **Delete** |
| `vita3k/resource.h` | File | `1357 bytes` | Windows resource header. | **Delete** |
| `vita3k/Windows.manifest` | File | `1374 bytes` | Windows compatibility and visual styles manifest. | **Delete** |
| `cmake/qt6.cmake` | File | `2507 bytes` | Qt6 package finder and build helper script. | **Delete** |
| `appimage/` | Directory | `3 files` | AppImage packaging configurations for Linux desktop. | **Delete** |
| `.ci/` | Directory | `9 files` | CI scripts for Android, Desktop, and Windows builds. | **Delete** |
| `.github/` | Directory | `2 subdirs / 4 files` | GitHub actions workflows, issues templates, and metadata. | **Delete** |
| `i18n/qt/` | Directory | `1 file (167113 bytes)` | Qt-specific translation source files. | **Delete** |

---

## 2. Additional Platforms & Desktop Assets Identified for Cleanup

During the exploration, the following additional non-iOS platform configurations and assets were identified for deletion:

| Target Path | Type | Details / Size | Reason for Deletion | Action |
|---|---|---|---|---|
| `android/` | Directory | Root folder | Root Android project directory (Gradle files, wrapper, prebuilts). Completely non-iOS. | **Delete** |
| `cmake/vcpkg_android.cmake` | File | `3853 bytes` | Android-specific vcpkg configuration script. | **Delete** |
| `gen-linux.sh` | File | `97 bytes` | Helper script for generating Linux desktop builds. | **Delete** |
| `gen-windows.bat` | File | `156 bytes` | Helper script for generating Windows desktop builds. | **Delete** |
| `vita3k/resources.qrc` | File | `1608 bytes` | Qt application resource definition. Only referenced by `gui-qt`. | **Delete** |
| `vita3k/Vita3K.ico` | File | `370070 bytes` | Windows executable icon asset. | **Delete** |
| `vita3k/Vita3K.png` | File | `124295 bytes` | Desktop application icon. | **Delete** |

### ⚠️ Special Dependency Note: `vita3k/icons/`
The folder `vita3k/icons/` contains various SVG/PNG icons. While most of these icons (`configure.png`, `controllers.png`, etc.) are only referenced in `resources.qrc` for the Qt GUI and are safe to delete, **the trophy icons must be preserved**.
- **Trophy Icons**: `bronze.png`, `silver.png`, `gold.png`, and `platinum.png`.
- **Reason**: The core emulator (`vita3k/app/src/app_init.cpp`) dynamically loads these trophy icons from `<static_assets>/icons/` for trophy notifications, which are shared across all platforms including the new SDL3 iOS target.
- **Plan**: Do NOT delete the `vita3k/icons/` folder completely. Instead, delete only the Qt-specific UI icons (e.g., `configure.png`, `controllers.png`, `fullscreen.png`, etc.) and preserve `bronze.png`, `silver.png`, `gold.png`, and `platinum.png`.

---

## 3. Order and Execution of Deletion Plan

Since the task is read-only for this phase, the implementer agent should follow this sequence to perform the deletions safely:

1. **Delete Root Platform-Specific Directories**:
   - `android/`
   - `appimage/`
   - `.ci/`
   - `.github/`
2. **Delete Source-Level Platform Directories**:
   - `vita3k/gui-qt/`
   - `vita3k/android/`
   - `i18n/qt/`
3. **Delete Root Platform Scripts**:
   - `gen-linux.sh`
   - `gen-windows.bat`
4. **Delete Build System helper files**:
   - `cmake/qt6.cmake`
   - `cmake/vcpkg_android.cmake`
5. **Delete Desktop UI files & assets in `vita3k/`**:
   - `vita3k/main.cpp`
   - `vita3k/Vita3K.rc`
   - `vita3k/resource.h`
   - `vita3k/Windows.manifest`
   - `vita3k/resources.qrc`
   - `vita3k/Vita3K.ico`
   - `vita3k/Vita3K.png`
6. **Clean Qt-specific UI icons**:
   - Delete all files inside `vita3k/icons/` *except* `bronze.png`, `silver.png`, `gold.png`, `platinum.png`.
