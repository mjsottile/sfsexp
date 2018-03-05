pysexp : python bindings for libsexp
------------------------------------

Original version contributed by Steve James (pyro@linuxlabs.com), Nov. 2004


1.0 BUILDING
============

The python bindings for the library, like ruby, are not built by default
when the main library is built.  To build the library, first make sure that
the main library has been built and libsexp is in ../src.  Next, type

  % python setup.py build

to build the shared-object used by Python for the pysexp extension module.
Distutils (setup.py) makes building extensions portable, removing the need
to worry about things like build flags, assembling shared libraries on
arbitrary platforms, etc...  Currently, there is no install or config target
for setup.py, but expect this to be added in the future.

2.0 BASIC USAGE
===============

Write me.
