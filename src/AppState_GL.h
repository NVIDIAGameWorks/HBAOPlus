// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2021 NVIDIA Corporation. All rights reserved.

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
