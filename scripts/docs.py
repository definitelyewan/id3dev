import os
from sys import platform
import subprocess
import errno
import webbrowser
import shutil

#mk build
print("Creating build dir")
if not os.path.exists("build"):
    os.mkdir("build")
else:
    shutil.rmtree("build")
    os.mkdir("build")

#build lib
try:
    if platform == "linux" or platform == "linux2" or platform == "darwin":
        subprocess.call(["cmake", "-S", ".", "-B", "./build", "-DBUILD_ID3_DOCS=ON"])
    elif platform == "win32":
        subprocess.call(["cmake", "-S", ".", "-B", ".\\build","-DBUILD_ID3_DOCS=ON"])

except OSError as e:
    if e.errno == errno.ENOENT:
        #program was not found
        print("cmake was not found or not installed")
        quit()
    else:
        #program output
        raise


#where are the docs
docDir = ""

if platform == "linux" or platform == "linux2" or "darwin":
    docDir = "build/docs"
elif platform == "win32":
    docDir = "build\\doc"
else:
    quit()

if os.path.exists(docDir):
    os.chdir(docDir)
else:
    print("HERE" + os.getcwd())
    quit()

#call doxygen
try:
    subprocess.call(["doxygen", "Doxyfile.docs"])
except OSError as e:
    if e.errno == errno.ENOENT:
        #program was not found
        print("cannot find doxygen or doxygen is not installed")
        quit()
    else:
        #program output
        raise

#open the doc
if platform == "linux" or platform == "linux2" or platform == "darwin":
    webbrowser.open_new_tab("html/index.html") 
elif platform == "win32":
    webbrowser.open_new_tab(".\\html\\index.html")