#!/usr/bin/env python3
import os
import fnmatch

# File extensions to normalize
TEXT_EXTENSIONS = {
    ".cpp", ".hpp", ".h", ".c", ".cc", ".cxx",
    ".py", ".md", ".txt", ".json", ".yaml", ".yml",
    ".cmake", ".sh", ".bat", ".ini", ".cfg", ".js",
    ".ts", ".html", ".css"
}

def load_gitignore_patterns(root="."):
    """Return sets of ignored directories and file patterns as absolute paths."""
    root = os.path.abspath(root)
    ignored_dirs = set()
    ignored_files = []
    for dirpath, _, filenames in os.walk(root):
        if ".git" in dirpath.split(os.sep):
            continue
        for name in filenames:
            if name == ".gitignore":
                gitignore_path = os.path.join(dirpath, name)
                with open(gitignore_path, "r") as f:
                    lines = [line.strip() for line in f if line.strip() and not line.startswith("#")]
                    for line in lines:
                        abs_dir = os.path.abspath(os.path.join(dirpath, line.rstrip("/")))
                        if line.endswith("/") or line in {".vs", ".vscode"}:
                            ignored_dirs.add(abs_dir)
                        else:
                            ignored_files.append(abs_dir)
    return ignored_dirs, ignored_files

def is_ignored_file(path, ignored_dirs, ignored_files):
    """Return True if path is in ignored dirs or matches ignored files."""
    path = os.path.abspath(path)
    # Skip files in ignored directories
    for idir in ignored_dirs:
        if os.path.commonpath([path, idir]) == idir:
            return True
    # Skip files matching any ignored file patterns
    for fpat in ignored_files:
        if fnmatch.fnmatch(path, fpat):
            return True
    return False

def normalize_file(path):
    try:
        with open(path, "rb") as f:
            data = f.read()

        text = data.decode("utf-8", errors="ignore")
        # Normalize line endings
        new_text = text.replace("\r\n", "\n").replace("\r", "\n")

        if new_text != text:
            with open(path, "w", newline="\n", encoding="utf-8") as f:
                f.write(new_text)
            print(f"Normalized: {path}")  # Only print if changes made
    except Exception as e:
        print(f"Skipping {path} ({e})")


def normalize_directory(root="."):
    ignored_dirs, ignored_files = load_gitignore_patterns(root)

    for dirpath, dirnames, filenames in os.walk(root):
        # Skip .git folder
        if ".git" in dirpath.split(os.sep):
            continue

        # Remove ignored directories from walk
        dirnames[:] = [d for d in dirnames if not is_ignored_file(os.path.join(dirpath, d), ignored_dirs, ignored_files)]

        for filename in filenames:
            _, ext = os.path.splitext(filename)
            if ext.lower() in TEXT_EXTENSIONS:
                full_path = os.path.join(dirpath, filename)
                if not is_ignored_file(full_path, ignored_dirs, ignored_files):
                    normalize_file(full_path)

if __name__ == "__main__":
    print("Normalizing line endings to LF for git-tracked files...")
    normalize_directory(".")
    print("Done.")
