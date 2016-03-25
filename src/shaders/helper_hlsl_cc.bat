@SET "HLSL_CC=%~dp0\HLSLcc.exe"
@SET "STRINGIFY=%~dp0\Stringify.exe"
@SET "HELPER_FXC=%~dp0\helper_fxc"

@CALL %HELPER_FXC%
@IF ERRORLEVEL 1 GOTO error

:: static const unsigned int HLSLCC_FLAG_UNIFORM_BUFFER_OBJECT = 0x1;
@%HLSL_CC% %HLSL_CC_CMD_LINE%
@IF ERRORLEVEL 1 GOTO error

@%STRINGIFY% %STRINGIFY_CMD_LINE%
@IF ERRORLEVEL 1 GOTO error

:done
@EXIT /B 0

:error
@echo %0: ERROR
@EXIT /B 1
