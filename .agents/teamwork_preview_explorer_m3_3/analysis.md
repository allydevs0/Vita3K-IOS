# Analysis Report: Cleanup of external/CMakeLists.txt for iOS Target

## Executive Summary
This analysis details the clean-up plan for `external/CMakeLists.txt` targeting iOS exclusively. Desktop-only, Android-only, and test dependencies are identified for removal, and the MoltenVK dependency is updated to use the iOS build target instead of the macOS desktop version.

---

## 1. Identified Dependency Blocks for Clean-Up

The following table summarizes the status of targeted blocks and libraries in `external/CMakeLists.txt`:

| Library / Feature | Block Location (Lines) | Action | iOS Target Justification |
| :--- | :--- | :--- | :--- |
| **`discord-rpc`** | 148–196 | **Remove** | Discord Rich Presence is a desktop feature; Discord IPC and SDK do not apply to iOS mobile builds. |
| **`libadrenotools`** | 111–119 | **Remove** | Qualcomm Adreno GPU tuning tools are specific to Android devices; iOS devices use Apple Silicon GPUs. |
| **`winsock`** | 120–126 | **Remove** | Windows Sockets API (`wsock32`, `ws2_32`, `iphlpapi`) is strictly for Windows desktop targets. |
| **`MoltenVK-macos.tar`** | 229–240 | **Adjust** | Switch download from `MoltenVK-macos.tar` to `MoltenVK-ios.tar` and set the target library path to the iOS dynamic library. |
| **Vulkan Validation Layers** | 241–261 | **Remove** | Validation layers are downloaded specifically for Android (`.so` prebuilts) and are not used in iOS packaging. |
| **`nativefiledialog-extended`** | 457–465 | **Remove** | Standard desktop floating dialogs (GTK/Win32/Cocoa) are unsupported on iOS; iOS uses UIKit Document Picker. |
| **`googletest`** | 74–78 | **Remove** | Unit testing with standard desktop runner is not built or run in target device/simulator builds. |
| **OpenSSL build-from-source** | 273–354 | **Remove** | The custom script builds macOS OpenSSL using `darwin64-*` configurations and is incompatible with iOS SDK targets; we use `find_package(OpenSSL REQUIRED)` directly as vcpkg installs OpenSSL for iOS. |

---

## 2. Detailed Technical Breakdown & Code Snippets

### Discord RPC
- **Lines**: 148–196
- **Snippet**:
```cmake
148: if(USE_DISCORD_RICH_PRESENCE)
149: 	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/discord_game_sdk.zip")
150: 		message(STATUS "Downloading discord gamesdk...")
151: 		file(DOWNLOAD https://dl-game-sdk.discordapp.net/latest/discord_game_sdk.zip
...
196: endif()
```
- **Rationale**: Safe to remove; rich presence functionality has no iOS implementation.

### libadrenotools
- **Lines**: 111–119
- **Snippet**:
```cmake
111: if(ANDROID)
112: 	if(ARCHITECTURE STREQUAL "arm64")
113: 		add_subdirectory(libadrenotools)
...
118: endif()
```
- **Rationale**: Exclusively for Android/Adreno.

### winsock
- **Lines**: 120–126
- **Snippet**:
```cmake
120: if(WIN32)
121: 	add_library(winsock INTERFACE)
...
126: endif()
```
- **Rationale**: Exclusively for Windows.

### MoltenVK Adjustment
- **Lines**: 229–240
- **Existing (macOS) Setup**:
```cmake
229: 	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar")
230: 		message(STATUS "Downloading MoltenVK...")
231: 		file(DOWNLOAD https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-macos.tar
232: 			"${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar" SHOW_PROGRESS)
233: 	endif()
234: 	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK")
235: 		execute_process(COMMAND tar xf "${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar" --strip-components=1 MoltenVK/MoltenVK 
236: 			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external")
237: 	endif()
238: 	target_include_directories(vulkan INTERFACE "${CMAKE_BINARY_DIR}/external/MoltenVK/include")
239: 	set(MOLTENVK_DYLIB "${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/macOS/libMoltenVK.dylib" PARENT_SCOPE)
```
- **Proposed (iOS) Adjustment**:
  Change the tarball from `MoltenVK-macos.tar` to `MoltenVK-ios.tar`, extract it, and set `MOLTENVK_DYLIB` (or library path) to target the iOS dylib path inside the package:
