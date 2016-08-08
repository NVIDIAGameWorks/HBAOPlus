@echo off

cd %~dp0

del /F /Q bin\*

set FXC=fxc.exe

%FXC% /O3 /T ps_5_0 src/Scene3D.hlsl /E CopyColorPS /Fh bin/CopyColorPS.h

%FXC% /O3 /T vs_5_0 src/Scene3D.hlsl /E FullScreenTriangleVS /Fh bin/FullScreenTriangleVS.h

%FXC% /O3 /T vs_5_0 src/Scene3D.hlsl /E GeometryVS /Fh bin/GeometryVS.h

%FXC% /O3 /T ps_5_0 src/Scene3D.hlsl /E GeometryColorPS /Fh bin/GeometryColorPS.h

%FXC% /O3 /T ps_5_0 src/Scene3D.hlsl /E GeometryColorNormalPS /Fh bin/GeometryColorNormalPS.h

pause
