# Deletion Plan Report: Platform Cleanup for VitaiOS

This report outlines the plan for deleting all non-iOS directories and files from the Vita3K repository, as part of the refactoring into iOS-exclusive VitaiOS.

## 1. Summary of Identified Target Deletions

The following directories and files are identified for complete removal to clean up desktop (Windows, Linux, macOS desktop) and Android code.

### Directories for Deletion

| Directory Path | Target Platform | Description / Purpose |
|---|---|---|
| `android/` | Android | Android Studio project root, containing gradle configurations, Java/Kotlin classes, resource files, and packaging metadata. |
| `appimage/` | Linux Desktop | AppImage packaging scripts (`build.sh`, `apprun.sh`) and desktop shortcut metadata. |
| `.ci/` | Desktop / Android | Continuous Integration shell and PowerShell scripts for desktop platforms and Android. |
| `.github/` | CI / Project | GitHub repository setup containing actions, issue templates, and workflow settings. |
| `i18n/qt/` | Desktop | Qt-specific translation source files (`.ts`). (Note: JSON overlay files in `i18n/lang` are kept as they are needed for SDL3 overlay UI). |
| `vita3k/gui-qt/` | Desktop | Complete Qt-based GUI desktop source files, headers, and CMake config. |
| `vita3k/android/` | Android | Android JNI wrapper implementation files (`jni/` subdirectory). |
| `_building/` | Desktop | Contains documentation assets (`vs-cmd-prompt.png`) for Windows MSVC building. |

### Individual Files for Deletion

| File Path | Target Platform | Description / Purpose |
|---|---|---|
| `vita3k/main.cpp` | Desktop | Desktop entry point coordinating Qt GUI initialization. |
| `vita3k/Vita3K.rc` | Windows Desktop | Windows resource file containing versioning info and icon bindings. |
| `vita3k/resource.h` | Windows Desktop | Resource header mapping definitions for `Vita3K.rc`. |
| `vita3k/Windows.manifest` | Windows Desktop | Manifest file specifying DPI awareness, UAC configuration, etc. |
| `vita3k/Vita3K.ico` | Windows Desktop | Desktop app icon asset in ICO format. |
| `vita3k/Vita3K.png` | Desktop | High-resolution PNG logo, used to generate macOS icns. |
| `vita3k/resources.qrc` | Desktop | Qt resources descriptor configuration. |
| `cmake/qt6.cmake` | Desktop | CMake setup script for fetching/configuring Qt6. |
| `cmake/vcpkg_android.cmake`| Android | CMake setup script for Android dependencies through vcpkg. |
| `vita3k/util/src/vc_runtime_checker.cpp` | Windows Desktop | Windows-only MSVC CRT version detection and update prompt logic. |
| `vita3k/util/src/android_driver.cpp` | Android | Utility driver hooks and setup code for Qualcomm/Android GPUs. |
| `vita3k/util/include/util/android_driver.h` | Android | Header for Android driver loading utility. |
| `gen-linux.sh` | Linux Desktop | Helper script to generate project configuration using ninja presets. |
| `gen-windows.bat` | Windows Desktop | Helper batch file to configure Visual Studio project. |

---

## 2. Platform-Specific Assets and Logic Checked and Confirmed

1. **Overlay Interface (`vita3k/overlay`)**: Keep. Checked `vita3k/overlay/CMakeLists.txt` and verified it depends on `SDL3::SDL3` and custom headers. It does not use Qt and is platform-agnostic, running on iOS.
2. **Mac Network Helper (`vita3k/modules/SceNet/macos_net_helper.cpp/.h`)**: Keep. Checked file contents and `vita3k/modules/CMakeLists.txt`. It uses system APIs (`SystemConfiguration/SystemConfiguration.h` framework) that are fully compatible and required for networking interfaces on iOS.
3. **vcpkg.json & CMakeLists.txt**: Must be edited in subsequent milestones (Milestones 2-4) to remove linkages/references to the deleted directories and files.
4. **codemagic.yaml**: Keep. Configures the iOS-exclusive build workflow.

---

## 3. Recommended Deletion Strategy

All deletions should be performed sequentially and verified via simple file check commands. The directory cleanup list covers:
1. Deleting target root folders: `android`, `appimage`, `.ci`, `.github`, `_building`.
2. Deleting subdirectories: `i18n/qt`, `vita3k/gui-qt`, `vita3k/android`.
3. Deleting root shell/batch files: `gen-linux.sh`, `gen-windows.bat`.
4. Deleting vita3k/cmake specific files: `vita3k/main.cpp`, `vita3k/Vita3K.rc`, `vita3k/resource.h`, `vita3k/Windows.manifest`, `vita3k/Vita3K.ico`, `vita3k/Vita3K.png`, `vita3k/resources.qrc`, `cmake/qt6.cmake`, `cmake/vcpkg_android.cmake`, `vita3k/util/src/vc_runtime_checker.cpp`, `vita3k/util/src/android_driver.cpp`, `vita3k/util/include/util/android_driver.h`.

---

## 4. Verification Methods

To independently verify that cleanup was successful, run the following PowerShell queries in the repository root:

```powershell
# Expected output: All must return False
Test-Path "vita3k/main.cpp"
Test-Path "vita3k/gui-qt"
Test-Path "vita3k/android"
Test-Path "vita3k/Vita3K.rc"
Test-Path "vita3k/resource.h"
Test-Path "vita3k/Windows.manifest"
Test-Path "cmake/qt6.cmake"
Test-Path "appimage"
Test-Path ".ci"
Test-Path ".github"
Test-Path "i18n/qt"
Test-Path "_building"
Test-Path "android"
Test-Path "gen-linux.sh"
Test-Path "gen-windows.bat"
Test-Path "vita3k/util/src/vc_runtime_checker.cpp"
Test-Path "vita3k/util/src/android_driver.cpp"
Test-Path "vita3k/util/include/util/android_driver.h"
Test-Path "cmake/vcpkg_android.cmake"
```
