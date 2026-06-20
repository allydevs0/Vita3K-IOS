## 2026-06-20T03:44:57Z
You are the worker agent for Milestone 1. Your working directory is C:\Users\denise\Downloads\Vita3K\.agents\teamwork_preview_worker_m1\. Your identity is worker_m1.

MANDATORY INTEGRITY WARNING:
> DO NOT CHEAT. All implementations must be genuine. DO NOT
> hardcode test results, create dummy/facade implementations, or
> circumvent the intended task. A Forensic Auditor will independently
> verify your work. Integrity violations WILL be detected and your
> work WILL be rejected.

Your objective is to perform the platform cleanup for Milestone 1.
Specifically, delete the following files and directories from the repository:
1. Root-level platform directories:
   - `android/`
   - `appimage/`
   - `.ci/`
   - `.github/` (Wait, you are running in .agents/ inside the repo. Be careful to delete only the .github/ directory at the root, do NOT delete .agents/ or anything else)
   - `_building/`
2. Source-level platform directories:
   - `vita3k/gui-qt/`
   - `vita3k/android/`
   - `i18n/qt/`
3. Platform-specific root scripts:
   - `gen-linux.sh`
   - `gen-windows.bat`
4. Helper/Asset files in `vita3k/` and `cmake/`:
   - `vita3k/main.cpp`
   - `vita3k/Vita3K.rc`
   - `vita3k/resource.h`
   - `vita3k/Windows.manifest`
   - `vita3k/resources.qrc`
   - `vita3k/Vita3K.ico`
   - `vita3k/Vita3K.png`
   - `cmake/qt6.cmake`
   - `cmake/vcpkg_android.cmake`
   - `vita3k/util/src/vc_runtime_checker.cpp`
   - `vita3k/util/src/android_driver.cpp`
   - `vita3k/util/include/util/android_driver.h`
5. Obsolete directories under `external/`:
   - `external/libadrenotools/`
   - `external/nativefiledialog-extended/`
   - `external/googletest/`
   - `external/tracy/`
   - `external/dirent/`
6. Selective Icon Cleanup:
   - Inside the folder `vita3k/icons/`, delete all files EXCEPT:
     - `bronze.png`
     - `silver.png`
     - `gold.png`
     - `platinum.png`
     - `cross.png`
     - `doublearrow.png`
     - `info.png`

Verify the deletions programmatically (e.g. by checking paths). Write your handoff and progress report in your working directory and notify the parent orchestrator (ccd702bd-de31-44a5-b808-05a9695c389d) via send_message when complete.
