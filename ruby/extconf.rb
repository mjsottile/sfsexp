require 'mkmf'
dir_config('sexp')
have_library('sexp', 'parse_sexp')
create_makefile("Sexp");
