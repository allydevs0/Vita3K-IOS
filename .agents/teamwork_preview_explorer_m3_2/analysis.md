# Analysis Report: external/CMakeLists.txt Cleanup for iOS Target

## Summary of Core Findings
This report details the cleanup of `external/CMakeLists.txt` to transition Vita3K to target iOS exclusively. We have identified all desktop and non-iOS dependencies (such as Discord RPC, nativefiledialog-extended, and Winsock) for removal, modified MoltenVK to download and use the iOS library instead of the macOS desktop version, and designed a proposed iOS-exclusive CMakeLists containing only the 24 required dependencies.

---

## 1. Identification of Targets/Blocks to Remove or Adjust

Below is a detailed analysis of the blocks, add_subdirectory commands, download instructions, and link setups for the components targeted for cleanup.

### 1.1. `discord-rpc`
* **File & Lines**: `external/CMakeLists.txt`, lines 148-196
* **Original Code Block**:
  ```cmake
  if(USE_DISCORD_RICH_PRESENCE)
  	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/discord_game_sdk.zip")
  		message(STATUS "Downloading discord gamesdk...")
  		file(DOWNLOAD https://dl-game-sdk.discordapp.net/latest/discord_game_sdk.zip
  			"${CMAKE_BINARY_DIR}/external/discord_game_sdk.zip" SHOW_PROGRESS)
  	endif()

  	if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/discord_game_sdk")
  		file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/external/discord_game_sdk")
  		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_BINARY_DIR}/external/discord_game_sdk.zip"
  			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/discord_game_sdk")
  		file(RENAME "${CMAKE_BINARY_DIR}/external/discord_game_sdk/lib/x86_64/discord_game_sdk.so"
  			"${CMAKE_BINARY_DIR}/external/discord_game_sdk/lib/x86_64/libdiscord_game_sdk.so")
  	endif()

  	add_library(discord-rpc STATIC
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/achievement_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/activity_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/application_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/core.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/image_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/lobby_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/network_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/overlay_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/relationship_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/storage_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/store_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/types.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/user_manager.cpp
  		${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp/voice_manager.cpp)

  	if(APPLE)
  		if(ARCHITECTURE STREQUAL "x86_64")
  			target_link_libraries(discord-rpc PUBLIC "${CMAKE_BINARY_DIR}/external/discord_game_sdk/lib/x86_64/discord_game_sdk.dylib")
  		elseif(ARCHITECTURE STREQUAL "arm64")
  			target_link_libraries(discord-rpc PUBLIC "${CMAKE_BINARY_DIR}/external/discord_game_sdk/lib/aarch64/discord_game_sdk.dylib")
  		else()
  			execute_process(COMMAND lipo -create -output discord_game_sdk.dylib x86_64/discord_game_sdk.dylib aarch64/discord_game_sdk.dylib
  			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/discord_game_sdk/lib")
  			target_link_libraries(discord-rpc PUBLIC "${CMAKE_BINARY_DIR}/external/discord_game_sdk/lib/discord_game_sdk.dylib")
  		endif()
  	elseif(WIN32)
  		target_link_libraries(discord-rpc PUBLIC "${CMAKE_BINARY_DIR}/external/discord_game_sdk/lib/x86_64/discord_game_sdk.dll.lib")
  	elseif(UNIX)
  		target_link_libraries(discord-rpc PUBLIC "${CMAKE_BINARY_DIR}/external/discord_game_sdk/lib/x86_64/libdiscord_game_sdk.so")
  	endif()

  	target_include_directories(discord-rpc PUBLIC "${CMAKE_BINARY_DIR}/external/discord_game_sdk/cpp")
  endif()
  ```
* **Rationale**: Discord Rich Presence is a desktop feature and has no native support / library usage on iOS platform.

### 1.2. `libadrenotools`
* **File & Lines**: `external/CMakeLists.txt`, lines 111-118
* **Original Code Block**:
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
* **Rationale**: Adrenotools is specifically designed for Qualcomm Adreno GPUs (Android only). iOS devices run Apple Silicon GPUs and do not use Adreno drivers.

