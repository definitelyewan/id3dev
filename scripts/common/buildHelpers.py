import os
from sys import platform
import subprocess
import errno
import shutil
from .InformUser import Inform


# replaces a folder
def replace_folder(foldername):
    if not os.path.exists(foldername):
        os.mkdir(foldername)
    else:
        shutil.rmtree(foldername)
        os.mkdir(foldername)


# builds a cmake project on different platforms
def cmake_build(src, build, options):
    
    update = Inform(1)

    # generate an array to pass to subprocess
    command = ["cmake"]

    if platform == "win32":
        command.append("-DCMAKE_CXX_COMPILER=g++")
        command.append("-DCMAKE_C_COMPILER=gcc")
        command.append("-G")
        command.append("MinGW Makefiles")

    command.append("-S")
    command.append(src)
    command.append("-B")
    
    if platform == "linux" or platform == "linux2" or "darwin":
        command.append("./" + build)
    elif platform == "win32":
        command.append(".\\" + build)


    for option in options:
        command.append(option)

    # build with cmake
    try:
        subprocess.call(command)
    
    # error out
    except OSError as e:
        if e.errno == errno.ENOENT:
            # program was not found
            print("Cmake is not installed or could not be found")
            quit()
        else:
            # program output
            raise

# compiles code with make or msbuild
def compile_code(project_name):
    try:
        if platform == "win32":
            subprocess.call(["mingw32-make", project_name])
        else:
            subprocess.call(["make", project_name])

    except OSError as e:
        if e.errno == errno.ENOENT:
            # program was not found
            print("failed to build tests either make or minGW was not found in PATH or it was not installed")
            quit()
        else:
            # program output
            raise

# checks to see if a program is installed on the host
def is_command(tool):
        return shutil.which(tool) is not None