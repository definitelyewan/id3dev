from common.buildHelpers import *
from common.InformUser import Inform
import shutil

update = Inform(1)

# mk build
update.message("Creating build dir ...")
replace_folder("build")

# build with cmake
update.message("Running Cmake")
cmake_build(".", "build", ["-DBUILD_SHARED_LIBS=ON", "-DDEDUB_ID3_SYMBOLS=OFF", "-DBUILD_ID3_PYTHON_INTERFACE=ON"])

# move to the build folder
update.message("Moving to build dir")
if os.path.exists("build"):
    os.chdir("build")
else:
    quit()

update.message("Compiling")
compile_code("Id3dev")

# move to the build interface folder
update.message("Moving to interfaces dir")
if os.path.exists("interfaces"):
    os.chdir("interfaces")
else:
    quit()

# move to the build interface folder
update.message("Moving to python dir")
if os.path.exists("python"):
    os.chdir("python")
    os.remove("cmake_install.cmake")
    os.remove("CMakeLists.txt")
    os.remove("Makefile")
    shutil.rmtree("CMakeFiles")
    update.message("Removed cmake files")
else:
    quit()



update.message("Done!")