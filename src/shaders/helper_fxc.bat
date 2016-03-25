@if "%FXC_CMD_LINE%"=="" goto error
@set FXC=%~dp0\fxc.exe

@echo fxc %FXC_CMD_LINE%

@%FXC% %FXC_CMD_LINE% >nul
@if ERRORLEVEL 1 goto error

:done
@exit /B 0

:error
@echo %0: ERROR
@exit /B 1
