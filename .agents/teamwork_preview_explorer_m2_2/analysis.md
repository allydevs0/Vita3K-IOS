# Analysis Report: iOS Porting Cleanup — Milestone 2

This report outlines the required cleanups and simplifications to target **iOS exclusively** in the `vcpkg.json` and the root `CMakeLists.txt` of the Vita3K project.

---

## 1. vcpkg.json Dependency Analysis

### 1.1 Desktop-Only/Non-iOS Dependencies to Remove
- **`boost-program-options`** (line 8):
  - **Function**: Used for command-line arguments parsing.
  - **Rationale for removal**: Standard iOS applications are compiled into sandboxed `.app` bundles and launched by the iOS SpringBoard interface (GUI-only) without command-line arguments. In addition, a `grep_search` across the codebase reveals zero `#include` directives or direct uses of Boost Program Options API. Thus, it is obsolete for iOS and should be removed.

### 1.2 Dependencies to Retain
The remaining dependencies in `vcpkg.json` are cross-platform libraries used by the emulator core:
- `boost-filesystem`, `boost-icl`, `boost-system`, `boost-variant` (core Boost libraries for utility structures, filesystems, and emulated memory mapping).
- `curl`, `openssl` (networking and SSL interfaces for PlayStation Network emulator services).
- `zlib` (used for compression/decompression of packages and archives).

---

## 2. Root CMakeLists.txt Analysis and Cleanup Plan

### 2.1 Removing ANDROID Blocks
Since the build target is iOS exclusively, all Android-specific code path branches are defunct and should be removed:
- **Lines 3-8**:
  ```cmake
  if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  	set(ANDROID 1)
  	set(VCPKG_TARGET_ANDROID 1)
  	# this file must be included before everything else
  	include("cmake/vcpkg_android.cmake")
  endif()
  ```
  *Cleanup action*: Delete this block completely.
- **Lines 34-50**:
  ```cmake
  if(ANDROID)
  	set(CMAKE_SYSROOT "")
  	set(CMAKE_OSX_SYSROOT "")
  	set_property(GLOBAL
  	PROPERTY
  	NO_SYSTEM_FROM_IMPORTED ON
  	)

  	string(APPEND CMAKE_CXX_FLAGS " -Wl,-Bsymbolic -Wno-unused-command-line-argument")
  	string(APPEND CMAKE_C_FLAGS " -Wl,-Bsymbolic -Wno-unused-command-line-argument")

  	if(CMAKE_BUILD_TYPE STREQUAL "Release")
  		set(CMAKE_C_VISIBILITY_PRESET hidden)
  		set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  		set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
  	endif()
  else()
     ...
  endif()
  ```
  *Cleanup action*: Delete the `if(ANDROID)` branch. The `else()` block logic (excluding the desktop-only options inside it) will be evaluated unconditionally.
- **Lines 87-94**:
  ```cmake
  	if(ANDROID)
  		# https://gitlab.kitware.com/cmake/cmake/-/issues/21772
  		STRING(REPLACE "-fuse-ld=gold" "" CMAKE_CXX_LINK_OPTIONS_IPO "${CMAKE_CXX_LINK_OPTIONS_IPO}")
  		STRING(REPLACE "-fuse-ld=gold" "" CMAKE_C_LINK_OPTIONS_IPO "${CMAKE_C_LINK_OPTIONS_IPO}")
  		set(ipo_supported TRUE)
  	else()
  		check_ipo_supported(RESULT ipo_supported OUTPUT ipo_supported_error)
  	endif()
  ```
  *Cleanup action*: Replace the entire conditional block with the `else()` branch body:
  ```cmake
  	check_ipo_supported(RESULT ipo_supported OUTPUT ipo_supported_error)
  ```

### 2.2 Removing Desktop Options (`USE_DISCORD_RICH_PRESENCE` & `BUILD_APPIMAGE`)
Desktop-only feature toggles and packaging steps are not applicable on iOS:
- **Lines 51-55**:
  ```cmake
  	if((ARCHITECTURE STREQUAL "x86_64") OR APPLE)
  		option(USE_DISCORD_RICH_PRESENCE "Build Vita3K with Discord Rich Presence" ON)
  	endif()
  	option(BUILD_APPIMAGE "Build an AppImage." OFF)
  ```
  *Cleanup action*: Delete both options completely.
