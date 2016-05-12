CalculiX
========

CalculiX is an OSS package designed to solve field problems.
The method used is the finite element method.
This package is a native Microsoft Windows 64-bit build of CalculiX. 

## Directory structure

### src

Contains all required scripts, source codes and patches, environment configurations etc. to enable compilation on Windows.
The source code of CalculiX and all required dependencies is automatically downloaded when
the `build.sh` script is run, unless they have been already downloaded. See `src/doc/BuildInstructions.txt` for further details.

### releases

Contains pre-compiled binary packages, ready to install on your machine (see the `README.txt` file in the
package for basic installation and usage instructions.)
