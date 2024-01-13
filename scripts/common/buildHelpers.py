import os
from sys import platform
import subprocess
import errno
import shutil


# replaces a folder
def replace_folder(foldername):
    if not os.path.exists(foldername):
        os.mkdir(foldername)
    else:
        shutil.rmtree(foldername)
        os.mkdir(foldername)


# builds a cmake project on different platforms
def cmake_build(src, build, options):
    
    # generate an array to pass to subprocess
    command = ["cmake", "-S", src, "-B"]
    
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
            print("cmake was not found or not installed")
            quit()
        else:
            # program output
            raise


# compiles code with make or msbuild
def compile_code(project_name):
    try:
        if platform == "linux" or platform == "linux2" or platform == "darwin":
            subprocess.call("make")
        elif platform == "win32":
            subprocess.call(["MSBuild.exe", project_name + ".vcxproj"])
    
    except OSError as e:
        if e.errno == errno.ENOENT:
            # program was not found
            print("failed to build tests either make or MSBuild was not found in PATH or it was not installed")
            quit()
        else:
            # program output
            raise
