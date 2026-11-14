from common.buildHelpers import *
from common.InformUser import Inform
from pathlib import Path
import subprocess
import sys
import shutil

update = Inform(1)
ROOT = Path.cwd()

VENDOR_DEST = ROOT / "interfaces" / "python" / "id3dev" / "vendor"
VENDOR_PARTS = [
    "include",
    "src",
    "LICENSE",
    "id3dependencies/ByteStream/include",
    "id3dependencies/ByteStream/src",
    "id3dependencies/ByteStream/LICENSE",
    "id3dependencies/LinkedListLib/include",
    "id3dependencies/LinkedListLib/src",
    "id3dependencies/LinkedListLib/LICENSE",
    "id3dependencies/HashTableLib/include",
    "id3dependencies/HashTableLib/src",
    "id3dependencies/HashTableLib/LICENSE",
]

PYTHON_DIR = ROOT / "interfaces" / "python"
VENV_DIR = PYTHON_DIR / ".venv"
PROJECT_DIR = PYTHON_DIR / "id3dev"
TEST_ASSETS_SRC = ROOT / "assets"
TEST_ASSETS_DEST = PYTHON_DIR / "id3dev" / "tests" / "assets"

# where am i?
if sys.platform == "win32":
    VENV_PYTHON = VENV_DIR / "Scripts" / "python.exe"
else:
    VENV_PYTHON = VENV_DIR / "bin" / "python"

def cleanup():
    #remove old build folders/files if they exists
    update.message("Cleaning build files")
    shutil.rmtree(VENDOR_DEST, ignore_errors=True)
    file_path = PROJECT_DIR / "src" / "id3dev" / "_id3dev.abi3.so"
    file_path.unlink(missing_ok=True)
    shutil.rmtree(VENV_DIR, ignore_errors=True)
    shutil.rmtree(PROJECT_DIR / "src" / "id3dev.egg-info", ignore_errors=True)
    shutil.rmtree(PROJECT_DIR / "build", ignore_errors=True)
    shutil.rmtree(TEST_ASSETS_DEST, ignore_errors=True)


cleanup()

update.message("Copying id3dev to vendor")
for part in VENDOR_PARTS:
    src, dst = ROOT / part, VENDOR_DEST / "id3dev" / part
    if src.is_dir():
        shutil.copytree(src, dst, ignore=shutil.ignore_patterns("CMakeLists.txt"))
    elif src.is_file():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
    else:
        sys.exit(f"missing: {src}")


update.message("Checking for pip")
if not is_command("pip"):
    update.message("pip not found")
    quit()


update.message("Setting up venv")
os.chdir(PYTHON_DIR)

subprocess.run(
    [sys.executable, "-m", "venv", str(VENV_DIR)],
    check=True
)

subprocess.run(
    [str(VENV_PYTHON), "-m", "pip", "install", "cffi", "setuptools", "pytest", "build"],
    check=True
)

update.message("Building id3dev ffi")
os.chdir(PROJECT_DIR)
subprocess.run(
    [str(VENV_PYTHON), "build_ffi.py"],
    check=True
)

update.message("Testing id3dev ffi")
copy_to_dest(TEST_ASSETS_SRC, TEST_ASSETS_DEST)
subprocess.run(
    [str(VENV_PYTHON), "-m", "pip", "install", "-e", ".[test]"],
    check=True
)

subprocess.run(
    [str(VENV_PYTHON), "-m", "pytest", "-q"],
    check=True
)

update.message("Building")
subprocess.run(
    [str(VENV_PYTHON), "-m", "build"],
    check=True
)

cleanup()

update.message("Done!")
print("Check /interfaces/python/id3dev/dist for a wheel")

