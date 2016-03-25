/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "AppState_GL.h"

#if SUPPORT_GL

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::AppState::Save(const GFSDK_SSAO_GLFunctions& GL)
{
    ASSERT_GL_ERROR(GL);

    GL.glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_DrawFBO);
    GL.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_VAO);
    GL.glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &m_ContextUBO);
    for (GLuint BindingPoint = 0; BindingPoint < SIZEOF_ARRAY(m_BindPointUBOs); ++BindingPoint)
    {
        GL.glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, BindingPoint, &m_BindPointUBOs[BindingPoint]);
    }

    GL.glGetIntegerv(GL_ACTIVE_TEXTURE, &m_ActiveTexture);
    GL.glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_TextureBinding2D);
    GL.glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &m_TextureBinding2DArray);
    GL.glGetIntegerv(GL_CURRENT_PROGRAM, &m_Program);
    GL.glGetIntegerv(GL_VIEWPORT, m_Viewport);
    GL.glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &m_PolygonOffsetFactor);
    GL.glGetFloatv(GL_POLYGON_OFFSET_UNITS, &m_PolygonOffsetUnits);

    m_CullFace = GL.glIsEnabled(GL_CULL_FACE);
    m_ScissorTest = GL.glIsEnabled(GL_SCISSOR_TEST);
    m_Multisample = GL.glIsEnabled(GL_MULTISAMPLE);
    m_DepthTest = GL.glIsEnabled(GL_DEPTH_TEST);
    m_StencilTest = GL.glIsEnabled(GL_STENCIL_TEST);
    
#if !USE_GLES
    m_ColorLogicOp = GL.glIsEnabled(GL_COLOR_LOGIC_OP);
#endif
    
    m_SampleCoverage = GL.glIsEnabled(GL_SAMPLE_COVERAGE);
    m_SampleAlphaToCoverage = GL.glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
    m_Blend = GL.glIsEnabledi(GL_BLEND, 0);

    GL.glGetIntegerv(GL_BLEND_SRC_RGB, &m_BlendSrcRGB);
    GL.glGetIntegerv(GL_BLEND_SRC_ALPHA, &m_BlendSrcAlpha);
    GL.glGetIntegerv(GL_BLEND_DST_RGB, &m_BlendDstRGB);
    GL.glGetIntegerv(GL_BLEND_DST_ALPHA, &m_BlendDstAlpha);
    GL.glGetIntegerv(GL_BLEND_EQUATION_RGB, &m_BlendEquationRGB);
    GL.glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &m_BlendEquationAlpha);
    GL.glGetFloatv(GL_BLEND_COLOR, m_BlendColor);
    GL.glGetBooleani_v(GL_COLOR_WRITEMASK, 0, m_ColorWriteMask);

    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::AppState::Restore(const GFSDK_SSAO_GLFunctions& GL)
{
    ASSERT_GL_ERROR(GL);

    GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_DrawFBO);
    GL.glBindVertexArray(m_VAO);
    GL.glBindBuffer(GL_UNIFORM_BUFFER, m_ContextUBO);
    for (GLuint BindingPoint = 0; BindingPoint < SIZEOF_ARRAY(m_BindPointUBOs); ++BindingPoint)
    {
        GL.glBindBufferBase(GL_UNIFORM_BUFFER, BindingPoint, m_BindPointUBOs[BindingPoint]);
    }

    GL.glActiveTexture(m_ActiveTexture);
    GL.glBindTexture(GL_TEXTURE_2D, m_TextureBinding2D);
    GL.glBindTexture(GL_TEXTURE_2D_ARRAY, m_TextureBinding2DArray);
    GL.glUseProgram(m_Program);
    GL.glViewport(m_Viewport[0], m_Viewport[1], m_Viewport[2], m_Viewport[3]);
    GL.glPolygonOffset(m_PolygonOffsetFactor, m_PolygonOffsetUnits);

    EnableState(GL, m_CullFace, GL_CULL_FACE);
    EnableState(GL, m_ScissorTest, GL_SCISSOR_TEST);
    EnableState(GL, m_Multisample, GL_MULTISAMPLE);
    EnableState(GL, m_DepthTest, GL_DEPTH_TEST);
    EnableState(GL, m_StencilTest, GL_STENCIL_TEST);
    
#if !USE_GLES
    EnableState(GL, m_ColorLogicOp, GL_COLOR_LOGIC_OP);
#endif
    
    EnableState(GL, m_SampleCoverage, GL_SAMPLE_COVERAGE);
    EnableState(GL, m_SampleAlphaToCoverage, GL_SAMPLE_ALPHA_TO_COVERAGE);
    EnableState(GL, m_Blend, GL_BLEND);

    GL.glBlendFuncSeparate(m_BlendSrcRGB, m_BlendDstRGB, m_BlendSrcAlpha, m_BlendDstAlpha);
    GL.glBlendEquationSeparate(m_BlendEquationRGB, m_BlendEquationAlpha);
    GL.glBlendColor(m_BlendColor[0], m_BlendColor[1], m_BlendColor[2], m_BlendColor[3]);
    GL.glColorMaski(0, m_ColorWriteMask[0], m_ColorWriteMask[1], m_ColorWriteMask[2], m_ColorWriteMask[3]);

    ASSERT_GL_ERROR(GL);
}

#endif //SUPPORT_GL
