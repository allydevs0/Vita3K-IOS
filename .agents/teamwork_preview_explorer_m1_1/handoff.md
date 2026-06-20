# Handoff Report: non-iOS File Deletion Plan

This report details the findings and deletion plan for removing all non-iOS directories and files to transition the Vita3K codebase into the iOS-exclusive VitaiOS.

## 1. Observation
The following paths and contents were verified on the local filesystem:
1. **Root Directory Scan (`C:\Users\denise\Downloads\Vita3K`)**:
   - `{"name":".ci","isDir":true}`
   - `{"name":".github","isDir":true}`
   - `{"name":"android","isDir":true}` (Root Android directory Gradle project)
   - `{"name":"appimage","isDir":true}`
   - `{"name":"gen-linux.sh","sizeBytes":"97"}`
   - `{"name":"gen-windows.bat","sizeBytes":"156"}`
2. **`vita3k/` Source Directory Scan (`C:\Users\denise\Downloads\Vita3K\vita3k`)**:
   - `{"name":"android","isDir":true}`
   - `{"name":"gui-qt","isDir":true}`
   - `{"name":"main.cpp","sizeBytes":"10008"}`
   - `{"name":"Vita3K.ico","sizeBytes":"370070"}`
   - `{"name":"Vita3K.png","sizeBytes":"124295"}`
   - `{"name":"Vita3K.rc","sizeBytes":"3866"}`
   - `{"name":"resource.h","sizeBytes":"1357"}`
   - `{"name":"resources.qrc","sizeBytes":"1608"}`
   - `{"name":"Windows.manifest","sizeBytes":"1374"}`
3. **`cmake/` Directory Scan (`C:\Users\denise\Downloads\Vita3K\cmake`)**:
   - `{"name":"qt6.cmake","sizeBytes":"2507"}`
   - `{"name":"vcpkg_android.cmake","sizeBytes":"3853"}`
4. **`i18n/` Directory Scan (`C:\Users\denise\Downloads\Vita3K\i18n`)**:
   - `{"name":"qt","isDir":true}` containing `{"name":"vita3k_en.ts","sizeBytes":"167113"}`
5. **Core Application Reference (`C:\Users\denise\Downloads\Vita3K\vita3k\app\src\app_init.cpp`)**:
   Lines 650-651:
   ```cpp
   const auto icon_path = emuenv.static_assets_path / "icons" / (std::string(grade_name) + ".png");
   std::ifstream file(icon_path.c_str(), std::ios::binary | std::ios::ate);
   ```
6. **Renderer reference (`C:\Users\denise\Downloads\Vita3K\vita3k\renderer\src\renderer.cpp`)**:
   Line 131:
   ```cpp
   auto icons_dir = fs_utils::path_to_utf8(static_assets / "icons");
   ```

## 2. Logic Chain
1. **Desktop/Android Exclusion Principle**: The objective is to make Vita3K iOS-only, meaning any directories/files serving desktop interfaces, desktop packaging, Android, or desktop/Android-only CI pipelines should be removed.
2. **Platform verification**:
   - Direct observations show that `vita3k/main.cpp`, `vita3k/gui-qt/`, `vita3k/android/`, `vita3k/Vita3K.rc`, `vita3k/resource.h`, `vita3k/Windows.manifest`, `cmake/qt6.cmake`, `appimage/`, `.ci/`, `.github/`, and `i18n/qt/` are all desktop or Android specific.
   - The root directories `android/`, `cmake/vcpkg_android.cmake`, `gen-linux.sh`, `gen-windows.bat`, `vita3k/resources.qrc`, `vita3k/Vita3K.ico`, and `vita3k/Vita3K.png` are also strictly desktop/Android build scripts, configurations, or assets.
3. **Core Dependency verification**:
   - Observations in `app_init.cpp` and `renderer.cpp` show that `static_assets_path / "icons" / <grade_name>.png` is loaded dynamically by the core emulator to render trophy notifications in the overlay.
   - Deleting the entire `vita3k/icons/` folder would break trophy notification graphics. Therefore, the trophy icons (`bronze.png`, `silver.png`, `gold.png`, `platinum.png`) must be preserved, while other Qt GUI-specific icons (e.g. `configure.png`, `controllers.png`) can be safely removed.

## 3. Caveats
- This investigation assumes that the existing CMake configuration files (`CMakeLists.txt` in root, `vita3k/CMakeLists.txt`, `external/CMakeLists.txt`) will be modified in later milestones (Milestones 2, 3, 4) to strip out references to these deleted targets. Direct deletion without modifying CMake will temporarily break configuration unless CMake files are updated immediately after.
- Retaining only `bronze.png`, `silver.png`, `gold.png`, `platinum.png` in the `vita3k/icons/` folder assumes no other files in that directory are loaded dynamically by other core features. We found no other references to icons in this folder.

## 4. Conclusion
We recommend proceeding with the deletion of:
1. `vita3k/main.cpp`
2. `vita3k/gui-qt/`
3. `vita3k/android/`
4. `vita3k/Vita3K.rc`, `vita3k/resource.h`, `vita3k/Windows.manifest`
5. `cmake/qt6.cmake`
6. `appimage/`
7. `.ci/`
8. `.github/`
9. `i18n/qt/`
10. Root `android/`
11. `cmake/vcpkg_android.cmake`
12. `gen-linux.sh`, `gen-windows.bat`
13. `vita3k/resources.qrc`, `vita3k/Vita3K.ico`, `vita3k/Vita3K.png`
14. Qt-specific UI icons inside `vita3k/icons/` (keeping `bronze.png`, `silver.png`, `gold.png`, `platinum.png` for trophy notifications).

## 5. Verification Method
After the deletion is executed, verification can be done by:
1. **Existence checks**:
   Ensure running `dir` or file search on any of the target files/directories returns a "not found" status.
2. **Preservation check**:
   Ensure `vita3k/icons/bronze.png`, `vita3k/icons/silver.png`, `vita3k/icons/gold.png`, and `vita3k/icons/platinum.png` still exist.
3. **Reference clean-up verification (subsequent milestones)**:
   Verify that cmake configuration does not error due to missing directories after deleting these targets and updating `CMakeLists.txt`.
