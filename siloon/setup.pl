#!/usr/bin/perl
# ACL:license
#
#This software and ancillary information (herein called "SOFTWARE")
#called Supermon is made available under the terms described
#here.  The SOFTWARE has been approved for release with associated
#LA-CC Number LA-CC 99-51.
#
#Unless otherwise indicated, this SOFTWARE has been authored by an
#employee or employees of the University of California, operator of the
#Los Alamos National Laboratory under Contract No.  W-7405-ENG-36 with
#the U.S. Department of Energy.  The U.S. Government has rights to use,
#reproduce, and distribute this SOFTWARE, and to allow others to do so.
#The public may copy, distribute, prepare derivative works and publicly
#display this SOFTWARE without charge, provided that this Notice and
#any statement of authorship are reproduced on all copies.  Neither the
#Government nor the University makes any warranty, express or implied,
#or assumes any liability or responsibility for the use of this
#SOFTWARE.
#
#If SOFTWARE is modified to produce derivative works, such modified
#SOFTWARE should be clearly marked, so as not to confuse it with the
#version available from LANL.
#
# ACL:license

##
## setup for siloon
##

# directory where siloon was installed
$siloon = "/Users/matt/siloon";

# current directory
$curdir = `pwd`;
$curdir =~ s/\n//g;
$curdir =~ s/\r//g;

# list of source files in src directory
@sources = ("sexpr.c","parser.c","io.c","faststack.c");

# list of functions we want to have callable from python
@functions = ("parse_sexp","find_sexp","destroy_sexp","print_sexp");

##
## remove module if it exists
##
if (-d "./siloon-sexpr") {
    print "Removing existing module...\n";
    system("rm -Rf ./siloon-sexpr");
}

##
## init
##
system($siloon."/bin/siloon-init sexpr");

##
## cd into the module dir
##
chdir("./siloon-sexpr");

##
## tweak the user.defs file
##
$tempfile = "/tmp/__user.defs";
open(USERDEF,"< user.defs");
open(TMPUSERDEF,"> $tempfile");

print TMPUSERDEF "SEXPRROOT=".$curdir."/../src\n";

while (<USERDEF>) {
    if (/^SILOON_PARSE_INCLUDES=/) {
	print TMPUSERDEF "SILOON_PARSE_INCLUDES=-I\${SEXPRROOT}\n";
    } else {
	if (/^SILOON_USER_SOURCES=/) {
	    $srclist = "";
	    foreach $src (@sources) {
		$srclist .= "\${SEXPRROOT}/$src ";
	    }
	    print TMPUSERDEF "SILOON_USER_SOURCES=$srclist\n";
	} else {
	    if (/^SILOON_USER_LIBRARIES=/) {
		print TMPUSERDEF "SILOON_USER_LIBRARIES=-L\${SEXPRROOT} ".
		    "-lsexpr\n";
	    } else {
		print TMPUSERDEF $_;
	    }
	} 
    }
}

close(USERDEF);
close(TMPUSERDEF);

# move the temp user.defs into place
system("mv /tmp/__user.defs ./user.defs");

##
## run siloon parse on each source file
##
system("make pdb");

# now, for the version I'm using we need to fix the files...

$srcdir = $curdir."/../src";

foreach $src (@sources) {
    if (-e $srcdir."/".$src.".pdb") {
	print "Fixing $src.pdb\n";
	$src =~ s/.c$//g;
	$cmd = "mv ".$srcdir."/".$src.".c.pdb ./".$src.".pdb";
	system($cmd);
    }
}

##
## run siloon-gen
##

system($siloon."/bin/siloon-gen sexpr.pdb");

##
## build prototypes.doinclude
##

foreach $fn (@functions) {
    system("grep $fn prototypes.excluded >> prototypes.doinclude");
}
