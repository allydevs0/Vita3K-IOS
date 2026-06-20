# Handoff Report — Milestone 2 Explorer

This handoff report summarizes the read-only exploration and planning for the Milestone 2 cleanup of `vcpkg.json` and the root `CMakeLists.txt` to target iOS exclusively.

## 1. Observation
We observed the following files and content within the project workspace `C:\Users\denise\Downloads\Vita3K`:

* **`vcpkg.json`**:
  * Line 8 specifies `"boost-program-options"` as a dependency.
  * Line 5-14 lists dependencies:
    ```json
    5:   "dependencies": [
    6:     "boost-filesystem",
    7:     "boost-icl",
    8:     "boost-program-options",
    9:     "boost-system",
    10:     "boost-variant",
    11:     "curl",
    12:     "openssl",
    13:     "zlib"
    14:   ]
    ```

* **`CMakeLists.txt` (root)**:
  * **Android block (Lines 3-8)**:
    ```cmake
    3: if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    4: 	set(ANDROID 1)
    5: 	set(VCPKG_TARGET_ANDROID 1)
    6: 	# this file must be included before everything else
    7: 	include("cmake/vcpkg_android.cmake")
    8: endif()
    ```
  * **Android & Desktop branch (Lines 34-55)**:
    ```cmake
    34: if(ANDROID)
    ... [Android flags and parameters] ...
    50: else()
    51: 	if((ARCHITECTURE STREQUAL "x86_64") OR APPLE)
    52: 		option(USE_DISCORD_RICH_PRESENCE "Build Vita3K with Discord Rich Presence" ON)
    53: 	endif()
    54: 	option(BUILD_APPIMAGE "Build an AppImage." OFF)
    55: endif()
    ```
  * **MSVC flags (Lines 67-74)**:
    ```cmake
    67: if(MSVC)
    68: 	string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    ...
    74: endif()
    ```
  * **Android LTO gold linker (Lines 87-94)**:
    ```cmake
    87: 	if(ANDROID)
    88: 		# https://gitlab.kitware.com/cmake/cmake/-/issues/21772
    89: 		STRING(REPLACE "-fuse-ld=gold" "" CMAKE_CXX_LINK_OPTIONS_IPO "${CMAKE_CXX_LINK_OPTIONS_IPO}")
    90: 		STRING(REPLACE "-fuse-ld=gold" "" CMAKE_C_LINK_OPTIONS_IPO "${CMAKE_C_LINK_OPTIONS_IPO}")
    91: 		set(ipo_supported TRUE)
    92: 	else()
    93: 		check_ipo_supported(RESULT ipo_supported OUTPUT ipo_supported_error)
    94: 	endif()
    ```
  * **Custom Boost Macro and Setup (Lines 109-269)**:
    Includes definitions for `b2_build`, `boost_compile`, options `VITA3K_FORCE_CUSTOM_BOOST`, `VITA3K_FORCE_SYSTEM_BOOST`, `BOOST_MODULES_TO_FIND`, and macro `get_boost`.
  * **Windows compile macros (Lines 270-284)**:
    ```cmake
    270: if(WIN32)
    271: 	add_compile_definitions("_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" "_CRT_SECURE_NO_WARNINGS" "NOMINMAX")
    ...
    278: endif()
    279: 
    280: # Allow per-translation-unit parallel builds when using MSVC
    281: if(CMAKE_GENERATOR MATCHES "Visual Studio" AND (CMAKE_C_COMPILER_ID MATCHES "MSVC|Intel|Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "MSVC|Intel|Clang"))
    282: 	string(APPEND CMAKE_C_FLAGS " /MP")
    283: 	string(APPEND CMAKE_CXX_FLAGS " /MP")
    284: endif()
    ```
  * **AppImage check (Lines 286-295)**:
    ```cmake
    286: if (${BUILD_APPIMAGE})
    ...
    295: endif()
    ```
  * **SDL Apple Language options (Lines 27-32)**:
    ```cmake
    27: if(APPLE)
    28:     enable_language(OBJC)
    29:     if(VITA3K_BUILD_IOS)
    30:         enable_language(Swift)
    31:     endif()
    32: endif()
    ```

