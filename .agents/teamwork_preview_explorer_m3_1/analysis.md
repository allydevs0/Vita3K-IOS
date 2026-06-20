# Analysis Report: external/CMakeLists.txt Cleanup for iOS Target

## Executive Summary
This report analyzes `external/CMakeLists.txt` at `C:\Users\denise\Downloads\Vita3K` and details the cleanup plan to target iOS exclusively. We identify and document the location of incompatible desktop and non-iOS dependencies (`discord-rpc`, `libadrenotools`, `winsock`, `MoltenVK-macos.tar`, Vulkan validation layers, `nativefiledialog-extended`, `googletest`, and desktop-specific OpenSSL build), explain how to transition MoltenVK for iOS, and propose a clean, iOS-exclusive `external/CMakeLists.txt`.

---

## 1. Identification of Removed/Modified Dependency Blocks

We identified the following blocks in the original `external/CMakeLists.txt` (lines 1 to 466) that are not needed for iOS or must be adjusted:

### A. Discord RPC (`discord-rpc`)
* **Lines**: 148–196
* **Details**: Downloads the Discord Game SDK zip file, extracts it, declares the static library `discord-rpc` compile target with all the source files, and sets up library links for Apple, Win32, and UNIX.
* **Why removed**: Desktop-only feature; Discord Rich Presence SDK is incompatible and unnecessary for iOS targets.

### B. Adreno Tools (`libadrenotools`)
* **Lines**: 111–118
* **Details**: Contains Android-specific compilation and definitions for `adrenotools` (GPU driver hooking/tweaks for Qualcomm Adreno GPUs).
* **Why removed**: Android-exclusive; incompatible and unnecessary on iOS.

### C. Winsock (`winsock`)
* **Lines**: 120–126
* **Details**: Windows-specific interface library linking `wsock32`, `ws2_32`, and `iphlpapi`.
* **Why removed**: Windows-exclusive; incompatible and unnecessary on iOS.

### D. MoltenVK macOS Tarball (`MoltenVK-macos.tar` Download)
* **Lines**: 229–240
* **Details**:
  ```cmake
  229: if(APPLE)
  230: 	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar")
  231: 		message(STATUS "Downloading MoltenVK...")
  232: 		file(DOWNLOAD https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-macos.tar
  233: 			"${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar" SHOW_PROGRESS)
  234: 	endif()
  235: 	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/MoltenVK")
  236: 		execute_process(COMMAND tar xf "${CMAKE_BINARY_DIR}/external/MoltenVK-macos.tar" --strip-components=1 MoltenVK/MoltenVK 
  237: 			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external")
  238: 	endif()
  239: 	target_include_directories(vulkan INTERFACE "${CMAKE_BINARY_DIR}/external/MoltenVK/include")
  240: 	set(MOLTENVK_DYLIB "${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/macOS/libMoltenVK.dylib" PARENT_SCOPE)
  ```
* **Adjustment for iOS**: Needs to download the iOS-specific build (`MoltenVK-ios.tar`) and link the iOS dynamic library.
  - **Download URL**: `https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-ios.tar`
  - **Library Destination**: `dylib/iOS/libMoltenVK.dylib`
  - **New Block**:
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

### E. Vulkan Validation Layers Download
* **Lines**: 241–261
* **Details**: Android-specific setup that downloads prebuilt Vulkan validation layers zip for Android and copies them to the target prebuilt directory.
* **Why removed**: Android-exclusive; incompatible and unnecessary on iOS.

### F. Native File Dialog Extended (`nativefiledialog-extended`)
* **Lines**: 459–466
* **Details**: Subdirectory configuration and alias declaration for the `nfd` library (`nativefiledialog-extended`).
* **Why removed**: Desktop file dialog library (specifically targeting Windows/macOS/Linux UI systems); incompatible and unnecessary on iOS.

### G. Google Test (`googletest`)
* **Lines**: 74–78
* **Details**: Compiles `gtest_main.cc` and `gtest-all.cc` to create static `googletest` target.
* **Why removed**: Unneeded for compiling the emulator binary for iOS; not in the required dependencies list.

### H. OpenSSL Build-From-Source for Desktop
* **Lines**: 273–355
* **Details**: Complex logic checking for Homebrew prefix paths on macOS, finding OpenSSL, or downloading and compiling OpenSSL from source zip (`openssl-3.6.1.zip` for MSVC or `openssl-3.3.2.tar.gz` for macOS). It uses custom configuring/make scripts with `lipo` tool chain calls.
* **Why removed/adjusted**: Building OpenSSL from source using desktop compiler steps (`darwin64-x86_64` / `darwin64-arm64`) fails or compiles for the host macOS architecture rather than iOS SDK. Instead, a clean, cross-compilation-friendly `find_package(OpenSSL REQUIRED)` should be used, relying on the environment or prebuilt dependencies (e.g. through the iOS toolchain/vcpkg/SDK root) to locate `ssl` and `crypto` libraries.

---

## 2. Proposed Clean `external/CMakeLists.txt`

