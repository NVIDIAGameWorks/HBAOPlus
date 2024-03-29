@set OUTPUT_GLSL=%SBT_OUTPUT%.glsl
@set OUTPUT_OBJ=%SBT_OUTPUT%.o

@set FXC_CMD_LINE=/O3 /T %SBT_PROFILE% %SBT_INPUT% /E %SBT_ENTRY_POINT% %SBT_DEFINES% /Fo %OUTPUT_OBJ% 

@set HLSL_CC_CMD_LINE=-flags=1 -in=%OUTPUT_OBJ% -out=%OUTPUT_GLSL% -lang=150

@set STRINGIFY_CMD_LINE=%OUTPUT_GLSL% %SBT_VARIABLE_NAME% %SBT_OUTPUT% 

@call %~dp0\helper_hlsl_cc

@del %OUTPUT_GLSL%
@del %OUTPUT_OBJ%
