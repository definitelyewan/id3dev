from common.buildHelpers import *
from common.InformUser import *
import sys

args = sys.argv
argv1 = ''

if len(args) > 0:
    argv1 = args[0]

update = Inform(1)

# mk build
if (argv1 == "clean"):
    update.message("Creating build dir ..")
    replace_folder("build")


# build with cmake
update.message("Running Cmake")
cmake_build(".", "build", ["-DBUILD_ID3_TESTS=ON", "-DBUILD_ID3_SHARED_LIBS=ON","-DDEBUG_ID3_SYMBOLS=ON"])

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
update.message("Compiling id3v2_context_test program")
compile_code("id3v2_context_test")
update.message("Compiling id3v2_parser_test program")
compile_code("id3v2_parser_test")
update.message("Compiling id3v2_frame_test program")
compile_code("id3v2_frame_test")
update.message("Compiling id3v2_test program")
compile_code("id3v2_test")
update.message("Compiling id3dev_test program")
compile_code("id3dev_test")

# call test execs
try:
    if platform == "linux" or platform == "linux2":
        
        if(is_command("valgrind")):
            subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3v1_test"])
            subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3v2_tag_identity_test"])
            subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3v2_context_test"])
            subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3v2_parser_test"])
            subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3v2_frame_test"])
            subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3v2_test"])
            subprocess.call(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "./id3dev_test"])
        else:
            subprocess.call(["./id3v1_test"])
            subprocess.call(["./id3v2_tag_identity_test"])
            subprocess.call(["./id3v2_context_test"])
            subprocess.call(["./id3v2_parser_test"])
            subprocess.call(["./id3v2_frame_test"])
            subprocess.call(["./id3v2_test"])
            subprocess.call(["./id3dev_test"])
        
    elif platform == "darwin":

        if(is_command("leaks")):
            
            malloc_stack_logging = False

            if(os.environ.get("MallocStackLogging") != "0"):
                malloc_stack_logging = True

            # will get changed back but just in case it needs to be set
            os.environ["MallocStackLogging"] = "1"

            subprocess.call(["leaks", "--atExit", "--list", "--", "./id3v1_test"])
            subprocess.call(["leaks", "--atExit", "--list", "--", "./id3v2_tag_identity_test"])
            subprocess.call(["leaks", "--atExit", "--list", "--", "./id3v2_context_test"])
            subprocess.call(["leaks", "--atExit", "--list", "--", "./id3v2_parser_test"])
            subprocess.call(["leaks", "--atExit", "--list", "--", "./id3v2_frame_test"])
            subprocess.call(["leaks", "--atExit", "--list", "--", "./id3v2_test"])
            subprocess.call(["leaks", "--atExit", "--list", "--", "./id3dev_test"])

            if(malloc_stack_logging == False):
                os.environ["MallocStackLogging"] = "0"
        else:
            subprocess.call(["./id3v1_test"])
            subprocess.call(["./id3v2_tag_identity_test"])
            subprocess.call(["./id3v2_context_test"])
            subprocess.call(["./id3v2_parser_test"])
            subprocess.call(["./id3v2_frame_test"])
            subprocess.call(["./id3v2_test"])
            subprocess.call(["./id3dev_test"])


    elif platform == "win32":
        subprocess.call("id3v1_test.exe")
        subprocess.call("id3v2_tag_identity_test.exe")
        subprocess.call(["id3v2_context_test.exe"])
        subprocess.call(["id3v2_parser_test.exe"])
        subprocess.call(["id3v2_frame_test.exe"])
        subprocess.call(["id3v2_test.exe"])
        subprocess.call(["id3dev_test.exe"])
        
except OSError as e:
    if e.errno == errno.ENOENT:
        # program was not found
        print("failed to run a test")
        quit()
    else:
        # program output
        raise

update.message("Done!")
