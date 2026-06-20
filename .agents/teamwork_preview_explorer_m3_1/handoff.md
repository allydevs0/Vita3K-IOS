# Handoff Report: external/CMakeLists.txt Cleanup for iOS Target

## 1. Observation
We observed the contents of `external/CMakeLists.txt` at `C:\Users\denise\Downloads\Vita3K\external\CMakeLists.txt` via `view_file` (lines 1 to 466). Specifically, we identified the following relevant dependency blocks:
- **discord-rpc** at lines 148–196:
  ```cmake
  if(USE_DISCORD_RICH_PRESENCE)
  	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/discord_game_sdk.zip")
  		message(STATUS "Downloading discord gamesdk...")
  		file(DOWNLOAD https://dl-game-sdk.discordapp.net/latest/discord_game_sdk.zip
  ...
  ```
- **libadrenotools** at lines 111–118:
  ```cmake
  if(ANDROID)
  	if(ARCHITECTURE STREQUAL "arm64")
  		add_subdirectory(libadrenotools)
  		target_compile_definitions(adrenotools INTERFACE USE_ADRENO_TOOLS)
  ...
  ```
- **winsock** at lines 120–126:
  ```cmake
  if(WIN32)
  	add_library(winsock INTERFACE)
  	find_library(WSOCK32 wsock32)
  ...
  ```
- **MoltenVK macOS tarball download and linkage setup** at lines 229–240:
  ```cmake
  if(APPLE)
  	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar")
  		message(STATUS "Downloading MoltenVK...")
  		file(DOWNLOAD https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-macos.tar
  ...
  	target_include_directories(vulkan INTERFACE "${CMAKE_BINARY_DIR}/external/MoltenVK/include")
  	set(MOLTENVK_DYLIB "${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/macOS/libMoltenVK.dylib" PARENT_SCOPE)
  ```
- **Vulkan Validation Layers download** at lines 241–261:
  ```cmake
  elseif(ANDROID AND NOT (CMAKE_BUILD_TYPE STREQUAL "Release"))
  	# add validation layer
  	if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../android/prebuilt/${ANDROID_ABI}/libVkLayer_khronos_validation.so")
  ...
  ```
- **nativefiledialog-extended** at lines 459–466:
  ```cmake
  if(NOT ANDROID)
  	# nativefiledialog-extended
  	add_subdirectory(nativefiledialog-extended)
  ...
  ```
- **googletest** at lines 74–78:
  ```cmake
  add_library(googletest STATIC googletest/googletest/src/gtest_main.cc googletest/googletest/src/gtest-all.cc)
  ...
  ```
- **OpenSSL build-from-source for desktop** at lines 273–355:
  ```cmake
  if(APPLE OR NOT FORCE_BUILD_OPENSSL_MAC)
  ...
  		if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/openssl.tar.gz")
  			message(STATUS "Downloading OpenSSL@3 from source...")
  			file(DOWNLOAD https://github.com/openssl/openssl/releases/download/openssl-3.3.2/openssl-3.3.2.tar.gz
  ...
  ```

Additionally, in `CMakeLists.txt` (root file) lines 3–5, we observed that iOS is configured exclusively as the system target:
```cmake
set(CMAKE_SYSTEM_NAME iOS CACHE STRING "Target iOS" FORCE)
set(VITA3K_BUILD_IOS ON CACHE BOOL "Build iOS" FORCE)
```

## 2. Logic Chain
1. Since the root `CMakeLists.txt` configures the build exclusively for iOS, any dependencies that are platform-specific to Windows (`winsock`), Android (`libadrenotools`, Android Vulkan validation layers), or macOS/Desktop-only platforms (`discord-rpc`, `nativefiledialog-extended`) are incompatible and can be safely removed.
2. MoltenVK is required for iOS Vulkan rendering, but the current block download targets macOS (`MoltenVK-macos.tar`) and sets the library path to the macOS dylib (`dylib/macOS/libMoltenVK.dylib`). To support iOS, this block must be adjusted to download `MoltenVK-ios.tar` and set the target library path to the iOS library at `dylib/iOS/libMoltenVK.dylib`.
3. OpenSSL source compilation in the original file targets desktop (using `Configure darwin64-x86_64` or `Configure darwin64-arm64` under macosx min version). Since cross-compiling OpenSSL for iOS requires iOS SDK path setups, it is better to search for OpenSSL using CMake's built-in `find_package(OpenSSL REQUIRED)`. This shifts the responsibility of locating pre-built iOS OpenSSL libraries to the environment/toolchain configuration.
4. Google Test (`googletest`) is not listed in the required dependencies list for building the iOS emulator binary, and is therefore removed.
5. All other required dependencies list: `capstone`, `dynarmic`, `fmt`, `spdlog`, `SDL3` (retained via the `sdl` subdirectory), `cubeb`, `glslang`, `SPIRV-Cross`, `pugixml`, `miniz`, `yaml-cpp`, `ffmpeg`, `xxHash`, `dlmalloc`, `libatrac9`, `libfat16`, `stb`, `ddspp`, `cppcommon`, `CLI11`, `vulkan` headers, `vma`, `psvpfstools`, and `ssl/crypto` can be cleanly retained. Unlisted helper targets/directories (such as `dirent`, `printf`, `glad`, `concurrentqueue`, `vita-toolchain`, `libcurl`, `tracy`, `substitute`) can be deleted to achieve the target clean file.

## 3. Caveats
- We assume that `OpenSSL` will be built/provided externally for the iOS target, and configured via standard CMake command line variables (e.g. `-DOPENSSL_ROOT_DIR=...` or toolchain variables).
- We assume that the iOS app packaging process will consume the dynamic library `libMoltenVK.dylib` located under `dylib/iOS/` within the extracted MoltenVK folder. If a static library or an `xcframework` is required instead, the target mapping in `vita3k/CMakeLists.txt` must be updated accordingly.

## 4. Conclusion
We proposed a clean `external/CMakeLists.txt` that:
- Retains only the 24 required dependencies.
- Replaces the macOS MoltenVK download with the iOS version (`MoltenVK-ios.tar` and `dylib/iOS/libMoltenVK.dylib`).
- Removes all Windows, Android, and Desktop-specific blocks (`discord-rpc`, `libadrenotools`, `winsock`, Vulkan validation layers download, `nativefiledialog-extended`, `googletest`, etc.).
- Replaces desktop OpenSSL build-from-source logic with a clean `find_package(OpenSSL REQUIRED)`.

## 5. Verification Method
To verify the clean `external/CMakeLists.txt`:
1. Save the proposed CMake code to `external/CMakeLists.txt`.
2. Clean the build directory.
3. Configure the CMake project targeting iOS (e.g. using a toolchain file or passing `-DCMAKE_SYSTEM_NAME=iOS` and target iOS SDK settings).
4. Run `cmake` generation and verify that it downloads the iOS MoltenVK package, configures the 24 required dependencies, and finishes configuration without requesting `discord-rpc`, `nativefiledialog-extended`, `googletest`, or attempting to compile OpenSSL from source.

## 6. Remaining Work
- Implement the proposed clean `external/CMakeLists.txt` file (to be completed by the Implementer agent).
- Verify the build with an iOS toolchain file (e.g. `ios.toolchain.cmake`) to ensure the configuration passes successfully.
