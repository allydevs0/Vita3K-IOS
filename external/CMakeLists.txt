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
