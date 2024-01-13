from common.buildHelpers import *
from common.InformUser import *

update = Inform(1)

# mk build
update.message("Creating build dir ..")
replace_folder("build")

# build with cmake
update.message("Running Cmake")
cmake_build(".", "build", ["-DBUILD_ID3_TESTS=ON", "-DBUILD_SHARED_LIBS=ON","-DDEBUG_ID3_SYMBOLS=ON"])

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
update.message("Moving to tests")
if os.path.exists("tests"):
    os.chdir("tests")
else:
    quit()

# compile code
update.message("Compiling id3v1_test program")
compile_code("id3v1_test")
update.message("Compiling id3v2_tag_identity_test program")
compile_code("id3v2_tag_identity_test")


# call test execs
try:
    if platform == "linux" or platform == "linux2":
        subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3v1_test"])
        subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3v2_tag_identity_test"])
    elif platform == "darwin":
        os.environ["MallocStackLogging"] = "1"
        subprocess.call(["leaks", "--atExit", "--list", "--", "./id3v1_test"])
        subprocess.call(["leaks", "--atExit", "--list", "--", "./id3v2_tag_identity_test"])
        os.environ["MallocStackLogging"] = "0"
    elif platform == "win32":
        if os.path.exists("Release"):
            os.chdir("Release")

        subprocess.call(".\\id3v1_test.exe")
        subprocess.call(".\\id3v2_tag_identity_test.exe")
        
except OSError as e:
    if e.errno == errno.ENOENT:
        # program was not found
        print("failed to run a test")
        quit()
    else:
        # program output
        raise

update.message("Done!")
