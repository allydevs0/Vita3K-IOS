# iOS Target Cleanup and Simplification Analysis

## Overview
This report analyzes `vcpkg.json` and the root `CMakeLists.txt` of the Vita3K project to plan the cleanup and simplification of the build system for targeting iOS exclusively. 

By pruning desktop-only and platform-specific configurations for Android, Windows (MSVC), and Linux (AppImage, etc.), the build system can be simplified from 300 lines down to roughly 64 lines (an ~78% reduction in complexity).

---

## 1. Analysis of `vcpkg.json` Dependencies

The current `vcpkg.json` contains:
```json
  "dependencies": [
    "boost-filesystem",
    "boost-icl",
    "boost-program-options",
    "boost-system",
    "boost-variant",
    "curl",
    "openssl",
    "zlib"
  ]
```

### Dependencies to Remove:
1. **`boost-icl` (Interval Container Library)**:
   - **Reason**: This package is completely unused in the active source code.
   - **Evidence**: Grepping for `boost/icl` or `boost::icl` returns 0 occurrences. There is only a single TODO comment in `vita3k/kernel/src/debugger.cpp` suggesting its future use.
2. **`boost-program-options`**:
   - **Reason**: This package is used to parse command-line options on desktop platforms. iOS apps do not run in a command-line environment or parse standard CLI options.
   - **Evidence**: Grepping for `program_options` or `options_description` shows 0 usages in the C++ source code.
3. **`boost-variant`**:
   - **Reason**: The codebase targets C++23 (`set(CMAKE_CXX_STANDARD 23)`), which provides the standard library `std::variant`. 
   - **Evidence**: Grepping for `boost::variant` or `boost/variant` confirms there are 0 usages.

### Dependencies to Keep:
- **`boost-filesystem`**: Extensively used across the utility, I/O, and configuration modules (e.g. `vita3k/util/include/util/fs.h`, `vita3k/io/src/filesystem.cpp`).
- **`boost-system`**: Required by `boost-filesystem` for system-related error-handling objects (`boost::system::error_code`).
- **`curl`**, **`openssl`**, **`zlib`**: Core networking, cryptography, and compression dependencies required on iOS.

A clean `proposed_vcpkg.json` file has been written in this folder.

---

## 2. Analysis of Root `CMakeLists.txt`

### 2.1 ANDROID Blocks to Remove
1. **Lines 3-8**:
   ```cmake
   if(CMAKE_SYSTEM_NAME STREQUAL "Android")
       set(ANDROID 1)
       set(VCPKG_TARGET_ANDROID 1)
       # this file must be included before everything else
       include("cmake/vcpkg_android.cmake")
   endif()
   ```
2. **Lines 34-50**:
   ```cmake
   if(ANDROID)
       set(CMAKE_SYSROOT "")
       set(CMAKE_OSX_SYSROOT "")
       set_property(GLOBAL PROPERTY NO_SYSTEM_FROM_IMPORTED ON)
       string(APPEND CMAKE_CXX_FLAGS " -Wl,-Bsymbolic -Wno-unused-command-line-argument")
       string(APPEND CMAKE_C_FLAGS " -Wl,-Bsymbolic -Wno-unused-command-line-argument")
       if(CMAKE_BUILD_TYPE STREQUAL "Release")
           set(CMAKE_C_VISIBILITY_PRESET hidden)
           set(CMAKE_CXX_VISIBILITY_PRESET hidden)
           set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
       endif()
   ...
   ```
3. **Lines 87-92**:
   ```cmake
   if(ANDROID)
       STRING(REPLACE "-fuse-ld=gold" "" CMAKE_CXX_LINK_OPTIONS_IPO "${CMAKE_CXX_LINK_OPTIONS_IPO}")
       STRING(REPLACE "-fuse-ld=gold" "" CMAKE_C_LINK_OPTIONS_IPO "${CMAKE_C_LINK_OPTIONS_IPO}")
       set(ipo_supported TRUE)
   else()
   ...
   ```
   *Simplification*: The `ANDROID` checks can be removed. The LTO section can execute the `else` branch unconditionally (calling standard `check_ipo_supported`).

### 2.2 Options `USE_DISCORD_RICH_PRESENCE` and `BUILD_APPIMAGE` to Remove
1. **Lines 51-54**:
   ```cmake
   if((ARCHITECTURE STREQUAL "x86_64") OR APPLE)
       option(USE_DISCORD_RICH_PRESENCE "Build Vita3K with Discord Rich Presence" ON)
   endif()
   option(BUILD_APPIMAGE "Build an AppImage." OFF)
   ```
2. **Lines 286-295**:
   ```cmake
   if (${BUILD_APPIMAGE})
       # AppImage packaging script execution
   ...
   endif()
   ```
   *Action*: Remove both options and the post-build block.
   *Subdirectory Cleanup*:
   - In `external/CMakeLists.txt` (line 148), remove the block that downloads and extracts the Discord Game SDK dylibs.
   - In `vita3k/CMakeLists.txt` (lines 94, 233, 306, 354, 379, 399), remove all blocks that define `-DUSE_DISCORD`, link `discord-rpc`, or copy the SDK libraries (`.dylib`/`.so`/`.dll`).
   - In `vita3k/app/CMakeLists.txt` (line 19), remove linking to the `discord-rpc` library.

### 2.3 MSVC / Windows Specific Compile Options to Remove
All blocks checking for MSVC compiler or Windows targets should be removed:
1. **Lines 67-74**:
   ```cmake
   if(MSVC)
       string(REPLACE "/Zi" "/Z7" ...)
   endif()
   ```
2. **Lines 270-279**:
   ```cmake
   if(WIN32)
       add_compile_definitions("_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" "_CRT_SECURE_NO_WARNINGS" "NOMINMAX")
       ...
   endif()
   ```
3. **Lines 280-284**:
   ```cmake
   if(CMAKE_GENERATOR MATCHES "Visual Studio" AND ...)
       string(APPEND CMAKE_C_FLAGS " /MP")
       ...
   endif()
   ```

### 2.4 Custom Desktop Boost Compilation Flags to Simplify/Remove
The original build system contains complex macros to compile Boost from an embedded source location (`b2_build`, `boost_compile`, `get_boost`) and options `VITA3K_FORCE_CUSTOM_BOOST` / `VITA3K_FORCE_SYSTEM_BOOST` (lines 109-269).
- **Simplification**: Delete all three macros and associated logic. Replace them with:
  ```cmake
  find_package(Boost REQUIRED COMPONENTS filesystem)
  ```
- **Why this is safe**: The vcpkg dependency manager handles Boost compilation and sets correct paths automatically when targeting iOS (`arm64-ios` triplet). As verified in `error.txt`:
  ```
  -- Found Boost: /Users/builder/clone/build-ios/vcpkg_installed/arm64-ios/share/boost/BoostConfig.cmake (found suitable version "1.90.0", minimum required is "1.81") found components: filesystem
  ```

### 2.5 iOS-Exclusive Target Configuration
To configure iOS as the target system exclusively and enable Objective-C and Swift support directly at the project level, we should set the following variables before the `project()` definition:
```cmake
set(CMAKE_SYSTEM_NAME iOS)
set(VITA3K_BUILD_IOS ON CACHE BOOL "Build iOS target exclusively" FORCE)
```
Then, declare the project and languages:
```cmake
project(Vita3K LANGUAGES C CXX OBJC Swift)
```
This replaces the old workaround check (lines 27-32) that tried to conditionally enable OBJC and Swift, simplifying the target setup.

A complete `proposed_CMakeLists.txt` file incorporating these changes has been created in this folder.