- **Lines 286-295**:
  ```cmake
  if (${BUILD_APPIMAGE})
  	if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  		set(LINUXDEPLOY_COMMAND "${CMAKE_SOURCE_DIR}/appimage/linuxdeploy.AppImage" CACHE INTERNAL "")
  		if (NOT EXISTS "${LINUXDEPLOY_COMMAND}")
  			message(FATAL_ERROR "Could not find linuxdeploy at ${LINUXDEPLOY_COMMAND}!")
  		endif()
  	else()
  		message(FATAL_ERROR "Cannot build an AppImage for a non-Linux host.")
  	endif()
  endif()
  ```
  *Cleanup action*: Delete this block completely.

### 2.3 Removing MSVC / Windows Specific Configurations
Since the compiler for iOS is always Apple Clang (via Xcode or makefiles), MSVC and Windows-specific blocks are obsolete:
- **Lines 67-74**:
  ```cmake
  if(MSVC)
  	string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  	string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  	string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  	string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
  	string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  	string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
  endif()
  ```
  *Cleanup action*: Delete this block completely.
- **Lines 270-278**:
  ```cmake
  if(WIN32)
  	add_compile_definitions("_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" "_CRT_SECURE_NO_WARNINGS" "NOMINMAX")

  	# Disable inclusion of timestamps in PCH files on Windows
  	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  		string(APPEND CMAKE_C_FLAGS " -Xclang -fno-pch-timestamp")
  		string(APPEND CMAKE_CXX_FLAGS " -Xclang -fno-pch-timestamp")
  	endif()
  endif()
  ```
  *Cleanup action*: Delete this block completely.
- **Lines 280-284**:
  ```cmake
  if(CMAKE_GENERATOR MATCHES "Visual Studio" AND (CMAKE_C_COMPILER_ID MATCHES "MSVC|Intel|Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "MSVC|Intel|Clang"))
  	string(APPEND CMAKE_C_FLAGS " /MP")
  	string(APPEND CMAKE_CXX_FLAGS " /MP")
  endif()
  ```
  *Cleanup action*: Delete this block completely.

### 2.4 Simplifying Boost Logic
- **Lines 109-269**: The root CMake contains ~160 lines of complex custom macro logic (`b2_build`, `boost_compile`, `get_boost`) that bootstraps, builds, and manages custom Boost distributions for various platforms.
- **Simplification**: Since we are using `vcpkg` for iOS build dependency management, vcpkg compiles and provides the standard Boost package for iOS. We can completely replace this entire section (lines 109-269) with a standard CMake call:
  ```cmake
  ############################
  ########## Boost ###########
  ############################

  # Find standard Boost package from vcpkg
  find_package(Boost REQUIRED COMPONENTS filesystem)
  ```

### 2.5 Configuring iOS as Target and Activating Languages
- **Lines 22-32**:
  ```cmake
  set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)

  if(APPLE)
      enable_language(OBJC)
      if(VITA3K_BUILD_IOS)
          enable_language(Swift)
      endif()
  endif()
  ```
- **Simplification**: Set the target platform configurations at the top of the file (before the `project()` call) so CMake initiates with the correct cross-compilation toolchain:
  ```cmake
  # Configure iOS exclusively as the target
  set(CMAKE_SYSTEM_NAME iOS CACHE STRING "Target iOS" FORCE)
  set(VITA3K_BUILD_IOS ON CACHE BOOL "Build iOS" FORCE)
  ```
  Then, enable `OBJC` and `Swift` languages unconditionally after the `project(Vita3K)` call:
  ```cmake
  # Enable Objective-C and Swift for the iOS build
  enable_language(OBJC)
  enable_language(Swift)
  ```

---

## 3. Proposed Files Diffs and Layout

The files `proposed_vcpkg.json` and `proposed_CMakeLists.txt` generated by the peer agent `explorer_m2_1` are verified as fully syntactically correct and clean. They represent the final target state for the implementations in Milestone 2.