Below is the proposed clean `external/CMakeLists.txt` designed to target iOS exclusively. It retains only the required dependencies: `capstone`, `dynarmic`, `fmt`, `spdlog`, `SDL3` (retained via the `sdl` subdirectory), `cubeb`, `glslang`, `SPIRV-Cross`, `pugixml`, `miniz`, `yaml-cpp`, `ffmpeg`, `xxHash`, `dlmalloc`, `libatrac9`, `libfat16`, `stb`, `ddspp`, `cppcommon`, `CLI11`, `vulkan` (configured with iOS MoltenVK), `vma`, `psvpfstools`, and `ssl/crypto` (linked via `find_package(OpenSSL REQUIRED)`).

```cmake
add_compile_options(-w)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

# Make option() in all external projects do nothing when a normal variable of the
# same name exists. The normal variable is not removed. The cache entry is not
# created or updated and is ignored if it exists.
set(CMAKE_POLICY_DEFAULT_CMP0077 "NEW")

# Helper function to check if submodules are checked out
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

# 1. Capstone
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(CAPSTONE_ARCHITECTURE_DEFAULT "Whether architectures are enabled by default" OFF)
option(CAPSTONE_ARM_SUPPORT "ARM support" ON)
set(CAPSTONE_BUILD_PACKAGING OFF CACHE BOOL "" FORCE)
add_subdirectory(capstone EXCLUDE_FROM_ALL)

# 2. fmt
set(FMT_INSTALL ON)
add_subdirectory(fmt)
set(fmt_DIR fmt)

# 3. spdlog
set(SPDLOG_WCHAR_FILENAMES ON CACHE BOOL "")
set(SPDLOG_NO_THREAD_ID ON CACHE BOOL "")
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "")
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

# 4. stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

# 5. ddspp
add_library(ddspp INTERFACE)
target_include_directories(ddspp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ddspp")

# 6. cppcommon
add_library(cppcommon STATIC CppCommon/source/cpu.cpp CppCommon/source/environment.cpp)
target_include_directories(cppcommon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/CppCommon")

# 7. libfat16
add_subdirectory(libfat16)

# 8. miniz
add_library(miniz STATIC miniz/miniz.c miniz/miniz.h)
target_include_directories(miniz PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/miniz")

# 9. SDL3 (maps to sdl directory)
add_subdirectory(sdl)

# 10. Cubeb
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

# 11. Dynarmic
set(DYNARMIC_TESTS OFF CACHE BOOL "")
set(DYNARMIC_NO_BUNDLED_FMT ON CACHE BOOL "")
set(DYNARMIC_FRONTENDS "A32" CACHE STRING "")
add_subdirectory(dynarmic)

# 12. yaml-cpp
option(YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
option(YAML_CPP_BUILD_CONTRIB "Enable contrib stuff in library" OFF)
add_subdirectory(yaml-cpp EXCLUDE_FROM_ALL)

# 13. glslang
option(BUILD_EXTERNAL "Build external dependencies in /External" OFF)
option(ENABLE_SPVREMAPPER "Enables building of SPVRemapper" OFF)
option(ENABLE_GLSLANG_BINARIES "Builds glslang and spirv-remap" OFF)
option(ENABLE_HLSL "Enables HLSL input support" OFF)
option(GLSLANG_TESTS "Enable glslang testing" OFF)
option(ENABLE_OPT "Enables spirv-opt capability if present" OFF)
add_subdirectory(glslang)

# 14. SPIRV-Cross
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

# 15. dlmalloc
add_library(dlmalloc STATIC "${CMAKE_CURRENT_SOURCE_DIR}/dlmalloc/dlmalloc.cc")
target_include_directories(dlmalloc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/dlmalloc")
target_compile_definitions(dlmalloc PUBLIC ONLY_MSPACES=1)
target_compile_definitions(dlmalloc PUBLIC USE_LOCK=0)

# 16. pugixml
add_subdirectory(pugixml)

# 17. CLI11
add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/cli11")

# 18. Vulkan Headers & iOS MoltenVK
add_library(vulkan INTERFACE)
target_include_directories(vulkan INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator-Hpp/Vulkan-Headers/include")

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

# 19. Vulkan Memory Allocator (vma)
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator-Hpp/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator-Hpp/VulkanMemoryAllocator/include")

# 20. FFmpeg
add_subdirectory(ffmpeg)

# 21. OpenSSL (ssl/crypto)
find_package(OpenSSL REQUIRED)
add_library(ssl INTERFACE)
target_link_libraries(ssl INTERFACE OpenSSL::SSL)
add_library(crypto INTERFACE)
target_link_libraries(crypto INTERFACE OpenSSL::Crypto)

# 22. psvpfstools (Must be after OpenSSL)
add_subdirectory(psvpfstools)

# 23. libatrac9
file(GLOB LIBATRAC9_SOURCES
	LibAtrac9/C/src/*.c
	LibAtrac9/C/src/*.h
)
add_library(libatrac9 STATIC ${LIBATRAC9_SOURCES})
target_include_directories(libatrac9 PUBLIC LibAtrac9/C/src)

# 24. xxHash
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(XXHASH_BUILD_XXHSUM "Build the xxhsum binary" OFF)
option(XXH_X86DISPATCH_ALLOW_AVX "Allow building XXH3 with AVX even if it crashes on SSE2-Only CPUs" OFF)
add_subdirectory(xxHash/cmake_unofficial EXCLUDE_FROM_ALL)
if(XXH_X86DISPATCH_ALLOW_AVX)
	target_compile_definitions(xxhash PRIVATE XXH_X86DISPATCH_ALLOW_AVX)
endif()
```
