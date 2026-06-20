# Handoff Report: Cleanup of external/CMakeLists.txt for iOS Target

## 1. Observation
- `external/CMakeLists.txt` is located at `C:\Users\denise\Downloads\Vita3K\external\CMakeLists.txt` and contains 466 lines of build instructions.
- Target library checks show specific platform setups:
  - **discord-rpc**: Lines 148–196 (`if(USE_DISCORD_RICH_PRESENCE)`) downloads `discord_game_sdk.zip` and builds for macOS, Windows, and Linux.
  - **libadrenotools**: Lines 111–119 (`if(ANDROID)`) adds `libadrenotools` for `arm64` Android ABI.
  - **winsock**: Lines 120–126 (`if(WIN32)`) configures WSOCK libraries for Windows.
  - **MoltenVK macOS**: Lines 229–240 (`if(APPLE)`) downloads `MoltenVK-macos.tar` and references macOS `libMoltenVK.dylib`.
  - **Vulkan Validation Layers**: Lines 241–261 (`elseif(ANDROID AND NOT (CMAKE_BUILD_TYPE STREQUAL "Release"))`) downloads `android-binaries` zip and copies them to the Android prebuilt directory.
  - **nativefiledialog-extended**: Lines 457–465 (`if(NOT ANDROID)`) adds `nativefiledialog-extended`.
  - **googletest**: Lines 74–78 adds googletest library from source files.
  - **OpenSSL Build-from-Source**: Lines 273–354 downloads `openssl-3.3.2.tar.gz` and compiles static libraries using macOS platform targets (`darwin64-arm64`, `darwin64-x86_64`).
- `vcpkg.json` at `C:\Users\denise\Downloads\Vita3K\vcpkg.json` lists OpenSSL as a package dependency:
  ```json
  "dependencies": [
    "boost-filesystem",
    "boost-system",
    "curl",
    "openssl",
    "zlib"
  ]
  ```

---

## 2. Logic Chain
1. *Observation*: The top-level `CMakeLists.txt` (line 4) enforces `set(CMAKE_SYSTEM_NAME iOS CACHE STRING "Target iOS" FORCE)`.
2. *Observation*: Libraries like `discord-rpc`, `libadrenotools`, `winsock`, and `nativefiledialog-extended` are designed exclusively for desktop environments (macOS/Windows/Linux) or Android systems and lack iOS mobile support or relevance.
3. *Observation*: Googletest targets testing executables that are not bundled inside production iOS application packages.
4. *Observation*: OpenSSL is specified as a direct dependency in `vcpkg.json`.
5. *Deduction*:
   - Since we are targeting iOS, the build configurations for `discord-rpc`, `libadrenotools`, `winsock`, `nativefiledialog-extended`, and `googletest` should be removed completely to optimize clean compile times and minimize configuration errors.
   - The custom macOS OpenSSL build script must be replaced with `find_package(OpenSSL REQUIRED)` because the vcpkg toolchain will compile the required `openssl` static libraries natively using the `arm64-ios` triplet.
   - The MoltenVK dependency download should point to the official `MoltenVK-ios.tar` package instead of `MoltenVK-macos.tar`, and set `MOLTENVK_DYLIB` to `dylib/iOS/libMoltenVK.dylib`.

---

## 3. Caveats
- Since we are in a read-only investigation environment, we have not run the actual CMake configure or compilation step with the proposed file.
- The existence of the `v1.2.11-artifacts/MoltenVK-ios.tar` release asset on GitHub is assumed based on the standard KhronosGroup MoltenVK release conventions. If the pipeline uses a unified/universal tar, the path structure must match.

---

## 4. Conclusion
- A clean `external/CMakeLists.txt` for the exclusive iOS target has been designed.
- It removes redundant and platform-incompatible dependencies: `discord-rpc`, `libadrenotools`, `winsock`, Android validation layers download, `nativefiledialog-extended`, `googletest`, and desktop-specific OpenSSL compilation blocks.
- It redirects the MoltenVK target to pull the iOS MoltenVK build and exports the iOS library path (`dylib/iOS/libMoltenVK.dylib`) via `MOLTENVK_DYLIB`.

---

## 5. Verification Method
- Independently verify the configuration by replacing the `external/CMakeLists.txt` with the proposed content in a write-enabled workspace and running the CMake configure command for the iOS toolchain:
  ```bash
  cmake -B build -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0
  ```
- Ensure that:
  - CMake successfully configures without trying to run macOS desktop configurations for OpenSSL.
  - The downloaded MoltenVK package targets the iOS path `dylib/iOS/libMoltenVK.dylib`.
  - Irrelevant subdirectories are not added to the build targets.
