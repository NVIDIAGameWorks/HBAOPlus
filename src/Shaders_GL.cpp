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

#include "Shaders_GL.h"

using namespace GFSDK::SSAO::GL;

//--------------------------------------------------------------------------------
static char* g_FullscreenTriangle_VS_GLSL = 
    (char*) STRINGIFY(
        out vec4 VtxGeoOutput1;
        void main()
        {
            vec2 TexCoords = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );
            gl_Position = vec4( TexCoords * vec2( 2.0, 2.0 ) + vec2( -1.0, -1.0) , 0.0, 1.0 );
            VtxGeoOutput1 = vec4( TexCoords, 0.0, 0.0 );
        }
    );

//--------------------------------------------------------------------------------
void GLSLPrograms::CopyDepth_PS::Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource)
{
    GLSLProgram::Create(GL, g_FullscreenTriangle_VS_GLSL, FragmentShaderSource);

    m_DepthTexture = GetUniformLocation(GL, "g_t0");
    ASSERT(m_DepthTexture != -1);
}

//--------------------------------------------------------------------------------
void GLSLPrograms::LinearizeDepth_PS::Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource)
{
    GLSLProgram::Create(GL, g_FullscreenTriangle_VS_GLSL, FragmentShaderSource);

    m_DepthTexture          = GetUniformLocation(GL, "g_t0");
    m_GlobalUniformBlock    = GetUniformBlockIndex(GL, "GlobalConstantBuffer");

    GL.glUniformBlockBinding(m_Program, m_GlobalUniformBlock, BaseConstantBuffer::BINDING_POINT_GLOBAL_UBO);
    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------
void GLSLPrograms::DeinterleaveDepth_PS::Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource)
{
    GLSLProgram::Create(GL, g_FullscreenTriangle_VS_GLSL, FragmentShaderSource);

    m_DepthTexture          = GetUniformLocation(GL, "g_t0");
    m_GlobalUniformBlock    = GetUniformBlockIndex(GL, "GlobalConstantBuffer");
    m_PerPassUniformBlock   = GetUniformBlockIndex(GL, "PerPassConstantBuffer");

    GL.glUniformBlockBinding(m_Program, m_GlobalUniformBlock, BaseConstantBuffer::BINDING_POINT_GLOBAL_UBO);
    GL.glUniformBlockBinding(m_Program, m_PerPassUniformBlock, BaseConstantBuffer::BINDING_POINT_PER_PASS_UBO);
    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------
void GLSLPrograms::ReconstructNormal_PS::Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource)
{
    GLSLProgram::Create(GL, g_FullscreenTriangle_VS_GLSL, FragmentShaderSource);

    m_FullResDepthTexture       = GetUniformLocation(GL, "g_t0");
    m_FullResNormalTexture      = GetUniformLocation(GL, "g_t1");
    m_GlobalUniformBlock        = GetUniformBlockIndex(GL, "GlobalConstantBuffer");

    if (m_GlobalUniformBlock != GL_INVALID_INDEX)
    {
        GL.glUniformBlockBinding(m_Program, m_GlobalUniformBlock, BaseConstantBuffer::BINDING_POINT_GLOBAL_UBO);
    }

    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------
void GLSLPrograms::CoarseAO_PS::Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource)
{
    static const char *VertexShaderSource = 
        STRINGIFY(
            void main()
            {
                gl_Position = vec4(0);
            }
        );

    static const char *GeometryShaderSource = 
        // Must match the uniform declaration from shaders/out/GL/CoarseAO_PS.cpp
        STRINGIFY(
            layout(std140) uniform;
            struct PerPassConstantBuffer_0_Type
            {
                vec4 f4Jitter;
                vec2 f2Offset;
                float fSliceIndex;
                uint uSliceIndex;
            };
            uniform PerPassConstantBuffer
            {
                PerPassConstantBuffer_0_Type PerPassConstantBuffer_0;
            };
        )
        STRINGIFY(
            layout(points) in;
            layout(triangle_strip, max_vertices = 3) out;

            void main()
            {
                gl_Layer = int(PerPassConstantBuffer_0.uSliceIndex);

                for (int VertexID = 0; VertexID < 3; VertexID++)
                {
                    vec2 texCoords = vec2( (VertexID << 1) & 2, VertexID & 2 );
                    gl_Position = vec4( texCoords * vec2( 2.0, 2.0 ) + vec2( -1.0, -1.0) , 0.0, 1.0 );
                    EmitVertex();
                }
                EndPrimitive();
            }
        );

    GLSLProgram::Create(GL, VertexShaderSource, GeometryShaderSource, FragmentShaderSource);

    m_QuarterResDepthTexture    = GetUniformLocation(GL, "g_t0");
    m_FullResNormalTexture      = GetUniformLocation(GL, "g_t1");
    m_GlobalUniformBlock        = GetUniformBlockIndex(GL, "GlobalConstantBuffer");
    m_PerPassUniformBlock       = GetUniformBlockIndex(GL, "PerPassConstantBuffer");

    GL.glUniformBlockBinding(m_Program, m_GlobalUniformBlock, BaseConstantBuffer::BINDING_POINT_GLOBAL_UBO);
    GL.glUniformBlockBinding(m_Program, m_PerPassUniformBlock, BaseConstantBuffer::BINDING_POINT_PER_PASS_UBO);
    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------
void GLSLPrograms::ReinterleaveAO_PS::Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource)
{
    GLSLProgram::Create(GL, g_FullscreenTriangle_VS_GLSL, FragmentShaderSource);

    m_AOTexture             = GetUniformLocation(GL, "g_t0");
    m_DepthTexture          = GetUniformLocation(GL, "g_t1");
    m_GlobalUniformBlock    = GetUniformBlockIndex(GL, "GlobalConstantBuffer");

    if (m_GlobalUniformBlock != GL_INVALID_INDEX)
    {
        GL.glUniformBlockBinding(m_Program, m_GlobalUniformBlock, BaseConstantBuffer::BINDING_POINT_GLOBAL_UBO);
        ASSERT_GL_ERROR(GL);
    }
}

//--------------------------------------------------------------------------------
void GLSLPrograms::Blur_PS::Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource)
{
    GLSLProgram::Create(GL, g_FullscreenTriangle_VS_GLSL, FragmentShaderSource);

    m_AODepthTextureNearest = GetUniformLocation(GL, "g_t0");
    m_AODepthTextureLinear  = GetUniformLocation(GL, "g_t1");
    m_GlobalUniformBlock    = GetUniformBlockIndex(GL, "GlobalConstantBuffer");

    GL.glUniformBlockBinding(m_Program, m_GlobalUniformBlock, BaseConstantBuffer::BINDING_POINT_GLOBAL_UBO);
    ASSERT_GL_ERROR(GL);
}

//--------------------------------------------------------------------------------

namespace GFSDK
{
namespace SSAO
{
namespace GL
{

#include "shaders/out/GL/CopyDepth_PS.cpp"
#include "shaders/out/GL/LinearizeDepth_PS.cpp"
#include "shaders/out/GL/DeinterleaveDepth_PS.cpp"
#include "shaders/out/GL/DebugNormals_PS.cpp"
#include "shaders/out/GL/ReconstructNormal_PS.cpp"
#include "shaders/out/GL/ReinterleaveAO_PS.cpp"
#include "shaders/out/GL/BlurX_PS.cpp"
#include "shaders/out/GL/BlurY_PS.cpp"
#include "shaders/out/GL/CoarseAO_PS.cpp"

} // namespace GL
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_GL
