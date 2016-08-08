@echo off

set XPJ="..\..\..\..\..\..\..\build\tools\xpj4.exe"

%XPJ% -v 1 -t VC9 -p win32 -x DXUT.xpj
%XPJ% -v 1 -t VC9 -p win64 -x DXUT.xpj
%XPJ% -v 1 -t VC10 -p win32 -x DXUT.xpj
%XPJ% -v 1 -t VC10 -p win64 -x DXUT.xpj
%XPJ% -v 1 -t VC11 -p win32 -x DXUT.xpj
%XPJ% -v 1 -t VC11 -p win64 -x DXUT.xpj
