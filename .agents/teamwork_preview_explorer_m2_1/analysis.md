# iOS Target Porting Analysis — Milestone 2

This report provides the detailed analysis and cleanup plan for `vcpkg.json` and the root `CMakeLists.txt` at `C:\Users\denise\Downloads\Vita3K` to target iOS exclusively for the **VitaiOS** project.

## Summary of Findings

1. **vcpkg.json**: Identify `boost-program-options` as a desktop-only/non-iOS dependency. There is no usage of this dependency in the C++ codebase, and it is not needed on iOS where command-line arguments are not parsed from a graphical shell launch.
2. **root CMakeLists.txt**:
   - **ANDROID block**: Remove system detection and flags for Android at lines 3-8, lines 34-50, and lines 87-91.
   - **USE_DISCORD_RICH_PRESENCE & BUILD_APPIMAGE**: Remove options, check structures, and external/local packaging triggers (lines 51-55, lines 286-295).
   - **MSVC/Windows settings**: Remove flag replacements and Windows-specific overrides (lines 67-74, lines 270-284).
   - **Custom desktop Boost build logic**: Remove ~160 lines of complex boost download/bootstrap/build code (lines 109-268) and replace it with a standard package lookup: `find_package(Boost REQUIRED COMPONENTS filesystem)`.
   - **iOS Target and Languages**: Pre-define `CMAKE_SYSTEM_NAME` to `iOS` and `VITA3K_BUILD_IOS` to `ON`, and unconditionally enable `OBJC` and `Swift`.

---

## Detailed Analysis

### 1. vcpkg.json Cleanup

The current `vcpkg.json` specifies:
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

- **`boost-program-options`**: This library is used for command-line arguments parsing. Ggrep search confirms there is no usage of `boost/program_options` headers or namespace in the source tree (`*.h`, `*.hpp`, `*.cpp`). Furthermore, iOS applications run in a sandboxed GUI container and do not receive command-line options. Thus, `boost-program-options` is safe to remove.
- **Other libraries** (`boost-filesystem`, `boost-icl`, `boost-system`, `boost-variant`, `curl`, `openssl`, `zlib`) are either standard core libraries or are actively utilized by core subsystems (such as `SceHttp`, `SceSsl`, cryptography, and package installers) and must remain.

### 2. root CMakeLists.txt Cleanup

#### 2.1 Remove Android Configuration Blocks
Android detection and setup should be removed since we target iOS exclusively:
- **Lines 3-8**:
  ```cmake
  if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  	set(ANDROID 1)
  	set(VCPKG_TARGET_ANDROID 1)
  	# this file must be included before everything else
  	include("cmake/vcpkg_android.cmake")
  endif()
  ```
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
- **Lines 87-91** (inside the LTO/IPO check):
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
  This is simplified to just the `else()` branch: `check_ipo_supported(RESULT ipo_supported OUTPUT ipo_supported_error)`.

#### 2.2 Remove Desktop Option Variables
The project-level variables `USE_DISCORD_RICH_PRESENCE` and `BUILD_APPIMAGE` are not applicable to iOS:
- **Lines 51-55**:
  ```cmake
  	if((ARCHITECTURE STREQUAL "x86_64") OR APPLE)
  		option(USE_DISCORD_RICH_PRESENCE "Build Vita3K with Discord Rich Presence" ON)
  	endif()
  	option(BUILD_APPIMAGE "Build an AppImage." OFF)
  ```
- **Lines 286-295**: The AppImage generation setup that validates Linux tools is also removed.

#### 2.3 Remove MSVC/Windows Compile Options and Definitions
All MSVC / Windows-specific builds are out of scope for the iOS target:
- **Lines 67-74**: The MSVC compiler `/Zi` debug flag conversion is removed.
- **Lines 270-278**: The `WIN32` preprocessor definitions (`_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS`, `_CRT_SECURE_NO_WARNINGS`, `NOMINMAX`) and PCH timestamps suppression are removed.
- **Lines 280-284**: The MSVC parallel builds `/MP` compiler flag is removed.

#### 2.4 Custom Boost Build System Simplification
- **Lines 109-268**: The current CMake file includes a complex set of macros (`b2_build`, `boost_compile`, `get_boost`) designed to bootstrap and compile Boost from source if a local copy is not found. This is a desktop-oriented legacy feature.
- **Simplification**: Since `vcpkg.json` includes Boost packages as dependencies, vcpkg will automatically build and provide them for the target iOS architecture. The entire custom compilation block can be deleted and replaced with a standard target configuration:
  ```cmake
  ############################
  ########## Boost ###########
  ############################

  # Find standard Boost package from vcpkg
  find_package(Boost REQUIRED COMPONENTS filesystem)
  ```

#### 2.5 Configure iOS Target and Enable Languages
Currently, iOS build triggers are guarded by platform checks and conditional option lookups:
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
- **Simplification**: Configure iOS target statically at the top of the file before `project(Vita3K)` is defined:
  ```cmake
  # Configure iOS exclusively as the target
  set(CMAKE_SYSTEM_NAME iOS CACHE STRING "Target iOS" FORCE)
  set(VITA3K_BUILD_IOS ON CACHE BOOL "Build iOS" FORCE)
  ```
  Then, unconditionally enable standard compilation support for Objective-C and Swift, which are both crucial for iOS development:
  ```cmake
  # Enable Objective-C and Swift for the iOS build
  enable_language(OBJC)
  enable_language(Swift)
  ```

---

## Proposed Cleaned Files

- **`proposed_vcpkg.json`**: Available at `.agents/teamwork_preview_explorer_m2_1/proposed_vcpkg.json`.
- **`proposed_CMakeLists.txt`**: Available at `.agents/teamwork_preview_explorer_m2_1/proposed_CMakeLists.txt`.
