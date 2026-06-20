# Handoff Report: external/CMakeLists.txt Cleanup for iOS Target

This report provides the analysis and plan to clean up `external/CMakeLists.txt` for Vita3K to target iOS exclusively.

## 1. Observation
We examined the file `external/CMakeLists.txt` (located at `C:\Users\denise\Downloads\Vita3K\external\CMakeLists.txt`) and identified the following blocks and commands:

* **discord-rpc** (lines 148-196):
  ```cmake
  if(USE_DISCORD_RICH_PRESENCE)
  	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/discord_game_sdk.zip")
  		message(STATUS "Downloading discord gamesdk...")
  		file(DOWNLOAD https://dl-game-sdk.discordapp.net/latest/discord_game_sdk.zip
  			"${CMAKE_BINARY_DIR}/external/discord_game_sdk.zip" SHOW_PROGRESS)
  	endif()
  ...
  	add_library(discord-rpc STATIC
  ...
  ```
* **libadrenotools** (lines 111-118):
  ```cmake
  if(ANDROID)
  	if(ARCHITECTURE STREQUAL "arm64")
  		add_subdirectory(libadrenotools)
  		target_compile_definitions(adrenotools INTERFACE USE_ADRENO_TOOLS)
  	else()
  		add_library(adrenotools INTERFACE)
  	endif()
  endif()
  ```
* **winsock** (lines 120-126):
  ```cmake
  if(WIN32)
  	add_library(winsock INTERFACE)
  	find_library(WSOCK32 wsock32)
  	find_library(WS2_32 ws2_32)
  	find_library(IPHLPAPI iphlpapi)
  	target_link_libraries(winsock INTERFACE WSOCK32 WS2_32 IPHLPAPI)
  endif()
  ```
* **MoltenVK macOS download** (lines 229-240):
  ```cmake
  if(APPLE)
  	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar")
  		message(STATUS "Downloading MoltenVK...")
  		file(DOWNLOAD https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-macos.tar
  			"${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar" SHOW_PROGRESS)
  	endif()
  	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK")
  		execute_process(COMMAND tar xf "${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar" --strip-components=1 MoltenVK/MoltenVK 
  			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external")
  	endif()
  	target_include_directories(vulkan INTERFACE "${CMAKE_BINARY_DIR}/external/MoltenVK/include")
  	set(MOLTENVK_DYLIB "${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/macOS/libMoltenVK.dylib" PARENT_SCOPE)
  ```
* **Vulkan validation layers download** (lines 241-261):
  ```cmake
  elseif(ANDROID AND NOT (CMAKE_BUILD_TYPE STREQUAL "Release"))
  	# add validation layer
  	if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../android/prebuilt/${ANDROID_ABI}/libVkLayer_khronos_validation.so")
  		set(VALIDATION_BINARY "android-binaries-1.4.341.0")
  ...
  ```
* **nativefiledialog-extended** (lines 459-465):
  ```cmake
  if(NOT ANDROID)
  	# nativefiledialog-extended
  	add_subdirectory(nativefiledialog-extended)

  	# Create alias to prevent the need of multiple changes in case the target name changes
  	add_library(NFDe::NFDe ALIAS nfd)
  endif()
  ```
* **googletest** (lines 74-77):
  ```cmake
  add_library(googletest STATIC googletest/googletest/src/gtest_main.cc googletest/googletest/src/gtest-all.cc)
  target_include_directories(googletest PUBLIC googletest/googletest/include)
  target_include_directories(googletest PRIVATE googletest/googletest)
  target_compile_definitions(googletest PUBLIC GTEST_HAS_PTHREAD=0)
  ```
* **OpenSSL build-from-source for desktop** (lines 273-346):
  Contains macOS homebrew lookup block and downloading/compilation instructions via Host Perl scripts (`Configure darwin64-x86_64` / `darwin64-arm64`).

We also verified the exact list of required dependencies specified: `capstone`, `dynarmic`, `fmt`, `spdlog`, `SDL3` (subdirectory `sdl`), `cubeb`, `glslang`, `SPIRV-Cross`, `pugixml`, `miniz`, `yaml-cpp`, `ffmpeg`, `xxHash`, `dlmalloc`, `libatrac9`, `libfat16`, `stb`, `ddspp`, `cppcommon`, `CLI11`, `vulkan` headers, `vma`, `psvpfstools`, `ssl/crypto`.

## 2. Logic Chain
1. **Target Identification**: Based on observations, components like `discord-rpc`, `libadrenotools`, `winsock`, Vulkan validation layers (Android binaries), `nativefiledialog-extended`, `googletest`, and desktop-specific OpenSSL build blocks are platform-specific (Windows, macOS, or Android) and incompatible with an iOS-only compile target. Removing them simplifies build maintenance.
2. **MoltenVK Target Adaptation**: In the original setup, MoltenVK is configured to download `MoltenVK-macos.tar` and links to `macOS/libMoltenVK.dylib`. To adapt for iOS, we substitute the tarball path to `MoltenVK-ios.tar` (retaining version consistency with `v1.2.11-artifacts`) and dynamically check the target environment via `CMAKE_OSX_SYSROOT` to choose between `iOS/libMoltenVK.dylib` (device) and `iOS-Simulator/libMoltenVK.dylib` (simulator).
3. **OpenSSL Target Simplification**: The original CMake script builds OpenSSL from source using host architecture configurations. On cross-compiled iOS platforms, OpenSSL is located using `find_package(OpenSSL REQUIRED)` via the CMake toolchain configuration, allowing the interface library bindings `ssl` and `crypto` to be setup natively.
4. **Dependency Filtering**: By removing all non-iOS dependencies (such as `tracy`, `glad`, `concurrentqueue`, `vita-toolchain`, and `curl`) and keeping the specified list of 24 required dependencies, we obtain a lightweight, clean, and iOS-focused build definition.

## 3. Caveats
* **Empty Submodule Folders**: The submodules under `external/` (e.g., `sdl`, `ffmpeg`, `capstone`) were empty in the local workspace analysis because submodules were not recursively initialized in the read-only inspection workspace. This does not impact the analysis since the CMake structure remains fully verifiable.
* **Simulator vs. Device**: The proposed MoltenVK library selection relies on the standard `CMAKE_OSX_SYSROOT` to detect simulator environments. If the user invokes CMake with a non-standard iOS toolchain configuration, the fallback defaults to device binaries.

## 4. Conclusion
Cleaning up `external/CMakeLists.txt` for iOS requires removing non-iOS desktop dependencies, switching MoltenVK's download payload to the iOS version, and letting the cross-compilation environment resolve OpenSSL. We have proposed a clean CMake build script satisfying these constraints.

## 5. Verification Method
1. **Visual Inspection**: Open the proposed `external/CMakeLists.txt` code block inside the `analysis.md` report and confirm that only the 24 specified dependencies are included and that `MoltenVK-ios.tar` is used.
2. **CMake Configuration Verification**: Once changes are applied, verify they configure correctly by running standard CMake with an iOS toolchain:
   ```bash
   cmake -S . -B build -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator
   ```
   Invalidation condition: If the configuration fails, check if `OpenSSL` is correctly supplied by the toolchain environment.
