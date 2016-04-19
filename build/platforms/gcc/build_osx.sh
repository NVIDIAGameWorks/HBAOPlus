#!/bin/bash

OutDir=../../../lib
SrcDir=../../../src
IncludeDir=../../../include

mkdir $OutDir 2>/dev/null

g++ -O3 -arch i386 -arch x86_64 -DSUPPORT_GL=1 -DMACOSX=1 -I$IncludeDir -flat_namespace -dynamiclib -o $OutDir/GFSDK_SSAO_GL.osx.dylib $SrcDir/*.cpp
