# Handoff & Forensic Audit Report - Milestone 2

## 1. Observation
### File Paths Checked
- Root `vcpkg.json` at: `C:\Users\denise\Downloads\Vita3K\vcpkg.json`
- Root `CMakeLists.txt` at: `C:\Users\denise\Downloads\Vita3K\CMakeLists.txt`

### Observations on `vcpkg.json`
Verbatim content of `vcpkg.json`:
```json
{
  "name": "vita3k",
  "version-string": "0.0.0",
  "builtin-baseline": "77df67cfff9c12ccfdb52284e07c87c75092f723",
  "dependencies": [
    "boost-filesystem",
    "boost-system",
    "curl",
    "openssl",
    "zlib"
  ]
}
```
Only the five listed dependencies are present.

### Observations on `CMakeLists.txt`
Verbatim content of `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.22)

# Configure iOS exclusively as the target
set(CMAKE_SYSTEM_NAME iOS CACHE STRING "Target iOS" FORCE)
set(VITA3K_BUILD_IOS ON CACHE BOOL "Build iOS" FORCE)

project(Vita3K)

# Detects the amount of processors of the host machine and forwards the result to CPU_COUNT
include(ProcessorCount)
ProcessorCount(CPU_COUNT)

# Define the Architecture variable, right now it should only contain "x86_64" or "arm64"
include("external/dynarmic/CMakeModules/DetectArchitecture.cmake")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)

# Enable Objective-C and Swift for the iOS build
enable_language(OBJC)
enable_language(Swift)

option(FORCE_BUILD_OPENSSL_MAC OFF)

if("${CMAKE_CXX_COMPILER_LAUNCHER}" STREQUAL "")
	find_program(CCACHE_PROGRAM ccache)
	if(CCACHE_PROGRAM)
		set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
	endif()
endif()

enable_testing()

cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(lto_usage_mode ALWAYS RELEASE_ONLY NEVER)
set(USE_LTO RELEASE_ONLY CACHE STRING "Use interprocedural optimization/link time optimization")
set_property(CACHE USE_LTO PROPERTY STRINGS ${lto_usage_mode})

if (NOT (USE_LTO STREQUAL "NEVER"))
	include(CheckIPOSupported)
	check_ipo_supported(RESULT ipo_supported OUTPUT ipo_supported_error)

	if( ipo_supported )
		if (USE_LTO STREQUAL "ALWAYS")
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
		elseif (USE_LTO STREQUAL "RELEASE_ONLY")
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
		endif()
	else()
		message(STATUS "IPO / LTO not supported: <${ipo_supported_error}>")
		set(USE_LTO NEVER CACHE STRING "Use interprocedural optimization/link time optimization")
	endif()
endif()


############################
########## Boost ###########
############################

# Find standard Boost package from vcpkg
find_package(Boost REQUIRED COMPONENTS filesystem)

add_subdirectory(external)
add_subdirectory(vita3k)
add_subdirectory(tools/gen-modules)
```

- Target configuration forced to iOS: `set(CMAKE_SYSTEM_NAME iOS CACHE STRING "Target iOS" FORCE)` and `set(VITA3K_BUILD_IOS ON CACHE BOOL "Build iOS" FORCE)` are set.
- Objective-C and Swift support enabled: `enable_language(OBJC)` and `enable_language(Swift)`.
- All `ANDROID`, `WIN32`, `MSVC`, AppImage, and Discord Rich Presence blocks/options have been completely removed.
- Custom desktop Boost compilation logic replaced with standard search: `find_package(Boost REQUIRED COMPONENTS filesystem)`.

---

## 2. Logic Chain
1. **Dependency Check**: Reading the content of `vcpkg.json` shows the list of dependencies contains exactly `boost-filesystem`, `boost-system`, `curl`, `openssl`, and `zlib`. No extra libraries are defined, and none of the requested ones are missing.
2. **Platform & Build Settings Check**: Checking `CMakeLists.txt` lines 4-5 and lines 22-23 verifies that `CMAKE_SYSTEM_NAME` is forced to `iOS`, `VITA3K_BUILD_IOS` is forced to `ON`, and Objective-C and Swift languages are enabled.
3. **Redundancy/Cleanup Check**: Grep searches for `ANDROID`, `WIN32`, `MSVC`, `AppImage`, and `Discord`/`Presence` in `CMakeLists.txt` returned no hits. The git diff confirms their complete elimination.
4. **Boost Search Check**: Inspection of `CMakeLists.txt` lines 65-66 verifies that the custom custom `get_boost` compilation logic was completely removed and replaced with a standard `find_package` for Boost.
5. **No Cheating or Bypasses**: The implementation logic in `CMakeLists.txt` matches standard project structures, delegates execution directly to project directories (`external`, `vita3k`, `tools/gen-modules`), and uses standard CMake syntax. No facades, dummy conditions, or execution bypasses are present.

---

## 3. Caveats
- No local compilation or test execution could be performed due to the lack of CMake and the iOS SDK/toolchain in the local environment, but syntax validity and structures have been verified manually and via the git history/CI artifacts.

---

## 4. Conclusion

### Forensic Audit Report

**Work Product**: Root `CMakeLists.txt` and `vcpkg.json` at `C:\Users\denise\Downloads\Vita3K`
**Profile**: General Project
**Verdict**: CLEAN

### Phase Results
- **vcpkg.json Dependency check**: PASS — Exactly the required dependencies exist.
- **Forced iOS target settings**: PASS — CMAKE_SYSTEM_NAME and VITA3K_BUILD_IOS configured as requested.
- **Language support**: PASS — OBJC and Swift languages enabled.
- **Desktop/Android blocks removal**: PASS — All blocks containing ANDROID, WIN32, MSVC, AppImage, and Discord Rich Presence were completely removed.
- **Boost compilation logic cleanup**: PASS — Replaced custom desktop Boost build macro/logic with standard component find search.
- **Bypass / Facade detection**: PASS — No facade code, hardcoded output, or bypass configurations exist.

---

## 5. Verification Method
1. Inspect the root `vcpkg.json` file to confirm only the specified list of dependencies is present.
2. Inspect the root `CMakeLists.txt` file to verify platform configurations (iOS target, OBJC/Swift languages, and Boost component find search).
3. Run `git diff HEAD~1` (or relative comparison branches) to verify that no remnants of `ANDROID`, `WIN32`, `MSVC`, `AppImage`, or `Discord` remain in the root build file.