### 1.3. `winsock`
* **File & Lines**: `external/CMakeLists.txt`, lines 120-126
* **Original Code Block**:
  ```cmake
  if(WIN32)
  	add_library(winsock INTERFACE)
  	find_library(WSOCK32 wsock32)
  	find_library(WS2_32 ws2_32)
  	find_library(IPHLPAPI iphlpapi)
  	target_link_libraries(winsock INTERFACE WSOCK32 WS2_32 IPHLPAPI)
  endif()
  ```
* **Rationale**: Winsock is the Windows Sockets API and is only applicable for Windows targets.

### 1.4. `MoltenVK-macos.tar` Download & Setup
* **File & Lines**: `external/CMakeLists.txt`, lines 229-240
* **Original Code Block**:
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
* **Adaptation Plan**: 
  - Change URL target from `MoltenVK-macos.tar` to `MoltenVK-ios.tar` (specifically, `https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.11-artifacts/MoltenVK-ios.tar`).
  - Extract `MoltenVK-ios.tar` inside `external`.
  - Check the active target platform (iOS Simulator or iOS Device) via `CMAKE_OSX_SYSROOT` to point `MOLTENVK_DYLIB` to the proper `.dylib` path:
    - iOS Device: `${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/iOS/libMoltenVK.dylib`
    - iOS Simulator: `${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/iOS-Simulator/libMoltenVK.dylib`

### 1.5. Vulkan Validation Layers Download
* **File & Lines**: `external/CMakeLists.txt`, lines 241-261
* **Original Code Block**:
  ```cmake
  elseif(ANDROID AND NOT (CMAKE_BUILD_TYPE STREQUAL "Release"))
  	# add validation layer
  	if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../android/prebuilt/${ANDROID_ABI}/libVkLayer_khronos_validation.so")
  		set(VALIDATION_BINARY "android-binaries-1.4.341.0")

  		# download it if not already done
  		if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/validation_layers.zip")
  			message(STATUS "Downloading validation layer...")
  			file(DOWNLOAD https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases/download/vulkan-sdk-1.4.341.0/${VALIDATION_BINARY}.zip
  				"${CMAKE_BINARY_DIR}/external/validation_layers.zip" SHOW_PROGRESS)
  		endif()

  		if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/validation_layers")
  			file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/external/validation_layers")
  			execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_BINARY_DIR}/external/validation_layers.zip"
  				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external/validation_layers")
  		endif()

  		file(COPY "${CMAKE_BINARY_DIR}/external/validation_layers/${VALIDATION_BINARY}/${ANDROID_ABI}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../android/prebuilt")
  	endif()
  endif()
  ```
* **Rationale**: This download fetches prebuilt Android Vulkan validation layers (`libVkLayer_khronos_validation.so`) and is not useful for iOS builds.

### 1.6. `nativefiledialog-extended`
* **File & Lines**: `external/CMakeLists.txt`, lines 459-465
* **Original Code Block**:
  ```cmake
  if(NOT ANDROID)
  	# nativefiledialog-extended
  	add_subdirectory(nativefiledialog-extended)

  	# Create alias to prevent the need of multiple changes in case the target name changes
  	add_library(NFDe::NFDe ALIAS nfd)
  endif()
  ```
* **Rationale**: This is a desktop file dialog component (designed for Windows, macOS, and Linux). iOS applications run in a sandbox and use system/UIKit components for document interaction, not nativefiledialog-extended.

### 1.7. `googletest`
* **File & Lines**: `external/CMakeLists.txt`, lines 74-78
* **Original Code Block**:
  ```cmake
  add_library(googletest STATIC googletest/googletest/src/gtest_main.cc googletest/googletest/src/gtest-all.cc)
  target_include_directories(googletest PUBLIC googletest/googletest/include)
  target_include_directories(googletest PRIVATE googletest/googletest)
  target_compile_definitions(googletest PUBLIC GTEST_HAS_PTHREAD=0)
  ```
* **Rationale**: GoogleTest is used for test executables. Vita3K does not build/run unit tests on the iOS target architecture.