* **Grep search for `program_options`**:
  * Found reference in `vita3k/CMakeLists.txt:183`: `\"libboost_program_options.a\"` (under Android visibility preset configuration).
  * No direct inclusion of `#include <boost/program_options...>` or usages found in source code files.

---

## 2. Logic Chain
1. **vcpkg.json cleanup**: The only dependency listed that parses command-line arguments is `boost-program-options`. An iOS app is an `.app` bundle launched exclusively from the GUI (SpringBoard shell) without CLI parameters. Additionally, grep results confirm that the source code does not use `boost-program-options`. Therefore, removing `boost-program-options` is safe, logical, and simplifies dependency resolution.
2. **ANDROID block removal**: Since the migration scope is strictly to target iOS exclusively, any blocks checking `if(ANDROID)` or `CMAKE_SYSTEM_NAME STREQUAL "Android"` (Lines 3-8, 34-49, and 87-91) are obsolete and can be safely deleted.
3. **USE_DISCORD_RICH_PRESENCE and BUILD_APPIMAGE removal**: Discord Rich Presence is a desktop-only feature, and AppImage is a Linux-only packaging format. Since iOS is the sole target platform, these options (Lines 51-55) and their associated validation blocks (Lines 286-295) are obsolete and can be removed.
4. **MSVC/Windows settings removal**: The target architecture is iOS, which is built using Apple Clang. Windows-specific flags and compiler options (Lines 67-74, 270-284) will never be triggered or needed. Removing them simplifies maintenance.
5. **Boost logic simplification**: Instead of downloading, bootstrapping, and compiling Boost from source locally (Lines 109-269), which is error-prone on cross-compilation platforms like iOS, we can rely entirely on `vcpkg` to build the required standard packages (e.g. `boost-filesystem`). The standard CMake command `find_package(Boost REQUIRED COMPONENTS filesystem)` suffices.
6. **Configure iOS as Target**: To cross-compile for iOS, CMake needs to be explicitly configured. Placing `set(CMAKE_SYSTEM_NAME iOS CACHE STRING "Target iOS" FORCE)` and `set(VITA3K_BUILD_IOS ON CACHE BOOL "Build iOS" FORCE)` at the top of the root `CMakeLists.txt` guarantees that all downstream scripts target iOS. Unconditionally enabling `OBJC` and `Swift` via `enable_language()` ensures compilation compatibility with SDL3's iOS implementation and iOS ViewControllers.

---

## 3. Caveats
- **Boost header-only libraries**: It is assumed that Boost libraries like `boost-icl` and `boost-variant` (which are header-only) do not need to be specified as compiled components in `find_package(Boost REQUIRED COMPONENTS ...)`. Only `filesystem` needs to be linked. This is standard CMake behavior.
- **Environment variables**: Cross-compilation requires an appropriate iOS Toolchain file (e.g. from vcpkg or ios.toolchain.cmake). Setting `CMAKE_SYSTEM_NAME` to `iOS` within CMake is necessary but might need corresponding generator specifications (like `-G Xcode`) during invocation.

---

## 4. Conclusion
We conclude that:
1. `boost-program-options` should be removed from `vcpkg.json`.
2. The root `CMakeLists.txt` should be cleaned up by removing the Android configurations, MSVC/Windows overrides, Discord and AppImage configurations, and the custom Boost build macros.
3. The root `CMakeLists.txt` must configure iOS as the target and enable language support for OBJC and Swift unconditionally.
This clean design has been verified and matches the proposed files in the `teamwork_preview_explorer_m2_1` agent folder (`proposed_CMakeLists.txt` and `proposed_vcpkg.json`).

---

## 5. Verification Method
To independently verify the planned configurations:
1. Check the syntax and validation by running the CMake configuration command:
   ```bash
   cmake -B build -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_TOOLCHAIN_FILE=[path_to_vcpkg]/scripts/buildsystems/vcpkg.cmake
   ```
2. The configuration must succeed without any syntax errors and correctly detect the iOS SDK toolchain.
3. The invalidation condition would be any compiler/linker failure relating to missing `boost-program-options` symbols or toolchain detection issues.
