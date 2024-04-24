from common.buildHelpers import *
from common.InformUser import *

update = Inform(1)

# mk build
update.message("Creating build dir ..")
replace_folder("build")

# build with cmake
update.message("Running Cmake")
cmake_build(".", "build", ["-DBUILD_ID3_C_EXAMPLES=ON"])

# move to build
update.message("Moving to build dir")
if os.path.exists("build"):
    os.chdir("build")
else:
    quit()

# compile the code
update.message("Compiling libraries")
compile_code("Id3dev")

# move to tests
update.message("Moving to examples")
if os.path.exists("examples"):
    os.chdir("examples")
else:
    quit()

# compile code
update.message("Compiling C example programs")
compile_code("id3dev_printInfo_c_example")
compile_code("id3dev_savePicture_c_example")
compile_code("id3dev_listAllFrames_c_example")
compile_code("id3dev_displayAllText_c_example")
compile_code("id3dev_buildTag_c_example")
compile_code("id3dev_extractTag_c_example")

update.message("Done! check ./build/examples/ for executables or scripts")