### 1.8. OpenSSL Build-From-Source for Desktop
* **File & Lines**: `external/CMakeLists.txt`, lines 273-346
* **Original Code Block**:
  ```cmake
  if(APPLE OR NOT FORCE_BUILD_OPENSSL_MAC)
  	execute_process(
  		COMMAND brew --prefix openssl
  		RESULT_VARIABLE BREW_RESULT
  		OUTPUT_VARIABLE BREW_OPENSSL
  		OUTPUT_STRIP_TRAILING_WHITESPACE
  	)
  	if(BREW_RESULT EQUAL 0)
  		set(OPENSSL_ROOT_DIR "${BREW_OPENSSL}")
  		set(OPENSSL_USE_STATIC_LIBS TRUE)
  	endif()
  endif()

  if(NOT FORCE_BUILD_OPENSSL_MAC)
  	find_package(OpenSSL QUIET)
  endif()

  if(NOT OPENSSL_FOUND OR FORCE_BUILD_OPENSSL_MAC)
  	if(MSVC)
  ...
  	elseif(APPLE)
  		message(STATUS "OpenSSL not found.")
  		if(NOT EXISTS "${CMAKE_BINARY_DIR}/external/openssl.tar.gz")
  			message(STATUS "Downloading OpenSSL@3 from source...")
  			file(DOWNLOAD https://github.com/openssl/openssl/releases/download/openssl-3.3.2/openssl-3.3.2.tar.gz
  				"${CMAKE_BINARY_DIR}/external/openssl.tar.gz" SHOW_PROGRESS)
  		endif()
  ...
  		set(OPENSSL_ROOT_DIR "${CMAKE_BINARY_DIR}/external/openssl")
  		set(OPENSSL_USE_STATIC_LIBS TRUE)
  	endif()
  	find_package(OpenSSL REQUIRED)
  endif()
  ```
* **Rationale**: This block attempts to find OpenSSL via Homebrew (for macOS) or manually download and build OpenSSL from source via host Perl configurations (configured specifically for desktop macOS `darwin64-x86_64`/`darwin64-arm64` and MSVC `openssl-3.6.1.zip`). For iOS, OpenSSL is found/provided by the target toolchain, and cross-compilation configurations should manage OpenSSL without attempting host compilation scripts.

---

## 2. Propose a Clean `external/CMakeLists.txt`

The proposed `external/CMakeLists.txt` retains ONLY the 24 required dependencies, adjusts MoltenVK for iOS devices and simulators, and removes all unnecessary build targets and platform-specific checks.

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

# ==========================================
# 1. Capstone
# ==========================================
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(CAPSTONE_ARCHITECTURE_DEFAULT "Whether architectures are enabled by default" OFF)
option(CAPSTONE_ARM_SUPPORT "ARM support" ON)
set(CAPSTONE_BUILD_PACKAGING OFF CACHE BOOL "" FORCE)
add_subdirectory(capstone EXCLUDE_FROM_ALL)

# ==========================================
# 2. fmt
# ==========================================
set(FMT_INSTALL ON)
add_subdirectory(fmt)
set(fmt_DIR fmt)

# ==========================================
# 3. spdlog
# ==========================================
set(SPDLOG_WCHAR_FILENAMES ON CACHE BOOL "")
set(SPDLOG_NO_THREAD_ID ON CACHE BOOL "")
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "")
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

# ==========================================
# 4. stb
# ==========================================
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

