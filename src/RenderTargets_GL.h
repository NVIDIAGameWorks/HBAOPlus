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
#include "RenderOptions.h"

namespace GFSDK
{
namespace SSAO
{
namespace GL
{

//--------------------------------------------------------------------------------
class BaseRTTexture
{
public:
    GLsizei m_AllocatedSizeInBytes;

    BaseRTTexture()
        : m_AllocatedSizeInBytes(0)
    {
    }

    UINT GetAllocatedSizeInBytes()
    {
        return m_AllocatedSizeInBytes;
    }

    static GLsizei GetFormatSizeInBytes(GLint InternalFormat)
    {
        switch (InternalFormat)
        {
        case GL_RG16F:
        case GL_R32F:
        case GL_RGBA8:
            return 4;
        case GL_R16F:
            return 2;
        case GL_R8:
            return 1;
        }
        ASSERT(0);
        return 0;
    }

    static GLenum GetBaseGLFormat(GLint InternalFormat)
    {
        switch(InternalFormat)
        {
        case GL_R32F:
        case GL_R16F:
        case GL_R8:
            return GL_RED;
        case GL_RG16F:
           return GL_RG;
        case GL_RGBA8:
            return GL_RGBA;
        }
        ASSERT(0);
        return 0;
    }
    
    static GLenum GetBaseGLType(GLint InternalFormat)
    {
        switch(InternalFormat)
        {
        case GL_R32F:
        case GL_R16F:
        case GL_RG16F:
            return GL_FLOAT;
        case GL_R8:
        case GL_RGBA8:
            return GL_UNSIGNED_BYTE;
        }
        ASSERT(0);
        return 0;
    }
};



//--------------------------------------------------------------------------------
class RTTexture2D : public BaseRTTexture
{
public:
    RTTexture2D()
        : BaseRTTexture()
        , m_TextureId(0)
        , m_FboId(0)
    {
    }

    void CreateOnce(const GFSDK_SSAO_GLFunctions& GL, GLsizei Width, GLsizei Height, GLint InternalFormat)
    {
        if (!m_TextureId)
        {
            GL.glGenTextures(1, &m_TextureId);
            GL.glBindTexture(GL_TEXTURE_2D, m_TextureId);
            GL.glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GetBaseGLFormat(InternalFormat), GetBaseGLType(InternalFormat), NULL);
            GL.glBindTexture(GL_TEXTURE_2D, 0);

            THROW_IF(GL.glGetError());

            GL.glGenFramebuffers(1, &m_FboId);
            GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FboId);
            GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureId, 0);
            GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            THROW_IF(GL.glGetError());

            m_AllocatedSizeInBytes = Width * Height * GetFormatSizeInBytes(InternalFormat);
        }
    }

    void SafeRelease(const GFSDK_SSAO_GLFunctions& GL)
    {
        if (m_TextureId)
        {
            GL.glDeleteTextures(1, &m_TextureId);
            m_TextureId = 0;
        }
        if (m_FboId)
        {
            GL.glDeleteFramebuffers(1, &m_FboId);
            m_FboId = 0;
        }
    }

    GFSDK_SSAO_Texture_GL GetTexture() const
    {
        return GFSDK_SSAO_Texture_GL(GL_TEXTURE_2D, m_TextureId);
    }

    GLuint GetFramebuffer() const
    {
        return m_FboId;
    }

private:
    GLuint m_TextureId;
    GLuint m_FboId;
};

//--------------------------------------------------------------------------------
template<GLsizei ARRAY_SIZE>
class RTTexture2DArray : public BaseRTTexture
{
public:
    RTTexture2DArray()
        : BaseRTTexture()
        , m_TextureArrayId(0)
        , m_LayeredFboId(0)
    {
        ZERO_ARRAY(m_OctaSliceFbo);
        ASSERT(MAX_NUM_MRTS == 8);
        ASSERT(ARRAY_SIZE == 16);
    }

