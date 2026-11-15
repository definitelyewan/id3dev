from setuptools import setup

# cffi hooks into setuptools
setup(
    cffi_modules=["build_ffi.py:ffibuilder"],
    options={"bdist_wheel": {"py_limited_api": "cp39"}},
)