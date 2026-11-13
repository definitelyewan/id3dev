from setuptools import setup

# cffi hooks into setuptools
setup(cffi_modules=["build_ffi.py:ffibuilder"])