# ==========================================
# 5. ddspp
# ==========================================
add_library(ddspp INTERFACE)
target_include_directories(ddspp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ddspp")

# ==========================================
# 6. cppcommon
# ==========================================
add_library(cppcommon STATIC CppCommon/source/cpu.cpp CppCommon/source/environment.cpp)
target_include_directories(cppcommon PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/CppCommon")

# ==========================================
# 7. libfat16
# ==========================================
add_subdirectory(libfat16)

# ==========================================
# 8. miniz
# ==========================================
add_library(miniz STATIC miniz/miniz.c miniz/miniz.h)
target_include_directories(miniz PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/miniz")

# ==========================================
# 9. SDL3 (sdl)
# ==========================================
add_subdirectory(sdl)

# ==========================================
# 10. cubeb
# ==========================================
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

# ==========================================
# 11. dynarmic
# ==========================================
set(DYNARMIC_TESTS OFF CACHE BOOL "")
set(DYNARMIC_NO_BUNDLED_FMT ON CACHE BOOL "")
set(DYNARMIC_FRONTENDS "A32" CACHE STRING "")
add_subdirectory(dynarmic)

# ==========================================
# 12. yaml-cpp
# ==========================================
option(YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
option(YAML_CPP_BUILD_CONTRIB "Enable contrib stuff in library" OFF)
add_subdirectory(yaml-cpp EXCLUDE_FROM_ALL)

# ==========================================
# 13. glslang
# ==========================================
option(BUILD_EXTERNAL "Build external dependencies in /External" OFF)
option(ENABLE_SPVREMAPPER "Enables building of SPVRemapper" OFF)
option(ENABLE_GLSLANG_BINARIES "Builds glslang and spirv-remap" OFF)
option(ENABLE_HLSL "Enables HLSL input support" OFF)
option(GLSLANG_TESTS "Enable glslang testing" OFF)
option(ENABLE_OPT "Enables spirv-opt capability if present" OFF)
add_subdirectory(glslang)

# ==========================================
# 14. SPIRV-Cross
# ==========================================
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

# ==========================================
# 15. dlmalloc
# ==========================================
add_library(dlmalloc STATIC "${CMAKE_CURRENT_SOURCE_DIR}/dlmalloc/dlmalloc.cc")
target_include_directories(dlmalloc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/dlmalloc")
target_compile_definitions(dlmalloc PUBLIC ONLY_MSPACES=1)
target_compile_definitions(dlmalloc PUBLIC USE_LOCK=0)

# ==========================================
# 16. pugixml
# ==========================================
add_subdirectory(pugixml)

# ==========================================
# 17. CLI11
# ==========================================
add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/cli11")

# ==========================================
# 18. vulkan (headers) & MoltenVK for iOS
# ==========================================
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
	
	# Determine if we are building for iOS Simulator or iOS Device target
	if(CMAKE_OSX_SYSROOT MATCHES "Simulator")
		set(MOLTENVK_DYLIB "${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/iOS-Simulator/libMoltenVK.dylib" PARENT_SCOPE)
	else()
		set(MOLTENVK_DYLIB "${CMAKE_BINARY_DIR}/external/MoltenVK/dylib/iOS/libMoltenVK.dylib" PARENT_SCOPE)
	endif()
endif()

# ==========================================
# 19. vma (Vulkan Memory Allocator)
# ==========================================
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE
	"${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator-Hpp/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator-Hpp/VulkanMemoryAllocator/include")

# ==========================================
# 20. ffmpeg
# ==========================================
add_subdirectory(ffmpeg)

# ==========================================
# 21. OpenSSL (ssl/crypto)
# ==========================================
find_package(OpenSSL REQUIRED)

add_library(ssl INTERFACE)
target_link_libraries(ssl INTERFACE OpenSSL::SSL)

add_library(crypto INTERFACE)
target_link_libraries(crypto INTERFACE OpenSSL::Crypto)

# ==========================================
# 22. psvpfstools
# ==========================================
add_subdirectory(psvpfstools)

# ==========================================
# 23. libatrac9
# ==========================================
file(GLOB LIBATRAC9_SOURCES
	LibAtrac9/C/src/*.c
	LibAtrac9/C/src/*.h
)
add_library(libatrac9 STATIC ${LIBATRAC9_SOURCES})
target_include_directories(libatrac9 PUBLIC LibAtrac9/C/src)

# ==========================================
# 24. xxHash
# ==========================================
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(XXHASH_BUILD_XXHSUM "Build the xxhsum binary" OFF)
add_subdirectory(xxHash/cmake_unofficial EXCLUDE_FROM_ALL)
```