    void CreateOnce(const GFSDK_SSAO_GLFunctions& GL, GLsizei Width, GLsizei Height, GLint InternalFormat)
    {
        if (!m_TextureArrayId)
        {
            GL.glGenTextures(1, &m_TextureArrayId);
            GL.glBindTexture(GL_TEXTURE_2D_ARRAY, m_TextureArrayId);
            GL.glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, InternalFormat, Width, Height, ARRAY_SIZE, 0, GetBaseGLFormat(InternalFormat), GetBaseGLType(InternalFormat), NULL);
            GL.glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

            THROW_IF(GL.glGetError());

            m_AllocatedSizeInBytes = Width * Height * ARRAY_SIZE * GetFormatSizeInBytes(InternalFormat);
        }

        if (!m_LayeredFboId)
        {
            GL.glGenFramebuffers(1, &m_LayeredFboId);
            GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_LayeredFboId);
            GL.glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TextureArrayId, 0);
            GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            THROW_IF(GL.glGetError());
        }

        if (!m_OctaSliceFbo[0])
        {
            GLenum DrawBuffers[MAX_NUM_MRTS];
            for (UINT BufferIndex = 0; BufferIndex < MAX_NUM_MRTS; ++BufferIndex)
            {
                DrawBuffers[BufferIndex] = GL_COLOR_ATTACHMENT0 + BufferIndex;
            }

            GL.glGenFramebuffers(2, m_OctaSliceFbo);
            for (UINT PassIndex = 0; PassIndex < 2; ++PassIndex)
            {
                GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_OctaSliceFbo[PassIndex]);
                for (UINT RenderTargetId = 0; RenderTargetId < ARRAY_SIZE/2; ++RenderTargetId)
                {
                    UINT LayerIndex = PassIndex * MAX_NUM_MRTS + RenderTargetId;
                    ASSERT(LayerIndex < ARRAY_SIZE);
                    GL.glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + RenderTargetId, m_TextureArrayId, 0, LayerIndex);
                }
                GL.glDrawBuffers(MAX_NUM_MRTS, DrawBuffers);
                GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            }

            THROW_IF(GL.glGetError());
        }
    }

    void SafeRelease(const GFSDK_SSAO_GLFunctions& GL)
    {
        if (m_TextureArrayId)
        {
            GL.glDeleteTextures(1, &m_TextureArrayId);
            m_TextureArrayId = 0;
        }
        if (m_LayeredFboId)
        {
            GL.glDeleteFramebuffers(1, &m_LayeredFboId);
            m_LayeredFboId = 0;
        }
        if (m_OctaSliceFbo[0])
        {
            GL.glDeleteFramebuffers(SIZEOF_ARRAY(m_OctaSliceFbo), m_OctaSliceFbo);
            ZERO_ARRAY(m_OctaSliceFbo);
        }
    }

    GLuint GetTextureArray() const
    {
        return m_TextureArrayId;
    }

    GLuint GetLayeredFramebuffer() const
    {
        return m_LayeredFboId;
    }

    GLuint GetOctaSliceFramebuffer(UINT PassIndex) const
    {
        ASSERT(PassIndex < SIZEOF_ARRAY(m_OctaSliceFbo));
        return m_OctaSliceFbo[PassIndex];
    }

private:
    GLuint m_TextureArrayId;
    GLuint m_LayeredFboId;
    GLuint m_OctaSliceFbo[2];
};

//--------------------------------------------------------------------------------
class RenderTargets
{
public:
    RenderTargets()
        : m_FullWidth(0)
        , m_FullHeight(0)
    {
    }

    void ReleaseResources(const GFSDK_SSAO_GLFunctions& GL)
    {
        m_FullResAOZTexture.SafeRelease(GL);
        m_FullResAOZTexture2.SafeRelease(GL);
        m_FullResNormalTexture.SafeRelease(GL);
        m_FullResViewDepthTexture.SafeRelease(GL);
        m_QuarterResAOTextureArray.SafeRelease(GL);
        m_QuarterResViewDepthTextureArray.SafeRelease(GL);
    }

    void Release(const GFSDK_SSAO_GLFunctions& GL)
    {
        ReleaseResources(GL);
    }

    void SetFullResolution(GLsizei Width, GLsizei Height)
    {
        m_FullWidth = Width;
        m_FullHeight = Height;
    }

    UINT GetFullWidth()
    {
        return UINT(m_FullWidth);
    }

    UINT GetFullHeight()
    {
        return UINT(m_FullHeight);
    }

