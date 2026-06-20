# Handoff Report: Milestone 1.2 File Deletion Plan

This handoff report summarizes the findings of the deletion planning phase for the non-iOS codebase cleanup.

## 1. Observation

Direct file-system observations were conducted to verify file existence and dependencies:

1. **Existence of Target Paths**:
   - `vita3k/main.cpp` (File, size: `10008 bytes`)
   - `vita3k/gui-qt/` (Directory)
   - `vita3k/android/` (Directory)
   - `vita3k/Vita3K.rc` (File, size: `3866 bytes`)
   - `vita3k/resource.h` (File, size: `1357 bytes`)
   - `vita3k/Windows.manifest` (File, size: `1374 bytes`)
   - `cmake/qt6.cmake` (File, size: `2507 bytes`)
   - `appimage/` (Directory)
   - `.ci/` (Directory)
   - `.github/` (Directory)
   - `i18n/qt/` (Directory)

2. **Additional Targets Identified**:
   - `android/` (Root directory, Gradle wrapper/assets)
   - `cmake/vcpkg_android.cmake` (File, size: `3853 bytes`)
   - `gen-linux.sh` (File, size: `97 bytes`)
   - `gen-windows.bat` (File, size: `156 bytes`)
   - `vita3k/resources.qrc` (File, size: `1608 bytes`)
   - `vita3k/Vita3K.ico` (File, size: `370070 bytes`)
   - `vita3k/Vita3K.png` (File, size: `124295 bytes`)

3. **External Dependencies in `external/`**:
   - `external/libadrenotools/` (Directory, Android-specific)
   - `external/nativefiledialog-extended/` (Directory, Desktop-only)
   - `external/googletest/` (Directory, Test framework)
   - `external/tracy/` (Directory, Profiler)
   - `external/dirent/` (Directory, Windows/MSVC compatibility headers)

4. **Trophy and Overlay Icon Code References**:
   - `vita3k/app/src/app_init.cpp:650`:
     `const auto icon_path = emuenv.static_assets_path / "icons" / (std::string(grade_name) + ".png");`
   - `vita3k/overlay/src/common_dialog.cpp:134-136`:
     ```cpp
     m_cross_icon_data = load_icon_white("cross.png");
     m_info_icon_btn_data = load_icon_white("info.png");
     m_back_arrow_icon_data = load_icon_white("doublearrow.png");
     ```

## 2. Logic Chain

1. **Target Removal**: All 9 files/folders explicitly named in the request are desktop or Android platform files and are not used by the iOS SDL3 target. Therefore, they should be deleted.
2. **Additional Root Cleanups**: The root-level `android/` directory contains Android-only build infrastructure (gradle, gradle properties, gradlew, and android app assets/configs). Similarly, `cmake/vcpkg_android.cmake` is used only for the Android build target. Helper files like `gen-linux.sh` and `gen-windows.bat` are helper scripts for desktop targets. Deleting them cleans the repository root.
3. **Desktop Asset Cleanups**: `resources.qrc` is a Qt resource definition file, and since the Qt GUI is being removed, it is obsolete. `Vita3K.ico` and `Vita3K.png` are desktop platform icons. Therefore, they should be deleted.
4. **External Library Cleanups**: `libadrenotools` is used to load custom drivers on Adreno GPUs (Android specific). `nativefiledialog-extended` is for native desktop file dialogs. `tracy` is disabled for the iOS executable target. `dirent` provides POSIX compatibility for Windows (which lacks standard POSIX headers, whereas iOS is built on Unix/POSIX and natively supports `<dirent.h>`). `googletest` is not needed for compiling the emulator. Therefore, these directories in `external/` should be deleted.
5. **Selective Icon Retention**: While SVG and PNG layout assets in `vita3k/icons/` are Qt-specific, `bronze.png`, `silver.png`, `gold.png`, and `platinum.png` are referenced dynamically in the emulation core (`app_init.cpp:650`) for trophy notifications, and `cross.png`, `doublearrow.png`, and `info.png` are referenced in the SDL3 in-game overlay (`common_dialog.cpp:134-136`). Thus, the folder must not be deleted entirely; rather, only these seven assets must be preserved, and the other Qt-specific icons deleted.

## 3. Caveats

- We assume that `external/substitute` (which is empty in the workspace) does not need to be deleted or modified in this milestone.
- We assume that other assets inside `data/` are platform-independent and should be retained.

## 4. Conclusion

A comprehensive deletion plan has been created in `analysis.md` outlining the deletion of the 9 requested targets, 7 additional build/platform scripts, 5 third-party external dependencies, and selective cleanup of `vita3k/icons/` preserving the 7 required core files.

## 5. Verification Method

To verify the cleanup:
1. Run a directory check command (e.g. `dir` or `ls` command in the respective paths) to confirm they no longer exist.
2. Verify that the 7 preserved files (`bronze.png`, `silver.png`, `gold.png`, `platinum.png`, `cross.png`, `doublearrow.png`, `info.png`) still exist in `vita3k/icons/`.
3. Running CMake configure for iOS target (to be completed in Milestone 2) should run without referencing any of the deleted targets.
