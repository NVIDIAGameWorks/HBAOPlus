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