```cmake
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK-ios.tar")
		message(STATUS "Downloading MoltenVK for iOS...")
		file(DOWNLOAD https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-ios.tar
			"${CMAKE_BINARY_DIR}/external/MoltenVK-ios.tar" SHOW_PROGRESS)
	endif()
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK")
		execute_process(COMMAND tar xf "${CMAKE_BINARY_DIR}/external/MoltenVK-ios.tar" --strip-components=1 MoltenVK/MoltenVK 
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external")
	endif()
	target_include_directories(vulkan INTERFACE "${CMAKE_BINARY_DIR}/external/MoltenVK/include")
	set(MOLTENVK_DYLIB "${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/iOS/libMoltenVK.dylib" PARENT_SCOPE)
```

### Vulkan Validation Layers
- **Lines**: 241–261
- **Snippet**:
```cmake
241: elseif(ANDROID AND NOT (CMAKE_BUILD_TYPE STREQUAL "Release"))
...
261: endif()
```
- **Rationale**: Exclusively targets Android `.so` prebuilts.

### nativefiledialog-extended
- **Lines**: 457–465
- **Snippet**:
```cmake
457: set(NFD_PORTAL ON)
458: 
459: if(NOT ANDROID)
460: 	# nativefiledialog-extended
461: 	add_subdirectory(nativefiledialog-extended)
...
465: endif()
```
- **Rationale**: Desktop-focused file pickers (unsupported on iOS).

### googletest
- **Lines**: 74–78
- **Snippet**:
```cmake
74: add_library(googletest STATIC googletest/googletest/src/gtest_main.cc googletest/googletest/src/gtest-all.cc)
...
```
- **Rationale**: Testing suite runner not required for production iOS builds.

### OpenSSL Build-from-Source for Desktop
- **Lines**: 273–354
- **Snippet**:
```cmake
273: if(APPLE OR NOT FORCE_BUILD_OPENSSL_MAC)
...
354: target_link_libraries(crypto INTERFACE OpenSSL::Crypto)
```
- **Rationale**: This block tries to build desktop OpenSSL with Darwin macOS configurations (`darwin64-arm64`, `darwin64-x86_64`) and lipo tools. We remove this custom compilation. Since `openssl` is declared in `vcpkg.json`, vcpkg natively handles building OpenSSL static libraries for the iOS triplet (`arm64-ios`). Thus, we replace the entire block with `find_package(OpenSSL REQUIRED)` and define interface targets `ssl` and `crypto` that link to `OpenSSL::SSL` and `OpenSSL::Crypto` respectively.

---

## 3. Proposed iOS-Exclusive `external/CMakeLists.txt`

The following proposed clean file contains only the required dependencies: `capstone`, `dynarmic`, `fmt`, `spdlog`, `SDL3`, `cubeb`, `glslang`, `SPIRV-Cross`, `pugixml`, `miniz`, `yaml-cpp`, `ffmpeg`, `xxHash`, `dlmalloc`, `libatrac9`, `libfat16`, `stb`, `ddspp`, `cppcommon`, `CLI11`, `vulkan` headers, `vma`, `psvpfstools`, and `ssl/crypto`.

