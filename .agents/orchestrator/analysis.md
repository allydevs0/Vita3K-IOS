# Deletion & Refactoring Synthesis Report: Milestone 3

## Consensus
All three Explorer subagents analyzed `external/CMakeLists.txt` and agree on the following:
1. **Remove Desktop/Android Dependencies**:
   - `discord-rpc`
   - `libadrenotools`
   - `winsock`
   - Vulkan validation layers download
   - `nativefiledialog-extended`
   - `googletest`
   - Custom OpenSSL build-from-source block
2. **MoltenVK adjustment**:
   - Download iOS-specific MoltenVK build: `MoltenVK-ios.tar` from KhronosGroup.
   - Configure dylib target path to target iOS.
3. **OpenSSL lookup**:
   - Rely on `find_package(OpenSSL REQUIRED)` since OpenSSL is handled by vcpkg.

## Resolved Conflicts / Additional Optimization
Explorer 2 identified a crucial refinement for MoltenVK targeting:
- **Simulator Compatibility**: Dynamic selection of MoltenVK dylib path depending on whether the target is iOS Device (`dylib/iOS/libMoltenVK.dylib`) or iOS Simulator (`dylib/iOS-Simulator/libMoltenVK.dylib`) by matching `CMAKE_OSX_SYSROOT` against "Simulator". This will be adopted in the implementation to prevent build breaks in simulator environments.

## Synthesis Plan
The worker subagent will be instructed to overwrite `external/CMakeLists.txt` with the proposed configuration from Explorer 2's report.
