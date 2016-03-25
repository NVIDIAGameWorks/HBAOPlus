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
#include "GLSLProgram.h"

namespace GFSDK
{
namespace SSAO
{
namespace GL
{

namespace GLSLPrograms
{

//--------------------------------------------------------------------------
class CopyDepth_PS : public GFSDK::SSAO::GL::GLSLProgram
{
public:
    CopyDepth_PS()
        : m_DepthTexture(-1)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource);

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource, size_t)
    {
        Create(GL, FragmentShaderSource);
    }

    void SetDepthTexture(const GFSDK_SSAO_GLFunctions& GL, const GFSDK_SSAO_Texture_GL &Texture)
    {
        SetTexture(GL, Texture.Target, m_DepthTexture, Texture.TextureId, 0);
    }

protected:
    GLint m_DepthTexture;
};

//--------------------------------------------------------------------------
class LinearizeDepth_PS : public CopyDepth_PS
{
public:
    LinearizeDepth_PS()
        : CopyDepth_PS()
        , m_GlobalUniformBlock(GL_INVALID_INDEX)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource);

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource, size_t)
    {
        Create(GL, FragmentShaderSource);
    }

    void SetDepthTexture(const GFSDK_SSAO_GLFunctions& GL, const GFSDK_SSAO_Texture_GL &Texture)
    {
        SetTexture(GL, Texture.Target, m_DepthTexture, Texture.TextureId, 0);
    }

private:
    GLuint m_GlobalUniformBlock;
};

//--------------------------------------------------------------------------
class DeinterleaveDepth_PS : public GFSDK::SSAO::GL::GLSLProgram
{
public:
    DeinterleaveDepth_PS()
        : m_DepthTexture(-1)
        , m_GlobalUniformBlock(GL_INVALID_INDEX)
        , m_PerPassUniformBlock(GL_INVALID_INDEX)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource);

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource, size_t)
    {
        Create(GL, FragmentShaderSource);
    }

    void SetDepthTexture(const GFSDK_SSAO_GLFunctions& GL, GLint TextureId)
    {
        SetTexture(GL, GL_TEXTURE_2D, m_DepthTexture, TextureId, 0);
    }

private:
    GLint m_DepthTexture;
    GLuint m_GlobalUniformBlock;
    GLuint m_PerPassUniformBlock;
};

//--------------------------------------------------------------------------
class ReconstructNormal_PS : public GFSDK::SSAO::GL::GLSLProgram
{
public:
    ReconstructNormal_PS()
        : m_FullResDepthTexture(-1)
        , m_FullResNormalTexture(-1)
        , m_GlobalUniformBlock(GL_INVALID_INDEX)
        , m_NormalMatrixUniformBlock(GL_INVALID_INDEX)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource);

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource, size_t)
    {
        Create(GL, FragmentShaderSource);
    }

    void SetDepthTexture(const GFSDK_SSAO_GLFunctions& GL, GLint TextureId, GLint WrapMode)
    {
        if (m_FullResDepthTexture != -1)
        {
            SetTexture(GL, GL_TEXTURE_2D, m_FullResDepthTexture, TextureId, 0, GL_NEAREST, WrapMode);
        }
    }

    void SetNormalTexture(const GFSDK_SSAO_GLFunctions& GL, const GFSDK_SSAO_Texture_GL &Texture)
    {
        if (m_FullResNormalTexture != -1)
        {
            SetTexture(GL, Texture.Target, m_FullResNormalTexture, Texture.TextureId, 1);
        }
    }

private:
    GLint m_FullResDepthTexture;
    GLint m_FullResNormalTexture;
    GLuint m_GlobalUniformBlock;
    GLuint m_NormalMatrixUniformBlock;
};

//--------------------------------------------------------------------------
class DebugNormals_PS : public ReconstructNormal_PS
{
};

//--------------------------------------------------------------------------
class CoarseAO_PS : public GFSDK::SSAO::GL::GLSLProgram
{
public:
    CoarseAO_PS()
        : m_QuarterResDepthTexture(-1)
        , m_FullResNormalTexture(-1)
        , m_GlobalUniformBlock(GL_INVALID_INDEX)
        , m_PerPassUniformBlock(GL_INVALID_INDEX)
        , m_NormalMatrixUniformBlock(GL_INVALID_INDEX)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource);

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource, size_t)
    {
        Create(GL, FragmentShaderSource);
    }

    void SetDepthTexture(const GFSDK_SSAO_GLFunctions& GL, GLint TextureId, GLint WrapMode)
    {
        SetTexture(GL, GL_TEXTURE_2D_ARRAY, m_QuarterResDepthTexture, TextureId, 0, GL_NEAREST, WrapMode);
    }

    void SetNormalTexture(const GFSDK_SSAO_GLFunctions& GL, const GFSDK_SSAO_Texture_GL &Texture)
    {
        SetTexture(GL, Texture.Target, m_FullResNormalTexture, Texture.TextureId, 1);
    }

