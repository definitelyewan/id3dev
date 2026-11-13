from common.buildHelpers import *
from common.InformUser import Inform
from pathlib import Path
import subprocess
import sys

update = Inform(1)
ROOT = Path.cwd()
HASH_TABLE_VENDOR = ROOT / Path("interfaces/python/id3pairs/vendor/HashTableLib")
ID3_PAIRS_DIR = ROOT / Path("interfaces/python/id3pairs/")
PYTHON_DIR = ROOT / Path("interfaces/python")
VENV_DIR = PYTHON_DIR / ".venv"

update.message("Checking for pip")
if not is_command("pip"):
    quit()

# where am i?
if sys.platform == "win32":
    VENV_PYTHON = VENV_DIR / "Scripts" / "python.exe"
else:
    VENV_PYTHON = VENV_DIR / "bin" / "python"

# set up venv
if not os.path.exists(VENV_DIR):
    update.message("setting up venv")
    os.chdir(PYTHON_DIR)

    subprocess.run(
        [sys.executable, "-m", "venv", str(VENV_DIR)],
        check=True
    )

    subprocess.run(
        [str(VENV_PYTHON), "-m", "pip", "install", "cffi", "setuptools", "pytest", "build"],
        check=True
    )

    os.chdir(ROOT)

# mk build
update.message("Creating build dir ...")
replace_folder("build")

# build with cmake
update.message("Running Cmake")
cmake_build(".", "build", ["-DBUILD_SHARED_LIBS=ON", "-DDEBUG_ID3_SYMBOLS=OFF"])

# move to the build folder
update.message("Moving to build dir")
if os.path.exists("build"):
    os.chdir("build")
else:
    quit()

update.message("Compiling")
compile_code("id3dev")

update.message("Moving to Id3Pairs")
os.chdir(ID3_PAIRS_DIR)

update.message("Copying HashTableLib to vendor")
replace_folder(HASH_TABLE_VENDOR)
copy_to_dest(ROOT / Path("id3dependencies/HashTableLib"), HASH_TABLE_VENDOR)


update.message("Building HashTableLib as Id3pairs")
os.chdir(ID3_PAIRS_DIR)
subprocess.run(
    [str(VENV_PYTHON), "build_ffi.py"],
    check=True
)

update.message("Testing Id3Pairs")
subprocess.run(
    [str(VENV_PYTHON), "-m", "pip", "install", "-e", ".[test]"],
    check=True
)

subprocess.run(
    [str(VENV_PYTHON), "-m", "pytest", "-q"],
    check=True
)

update.message("Installing")
subprocess.run(
    [str(VENV_PYTHON), "-m", "pip", "install", "id3pairs"],
    check=True
)

subprocess.run(
    [str(VENV_PYTHON), "-m", "build"],
    check=True
)

# clean up
update.message("cleaning up")
remove_dir(str(ROOT) + "/interfaces/python/id3pairs/vendor")
remove_dir(str(ROOT) + "/interfaces/python/id3pairs/build")
remove_dir(str(ROOT) + "/interfaces/python/id3pairs/src/id3pairs.egg-info")
os.remove(str(ROOT) + "/interfaces/python/id3pairs/src/id3pairs/_id3pairs.abi3.so")

