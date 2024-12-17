from setuptools import setup, Extension

module1 = Extension(
    "pysexp",
    define_macros=[("MAJOR_VERSION", "1"), ("MINOR_VERSION", "0")],
    include_dirs=["../src"],
    libraries=["sexp"],
    library_dirs=["../src"],
    sources=["pysexp.c"],
)

setup(
    name="pysexp",
    version="1.0",
    description="Python libsexp interface",
    author="Steve James",
    author_email="pyro@linuxlabs.com",
    url="http://sexpr.sf.net/",
    long_description="""
Python interface for s-expression library
""",
    # TODO: Fix the extension module
    # https://setuptools.pypa.io/en/latest/userguide/ext_modules.html
    ext_modules=[module1],
)
