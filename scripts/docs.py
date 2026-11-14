import webbrowser
from pathlib import Path
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
docDir = os.path.join("build", "docs")

if os.path.exists(docDir):
    os.chdir(docDir)
else:
    quit()

# Run doxygen
update.message("Running doxygen")
try:
    # C library docs -> build/docs/doxygen/doxygen
    # Python binding docs -> build/docs/doxygen/python (each site links to the other)
    for doxyfile in ["Doxyfile.docs", "Doxyfile.docs_python"]:
        subprocess.call(["doxygen", doxyfile])
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
webbrowser.open_new_tab(Path("doxygen", "doxygen", "index.html").resolve().as_uri())

update.message("Done!")
