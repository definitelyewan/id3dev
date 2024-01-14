import webbrowser
from common.buildHelpers import *
from common.InformUser import Inform

update = Inform(1)

# mk build
update.message("Creating build dir ...")
replace_folder("build")

# build with cmake
update.message("Running Cmake")
cmake_build(".", "build", ["-DBUILD_ID3_DOCS=ON"])

# where are the docs
update.message("moving to docs dir")
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
    quit()

# Run doxygen
update.message("Running doxygen")
try:
    subprocess.call(["doxygen", "Doxyfile.docs"])
except OSError as e:
    if e.errno == errno.ENOENT:
        # program was not found
        print("cannot find doxygen or doxygen is not installed")
        quit()
    else:
        # program output
        raise

# open the doc
update.message("Attempting to open docs in a browser tab")
if platform == "linux" or platform == "linux2" or platform == "darwin":
    webbrowser.open_new_tab("html/index.html") 
elif platform == "win32":
    webbrowser.open_new_tab(".\\html\\index.html")


update.message("Done!")
