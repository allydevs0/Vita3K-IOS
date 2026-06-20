import os
import shutil
import sys

REPO_ROOT = r"C:\Users\denise\Downloads\Vita3K"

paths_to_delete = [
    # 1. Root-level platform directories
    os.path.join(REPO_ROOT, "android"),
    os.path.join(REPO_ROOT, "appimage"),
    os.path.join(REPO_ROOT, ".ci"),
    os.path.join(REPO_ROOT, ".github"),
    os.path.join(REPO_ROOT, "_building"),

    # 2. Source-level platform directories
    os.path.join(REPO_ROOT, "vita3k", "gui-qt"),
    os.path.join(REPO_ROOT, "vita3k", "android"),
    os.path.join(REPO_ROOT, "i18n", "qt"),

    # 3. Platform-specific root scripts
    os.path.join(REPO_ROOT, "gen-linux.sh"),
    os.path.join(REPO_ROOT, "gen-windows.bat"),

    # 4. Helper/Asset files in vita3k/ and cmake/
    os.path.join(REPO_ROOT, "vita3k", "main.cpp"),
    os.path.join(REPO_ROOT, "vita3k", "Vita3K.rc"),
    os.path.join(REPO_ROOT, "vita3k", "resource.h"),
    os.path.join(REPO_ROOT, "vita3k", "Windows.manifest"),
    os.path.join(REPO_ROOT, "vita3k", "resources.qrc"),
    os.path.join(REPO_ROOT, "vita3k", "Vita3K.ico"),
    os.path.join(REPO_ROOT, "vita3k", "Vita3K.png"),
    os.path.join(REPO_ROOT, "cmake", "qt6.cmake"),
    os.path.join(REPO_ROOT, "cmake", "vcpkg_android.cmake"),
    os.path.join(REPO_ROOT, "vita3k", "util", "src", "vc_runtime_checker.cpp"),
    os.path.join(REPO_ROOT, "vita3k", "util", "src", "android_driver.cpp"),
    os.path.join(REPO_ROOT, "vita3k", "util", "include", "util", "android_driver.h"),

    # 5. Obsolete directories under external/
    os.path.join(REPO_ROOT, "external", "libadrenotools"),
    os.path.join(REPO_ROOT, "external", "nativefiledialog-extended"),
    os.path.join(REPO_ROOT, "external", "googletest"),
    os.path.join(REPO_ROOT, "external", "tracy"),
    os.path.join(REPO_ROOT, "external", "dirent"),
]

icons_to_keep = {
    "bronze.png",
    "silver.png",
    "gold.png",
    "platinum.png",
    "cross.png",
    "doublearrow.png",
    "info.png"
}

icons_dir = os.path.join(REPO_ROOT, "vita3k", "icons")

def delete_path(path):
    if not os.path.exists(path):
        print(f"Skipping {path} (already does not exist)")
        return
    
    try:
        if os.path.isdir(path):
            shutil.rmtree(path)
            print(f"Deleted directory: {path}")
        else:
            os.remove(path)
            print(f"Deleted file: {path}")
    except Exception as e:
        print(f"Error deleting {path}: {e}")

def main():
    print("Starting cleanup process...")

    # Delete paths
    for path in paths_to_delete:
        delete_path(path)

    # Clean icons directory
    if os.path.exists(icons_dir):
        for item in os.listdir(icons_dir):
            item_path = os.path.join(icons_dir, item)
            if os.path.isdir(item_path):
                print(f"Deleting unexpected directory inside icons: {item_path}")
                shutil.rmtree(item_path)
            elif item not in icons_to_keep:
                print(f"Deleting icon file: {item}")
                os.remove(item_path)
            else:
                print(f"Keeping icon file: {item}")
    else:
        print(f"Warning: Icons directory {icons_dir} does not exist.")

    # Programmatic Verification
    print("\nVerifying deletions...")
    failed = False

    # Check deleted paths
    for path in paths_to_delete:
        if os.path.exists(path):
            print(f"VERIFICATION FAILURE: {path} still exists!")
            failed = True
        else:
            print(f"Verified deleted: {path}")

    # Check icons directory
    if os.path.exists(icons_dir):
        remaining_icons = os.listdir(icons_dir)
        for item in remaining_icons:
            if item not in icons_to_keep:
                print(f"VERIFICATION FAILURE: Unexpected file in icons: {item}")
                failed = True
        for item in icons_to_keep:
            if item not in remaining_icons:
                print(f"VERIFICATION FAILURE: Expected icon missing: {item}")
                failed = True
    else:
        print(f"VERIFICATION FAILURE: Icons directory {icons_dir} does not exist!")
        failed = True

    if failed:
        print("\nCleanup failed verification.")
        sys.exit(1)
    else:
        print("\nAll deletions verified successfully!")
        sys.exit(0)

if __name__ == "__main__":
    main()
