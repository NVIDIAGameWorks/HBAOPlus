/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{
namespace GL
{

#if SUPPORT_GL

//--------------------------------------------------------------------------------
class AppState
{
public:
    AppState()
    {
        memset(this, 0, sizeof(*this));
    }

    void EnableState(const GFSDK_SSAO_GLFunctions& GL, GLboolean IsEnabled, GLenum Cap)
    {
        if (IsEnabled)
        {
            GL.glEnable(Cap);
        }
        else
        {
            GL.glDisable(Cap);
        }
    }

    void Save(const GFSDK_SSAO_GLFunctions& GL);
    void Restore(const GFSDK_SSAO_GLFunctions& GL);

private:
    GLint m_DrawFBO;
    GLint m_VAO;

    GLint m_ContextUBO;
    GLint m_BindPointUBOs[3];

    GLint m_ActiveTexture;
    GLint m_TextureBinding2D;
    GLint m_TextureBinding2DArray;

    GLint m_Program;

    GLint m_Viewport[4];
    GLfloat m_PolygonOffsetFactor;
    GLfloat m_PolygonOffsetUnits;

    GLboolean m_CullFace;
    GLboolean m_ScissorTest;
    GLboolean m_Multisample;
    GLboolean m_DepthTest;
    GLboolean m_StencilTest;
    GLboolean m_ColorLogicOp;
    GLboolean m_SampleCoverage;
    GLboolean m_SampleAlphaToCoverage;

    GLboolean m_Blend;
    GLint m_BlendSrcRGB;
    GLint m_BlendDstRGB;
    GLint m_BlendSrcAlpha;
    GLint m_BlendDstAlpha;
    GLint m_BlendEquationRGB;
    GLint m_BlendEquationAlpha;
    GLfloat m_BlendColor[4];
    GLboolean m_ColorWriteMask[4];
};

#endif // SUPPORT_GL

} // namespace GL
} // namespace SSAO
} // namespace GFSDK