    const RTTexture2D* GetFullResAOZTexture(const GFSDK_SSAO_GLFunctions& GL)
    {
        m_FullResAOZTexture.CreateOnce(GL, m_FullWidth, m_FullHeight, GL_RG16F);
        return &m_FullResAOZTexture;
    }

    const RTTexture2D* GetFullResAOZTexture2(const GFSDK_SSAO_GLFunctions& GL)
    {
        m_FullResAOZTexture2.CreateOnce(GL, m_FullWidth, m_FullHeight, GL_RG16F);
        return &m_FullResAOZTexture2;
    }

    const RTTexture2D* GetFullResViewDepthTexture(const GFSDK_SSAO_GLFunctions& GL)
    {
        m_FullResViewDepthTexture.CreateOnce(GL, m_FullWidth, m_FullHeight, GL_R32F);
        return &m_FullResViewDepthTexture;
    }

    GLint GetViewDepthTextureFormat(GFSDK_SSAO_DepthStorage DepthStorage)
    {
        return (DepthStorage == GFSDK_SSAO_FP16_VIEW_DEPTHS) ? GL_R16F : GL_R32F;
    }

    const RTTexture2DArray<16>* GetQuarterResViewDepthTextureArray(const GFSDK_SSAO_GLFunctions& GL, const RenderOptions &Options)
    {
        m_QuarterResViewDepthTextureArray.CreateOnce(GL, iDivUp(m_FullWidth,4), iDivUp(m_FullHeight,4), GetViewDepthTextureFormat(Options.DepthStorage));
        return &m_QuarterResViewDepthTextureArray;
    }

    const RTTexture2DArray<16>* GetQuarterResAOTextureArray(const GFSDK_SSAO_GLFunctions& GL)
    {
        m_QuarterResAOTextureArray.CreateOnce(GL, iDivUp(m_FullWidth,4), iDivUp(m_FullHeight,4), GL_R8);
        return &m_QuarterResAOTextureArray;
    }

    const RTTexture2D* GetFullResNormalTexture(const GFSDK_SSAO_GLFunctions& GL)
    {
        m_FullResNormalTexture.CreateOnce(GL, m_FullWidth, m_FullHeight, GL_RGBA8);
        return &m_FullResNormalTexture;
    }

    void CreateOnceAll(const GFSDK_SSAO_GLFunctions& GL, const RenderOptions &Options)
    {
        GetFullResViewDepthTexture(GL);
        GetFullResNormalTexture(GL);
        GetQuarterResViewDepthTextureArray(GL, Options);
        GetQuarterResAOTextureArray(GL);

        if (Options.Blur.Enable)
        {
            GetFullResAOZTexture(GL);
            GetFullResAOZTexture2(GL);
        }
    }

    GFSDK_SSAO_Status PreCreate(const GFSDK_SSAO_GLFunctions& GL, const RenderOptions &Options)
    {
#if ENABLE_EXCEPTIONS
        try
        {
            CreateOnceAll(GL, Options);
        }
        catch (...)
        {
            ReleaseResources(GL);

            return GFSDK_SSAO_GL_RESOURCE_CREATION_FAILED;
        }
#else
        CreateOnceAll(GL, Options);
#endif

        return GFSDK_SSAO_OK;
    }

    UINT GetCurrentAllocatedVideoMemoryBytes()
    {
        return m_FullResAOZTexture.GetAllocatedSizeInBytes() +
               m_FullResAOZTexture2.GetAllocatedSizeInBytes() +
               m_FullResNormalTexture.GetAllocatedSizeInBytes() +
               m_FullResViewDepthTexture.GetAllocatedSizeInBytes() +
               m_QuarterResAOTextureArray.GetAllocatedSizeInBytes() +
               m_QuarterResViewDepthTextureArray.GetAllocatedSizeInBytes();
    }

private:
    GLsizei m_FullWidth;
    GLsizei m_FullHeight;

    RTTexture2D m_FullResAOZTexture;
    RTTexture2D m_FullResAOZTexture2;
    RTTexture2D m_FullResNormalTexture;
    RTTexture2D m_FullResViewDepthTexture;
    RTTexture2DArray<16> m_QuarterResAOTextureArray;
    RTTexture2DArray<16> m_QuarterResViewDepthTextureArray;
};

} // namespace GL
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_GL
