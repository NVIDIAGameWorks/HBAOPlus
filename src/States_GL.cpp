/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "States_GL.h"

#if SUPPORT_GL

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::States::SetSharedBlendState(const GFSDK_SSAO_GLFunctions& GL)
{
#if !USE_GLES
    GL.glDisable(GL_COLOR_LOGIC_OP);
#endif
    GL.glDisable(GL_SAMPLE_COVERAGE);
    GL.glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::States::SetBlendStateMultiplyPreserveAlpha(const GFSDK_SSAO_GLFunctions& GL)
{
    GL.glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
    GL.glEnable(GL_BLEND);
    GL.glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    GL.glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::States::SetBlendStateDisabledPreserveAlpha(const GFSDK_SSAO_GLFunctions& GL)
{
    GL.glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
    GL.glDisable(GL_BLEND);
    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::States::SetBlendStateDisabled(const GFSDK_SSAO_GLFunctions& GL)
{
    GL.glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    GL.glDisable(GL_BLEND);
    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::States::SetDepthStencilStateDisabled(const GFSDK_SSAO_GLFunctions& GL)
{
    GL.glDisable(GL_DEPTH_TEST);
    GL.glDisable(GL_STENCIL_TEST);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::States::SetRasterizerStateFullscreenNoScissor(const GFSDK_SSAO_GLFunctions& GL)
{
    GL.glDisable(GL_CULL_FACE);
    GL.glPolygonOffset(0.f, 0.f);
    GL.glDisable(GL_SCISSOR_TEST);
    GL.glDisable(GL_MULTISAMPLE);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::States::SetCustomBlendState(const GFSDK_SSAO_GLFunctions& GL, const GFSDK_SSAO_CustomBlendState_GL& CustomBlendState)
{
    GL.glEnable(GL_BLEND);

    GL.glBlendEquationSeparate(
        CustomBlendState.BlendEquationSeparate.ModeRGB,
        CustomBlendState.BlendEquationSeparate.ModeAlpha);

    GL.glBlendFuncSeparate(
        CustomBlendState.BlendFuncSeparate.SrcRGB,
        CustomBlendState.BlendFuncSeparate.DstRGB,
        CustomBlendState.BlendFuncSeparate.SrcAlpha,
        CustomBlendState.BlendFuncSeparate.DstAlpha);

    GL.glBlendColor(
        CustomBlendState.BlendColor.R,
        CustomBlendState.BlendColor.G,
        CustomBlendState.BlendColor.B,
        CustomBlendState.BlendColor.A);

    GL.glColorMaski(0,
        CustomBlendState.ColorMask.R,
        CustomBlendState.ColorMask.G,
        CustomBlendState.ColorMask.B,
        CustomBlendState.ColorMask.A);

    ASSERT_GL_ERROR(GL);
}

#endif // SUPPORT_GL
