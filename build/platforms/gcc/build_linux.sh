#!/bin/bash

OutDir=../../../lib
SrcDir=../../../src
IncludeDir=../../../include

mkdir $OutDir 2>/dev/null

rm -f All.cpp
for f in $SrcDir/*cpp; do echo "#include "\"$f\" >> All.cpp; done

g++ -c -O3 -DSUPPORT_GL=1 -DLINUX=1 -I$IncludeDir -fPIC -o All.o -std=c++11 All.cpp

g++ -shared -o $OutDir/GFSDK_SSAO_GL.x86_64-linux-gnu.so All.o