```cmake
add_compile_options(-w)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

# Make option() in all external projects do nothing when a normal variable of the
# same name exists. The normal variable is not removed. The cache entry is not
# created or updated and is ignored if it exists.
set(CMAKE_POLICY_DEFAULT_CMP0077 "NEW")

function(check_submodules_present)
	file(READ "${CMAKE_SOURCE_DIR}/.gitmodules" gitmodules)
	string(REGEX MATCHALL "path *= *[^ \t\r\n]*" gitmodules ${gitmodules})

	foreach(module ${gitmodules})
		# Get module name
		string(REGEX REPLACE "path *= *" "" module ${module})

		# Stat the folder and get amount of entries
		file(GLOB RESULT "${CMAKE_SOURCE_DIR}/${module}/*")
		list(LENGTH RESULT RES_LEN)

		# If the module has no files, bring fatal error
		if(RES_LEN EQUAL 0)
			# directory is empty
			message(FATAL_ERROR "Submodule ${module} is empty. "
				"Please run: git submodule update --init --recursive")
		endif()
	endforeach()
endfunction()

check_submodules_present()

# Set folder for external projects (for project tree in IDE)
set(CMAKE_FOLDER externals)

option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(CAPSTONE_ARCHITECTURE_DEFAULT "Whether architectures are enabled by default" OFF)
option(CAPSTONE_ARM_SUPPORT "ARM support" ON)
set(CAPSTONE_BUILD_PACKAGING OFF CACHE BOOL "" FORCE)
add_subdirectory(capstone EXCLUDE_FROM_ALL)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

add_library(ddspp INTERFACE)
target_include_directories(ddspp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ddspp")

add_library(cppcommon STATIC CppCommon/source/cpu.cpp CppCommon/source/environment.cpp)
target_include_directories(cppcommon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/CppCommon")

set(FMT_INSTALL ON)
add_subdirectory(fmt)
set(fmt_DIR fmt)

set(SPDLOG_WCHAR_FILENAMES ON CACHE BOOL "")
set(SPDLOG_NO_THREAD_ID ON CACHE BOOL "")
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "")
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

add_subdirectory(libfat16)

add_library(miniz STATIC miniz/miniz.c miniz/miniz.h)
target_include_directories(miniz PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/miniz")

add_subdirectory(sdl)

# Cubeb setup, from https://github.com/RPCS3/rpcs3/blob/master/3rdparty/cubeb/CMakeLists.txt
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "Don't build shared libs")
set(BUILD_TESTS FALSE CACHE BOOL "Don't build tests")
set(BUILD_RUST_LIBS FALSE CACHE BOOL "Don't build rust libs")
set(BUILD_TOOLS FALSE CACHE BOOL "Don't build tools")
set(BUNDLE_SPEEX TRUE CACHE BOOL "Bundle the speex library")
set(LAZY_LOAD_LIBS TRUE CACHE BOOL "Lazily load shared libraries")
set(USE_SANITIZERS FALSE CACHE BOOL "Dont't use sanitizers")

add_subdirectory(cubeb EXCLUDE_FROM_ALL)

if (ARCHITECTURE STREQUAL "arm64")
	target_compile_definitions(speex PUBLIC
		#_USE_NEON
	)
elseif (ARCHITECTURE STREQUAL "x86_64")
	target_compile_definitions(speex PUBLIC
		_USE_SSE
		_USE_SSE2
	)
endif ()

set(DYNARMIC_TESTS OFF CACHE BOOL "")
set(DYNARMIC_NO_BUNDLED_FMT ON CACHE BOOL "")
set(DYNARMIC_FRONTENDS "A32" CACHE STRING "")
add_subdirectory(dynarmic)

option(YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
option(YAML_CPP_BUILD_CONTRIB "Enable contrib stuff in library" OFF)
add_subdirectory(yaml-cpp EXCLUDE_FROM_ALL)

option(BUILD_EXTERNAL "Build external dependencies in /External" OFF)
option(ENABLE_SPVREMAPPER "Enables building of SPVRemapper" OFF)
option(ENABLE_GLSLANG_BINARIES "Builds glslang and spirv-remap" OFF)
option(ENABLE_HLSL "Enables HLSL input support" OFF)
option(GLSLANG_TESTS "Enable glslang testing" OFF)
option(ENABLE_OPT "Enables spirv-opt capability if present" OFF)
add_subdirectory(glslang)

option(SPIRV_CROSS_CLI "Build the CLI binary. Requires SPIRV_CROSS_STATIC." OFF)
option(SPIRV_CROSS_ENABLE_TESTS "Enable SPIRV-Cross tests." OFF)
option(SPIRV_CROSS_ENABLE_HLSL "Enable HLSL target support." OFF)
option(SPIRV_CROSS_ENABLE_MSL "Enable MSL target support." OFF)
option(SPIRV_CROSS_ENABLE_CPP "Enable C++ target support." OFF)
option(SPIRV_CROSS_ENABLE_REFLECT "Enable JSON reflection target support." OFF)
option(SPIRV_CROSS_ENABLE_C_API "Enable C API wrapper support in static library." OFF)
option(SPIRV_CROSS_ENABLE_UTIL "Enable util module support." OFF)
option(SPIRV_CROSS_SKIP_INSTALL "Skips installation targets." ON)
add_subdirectory(SPIRV-Cross)

add_library(dlmalloc STATIC "${CMAKE_CURRENT_SOURCE_DIR}/dlmalloc/dlmalloc.cc")
target_include_directories(dlmalloc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/dlmalloc")
target_compile_definitions(dlmalloc PUBLIC ONLY_MSPACES=1)
target_compile_definitions(dlmalloc PUBLIC USE_LOCK=0)

add_subdirectory(pugixml)

add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/cli11")

add_library(vulkan INTERFACE)
target_include_directories(vulkan INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator-Hpp/Vulkan-Headers/include")
if(APPLE)
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK-ios.tar")
		message(STATUS "Downloading MoltenVK for iOS...")
		file(DOWNLOAD https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-ios.tar
			"${CMAKE_BINARY_DIR}/external/MoltenVK-ios.tar" SHOW_PROGRESS)
	endif()
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK")
		execute_process(COMMAND tar xf "${CMAKE_BINARY_DIR}/external/MoltenVK-ios.tar" --strip-components=1 MoltenVK/MoltenVK 
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external")
	endif()
	target_include_directories(vulkan INTERFACE "${CMAKE_BINARY_DIR}/external/MoltenVK/include")
	set(MOLTENVK_DYLIB "${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/iOS/libMoltenVK.dylib" PARENT_SCOPE)
endif()

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator-Hpp/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator-Hpp/VulkanMemoryAllocator/include")

add_subdirectory(ffmpeg)

find_package(OpenSSL REQUIRED)
message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")

add_library(ssl INTERFACE)
target_link_libraries(ssl INTERFACE OpenSSL::SSL)
add_library(crypto INTERFACE)
target_link_libraries(crypto INTERFACE OpenSSL::Crypto)

# Must be after openssl
add_subdirectory(psvpfstools)

file(GLOB LIBATRAC9_SOURCES
	LibAtrac9/C/src/*.c
	LibAtrac9/C/src/*.h
)
add_library(libatrac9 STATIC ${LIBATRAC9_SOURCES})
target_include_directories(libatrac9 PUBLIC LibAtrac9/C/src)

option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(XXHASH_BUILD_XXHSUM "Build the xxhsum binary" OFF)
option(XXH_X86DISPATCH_ALLOW_AVX "Allow building XXH3 with AVX even if it crashes on SSE2-Only CPUs" OFF)
if((NOT APPLE) AND (ARCHITECTURE STREQUAL "x86_64"))
	set(DISPATCH 1)
endif()
add_subdirectory(xxHash/cmake_unofficial EXCLUDE_FROM_ALL)
if(XXH_X86DISPATCH_ALLOW_AVX)
	target_compile_definitions(xxhash PRIVATE XXH_X86DISPATCH_ALLOW_AVX)
endif()
```
