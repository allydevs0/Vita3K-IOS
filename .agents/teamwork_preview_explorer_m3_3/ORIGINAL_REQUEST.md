## 2026-06-20T04:15:55Z
You are a read-only exploration agent for Milestone 3. Your working directory is C:\Users\denise\Downloads\Vita3K\.agents\teamwork_preview_explorer_m3_3\. Your identity is explorer_m3_3.

Your task is to analyze `external/CMakeLists.txt` at C:\Users\denise\Downloads\Vita3K and plan the cleanup to target iOS exclusively.
Specifically:
1. Identify all blocks, add_subdirectory commands, download instructions, and link setups for:
   - `discord-rpc`
   - `libadrenotools`
   - `winsock`
   - `MoltenVK-macos.tar` download (identify how to adjust MoltenVK for iOS target, e.g., downloading/using the iOS MoltenVK library instead of macOS desktop)
   - Vulkan validation layers download
   - `nativefiledialog-extended`
   - `googletest`
   - OpenSSL build-from-source for desktop
2. Propose a clean `external/CMakeLists.txt` that retains only the required dependencies: `capstone`, `dynarmic`, `fmt`, `spdlog`, `SDL3`, `cubeb`, `glslang`, `SPIRV-Cross`, `pugixml`, `miniz`, `yaml-cpp`, `ffmpeg`, `xxHash`, `dlmalloc`, `libatrac9`, `libfat16`, `stb`, `ddspp`, `cppcommon`, `CLI11`, `vulkan` headers, `vma`, `psvpfstools`, `ssl/crypto`.
3. Write your analysis report to `C:\Users\denise\Downloads\Vita3K\.agents\teamwork_preview_explorer_m3_3\analysis.md` and handoff report to `handoff.md`.
4. Notify the parent orchestrator (ccd702bd-de31-44a5-b808-05a9695c389d) via send_message when complete.
