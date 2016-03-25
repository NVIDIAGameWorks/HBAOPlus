@set FXC_CMD_LINE=/Qstrip_reflect /O3 /T %SBT_PROFILE% %SBT_INPUT% /E %SBT_ENTRY_POINT% %SBT_DEFINES% /Vn %SBT_VARIABLE_NAME% /Fh %SBT_OUTPUT%

@call %~dp0\helper_fxc
