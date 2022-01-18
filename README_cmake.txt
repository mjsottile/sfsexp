Using sfsexp library from cmake-based projects:
----------------------------------------------

Presently, the library does not come with a proper FindSfsexp.cmake module
file.  In the meantime, you can add the following lines to a CMakeLists.txt
file.  Assuming the source tree for the sfsexp library exists in
/Users/matt/Research/sfsexp/src  :

INCLUDE_DIRECTORIES(
  /Users/matt/Research/sfsexp/src/src
)

LINK_DIRECTORIES(
  /Users/matt/Research/sfsexp/src/src
)

And then just add "sexp" to the TARGET_LINK_LIBRARIES list.
