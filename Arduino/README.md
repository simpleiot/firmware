[JWS] 7/2/22

This directory contains a small sketch that demonstrates that Google
protobuffers can run on small systems using the nanopb protoc compiler and
toolchain which generates protobuffer bindings in C.  It's helpful to have the
standard implementation of the protoc toolchain installed since some of the
.proto files are shared between the two implementations.  Also it's helpful to
see how the standard implementations may vary slightly from the nanopb .proto
files.

INSTALLATION:

See https://jpa.kapsi.fi/nanopb/ for information on downloading, building,
testing and installing nanopb for generating plain C bindings.
In the folder for your sketch create a sub-directory called 'src' and place
the outputs from protoc there.

BUILDING BINDINGS FOR ARDUINO

Be sure you are using the right protoc compiler when building for Arduino!  By
default the Google protoc compiler will be installed in /usr/local/bin.  The
nanopb protoc compiler is run from <INSTALL_DIR>/nanopb/generator.  It needs a
little help in finding necessary things to create the pb header files.  The
command line I use to build the bindings looks like this:

./protoc --fatal_warnings --proto_path=<path to your .proto source files dir> \
  --proto_path=<INSTALL_DIR>/nanopb/generator/proto/google/protobuf/ \
  --proto_path=<INSTALL_DIR>/nanopb/generator/proto/ \
  --nanopb_out=<path to OUTPUT_DIR> <.proto source file>

Why the nanopb developers didn't put all the needed header files in the same
directory is a mystery, hence all the --proto_path directives. The generated 
*.pb.h and *.pb.c files will end up in the output directory.  These should be
copied to the src directory in the sketch folder.

In the Arduino IDE be sure to include the Nanopb library for the
SIOT_MCU_serial sketch.  Then just load the sketch and compile and upload.
The sketch will run once then stop, monitor the output in the serial console.
This was tested on the Sparkfun RedBoard (Arduino UNO clone),
The Sparkfun RedBoard Turbo (SAMD based processor) will require some kind of
c++ wrapper in order to build Nanopb bindings.  I noted a couple of these
on the web but have not attempted to use them yet.