private:
    GLint m_QuarterResDepthTexture;
    GLint m_FullResNormalTexture;
    GLuint m_GlobalUniformBlock;
    GLuint m_PerPassUniformBlock;
    GLuint m_NormalMatrixUniformBlock;
};

//--------------------------------------------------------------------------
class ReinterleaveAO_PS : public GFSDK::SSAO::GL::GLSLProgram
{
public:
    ReinterleaveAO_PS()
        : m_AOTexture(-1)
        , m_DepthTexture(-1)
        , m_GlobalUniformBlock(GL_INVALID_INDEX)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource);

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource, size_t)
    {
        Create(GL, FragmentShaderSource);
    }

    void SetAOTexture(const GFSDK_SSAO_GLFunctions& GL, GLint TextureId)
    {
        SetTexture(GL, GL_TEXTURE_2D_ARRAY, m_AOTexture, TextureId, 0);
    }

    void SetDepthTexture(const GFSDK_SSAO_GLFunctions& GL, GLint TextureId)
    {
        SetTexture(GL, GL_TEXTURE_2D, m_DepthTexture, TextureId, 1);
    }

private:
    GLint m_AOTexture;
    GLint m_DepthTexture;
    GLuint m_GlobalUniformBlock;
};

//--------------------------------------------------------------------------
class Blur_PS : public GFSDK::SSAO::GL::GLSLProgram
{
public:
    Blur_PS()
        : m_AODepthTextureNearest(-1)
        , m_AODepthTextureLinear(-1)
        , m_GlobalUniformBlock(GL_INVALID_INDEX)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource);

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *FragmentShaderSource, size_t)
    {
        Create(GL, FragmentShaderSource);
    }

    void SetAODepthTexture(const GFSDK_SSAO_GLFunctions& GL, GLint TextureId)
    {
        SetTexture(GL, GL_TEXTURE_2D, m_AODepthTextureNearest, TextureId, 0, GL_NEAREST);

        if (m_AODepthTextureLinear != -1)
        {
            SetTexture(GL, GL_TEXTURE_2D, m_AODepthTextureLinear,  TextureId, 1, GL_LINEAR);
        }
    }

private:
    GLint m_AODepthTextureNearest;
    GLint m_AODepthTextureLinear;
    GLuint m_GlobalUniformBlock;
};

class BlurX_PS : public Blur_PS
{
};

class BlurY_PS : public Blur_PS
{
};

} //namespace GLSLPrograms

typedef GFSDK_SSAO_GLFunctions DevicePointer;

#include "shaders/out/GL/CopyDepth_PS.h"
#include "shaders/out/GL/LinearizeDepth_PS.h"
#include "shaders/out/GL/DeinterleaveDepth_PS.h"
#include "shaders/out/GL/DebugNormals_PS.h"
#include "shaders/out/GL/ReconstructNormal_PS.h"
#include "shaders/out/GL/ReinterleaveAO_PS.h"
#include "shaders/out/GL/BlurX_PS.h"
#include "shaders/out/GL/BlurY_PS.h"
#include "shaders/out/GL/CoarseAO_PS.h"

//--------------------------------------------------------------------------------
struct Shaders
{
    Generated::CopyDepth_PS         CopyDepth_PS;
    Generated::LinearizeDepth_PS    LinearizeDepth_PS;
    Generated::DeinterleaveDepth_PS DeinterleaveDepth_PS;
    Generated::DebugNormals_PS      DebugNormals_PS;
    Generated::ReconstructNormal_PS ReconstructNormal_PS;
    Generated::ReinterleaveAO_PS    ReinterleaveAO_PS;
    Generated::BlurX_PS             BlurX_PS;
    Generated::BlurY_PS             BlurY_PS;
    Generated::CoarseAO_PS          CoarseAO_PS;

    void Create(const GFSDK_SSAO_GLFunctions& GL)
    {
        CopyDepth_PS.Create(GL);
        LinearizeDepth_PS.Create(GL);
        DeinterleaveDepth_PS.Create(GL);
        DebugNormals_PS.Create(GL);
        ReconstructNormal_PS.Create(GL);
        ReinterleaveAO_PS.Create(GL);
        BlurX_PS.Create(GL);
        BlurY_PS.Create(GL);
        CoarseAO_PS.Create(GL);
    }

    void Release(const GFSDK_SSAO_GLFunctions& GL)
    {
        CopyDepth_PS.Release(GL);
        LinearizeDepth_PS.Release(GL);
        DeinterleaveDepth_PS.Release(GL);
        DebugNormals_PS.Release(GL);
        ReconstructNormal_PS.Release(GL);
        ReinterleaveAO_PS.Release(GL);
        BlurX_PS.Release(GL);
        BlurY_PS.Release(GL);
        CoarseAO_PS.Release(GL);
    }
};

} // namespace GL
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_GL
