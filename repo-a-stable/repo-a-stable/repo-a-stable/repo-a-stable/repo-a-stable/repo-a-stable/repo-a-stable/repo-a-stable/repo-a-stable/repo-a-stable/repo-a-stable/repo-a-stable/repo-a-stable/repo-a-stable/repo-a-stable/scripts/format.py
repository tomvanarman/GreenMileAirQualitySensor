"""
Script that runs prettier on all input files
"""
import os
import sys
import subprocess


def main():
    """Run npx prettier --write on all files passed as arguments."""
    if len(sys.argv) < 2:
        print("No files to format")
        return 0

    os.chdir("web")

    if not os.path.isdir("node_modules"):
        print("Need to install node modules...")
        subprocess.run(["npm", "install"], shell=True, capture_output=False)

    # Get all files passed to the hook (excluding the script name)
    files = sys.argv[1:]
    # We are in the web directory, so all files are one level up
    files = [f"../{f}" for f in files]

    # Build the prettier command
    cmd = [
        "npx",
        "prettier",
        "--config",
        "../.prettierrc",
        "--log-level",
        "silent",
        "--write",
    ] + files

    os.execvp("npx", cmd)


if __name__ == "__main__":
    sys.exit(main())
