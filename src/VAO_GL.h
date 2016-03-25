/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if SUPPORT_GL

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{
namespace GL
{

class VAO
{
public:
    VAO() : m_VertexArrayObjectId(0)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL)
    {
        GL.glGenVertexArrays(1, &m_VertexArrayObjectId);
        ASSERT_GL_ERROR(GL);
    }

    void Release(const GFSDK_SSAO_GLFunctions& GL)
    {
        GL.glDeleteVertexArrays(1, &m_VertexArrayObjectId);
        m_VertexArrayObjectId = 0;
    }

    void Bind(const GFSDK_SSAO_GLFunctions& GL)
    {
        GL.glBindVertexArray(m_VertexArrayObjectId);
    }

private:
    GLuint m_VertexArrayObjectId;
};

} // namespace GL
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_GL
