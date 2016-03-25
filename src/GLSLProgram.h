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
#include "ConstantBuffers.h"

#if _DEBUG
#include <stdio.h>
#endif

#define GLSL_VERSION_150\
  (char *)"#version 150\n"

#define GLSL_VERSION_300ES\
  (char *)"#version 300 es\n#extension GL_EXT_geometry_shader : enable\n"

namespace GFSDK
{
namespace SSAO
{
namespace GL
{

//--------------------------------------------------------------------------
class GLSLProgram
{
public:
    GLSLProgram()
        : m_Program(0)
        , m_VS(0)
        , m_GS(0)
        , m_FS(0)
    {
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *pVertexShaderGLSL, const char *pFragmentShaderGLSL)
    {
        m_Program = CompileProgram(GL, pVertexShaderGLSL, pFragmentShaderGLSL);
    }

    void Create(const GFSDK_SSAO_GLFunctions& GL, const char *pVertexShaderGLSL, const char *pGeometryShaderGLSL, const char *pFragmentShaderGLSL)
    {
        m_Program = CompileProgram(GL, pVertexShaderGLSL, pGeometryShaderGLSL, pFragmentShaderGLSL);
    }

    static void DeleteShader(const GFSDK_SSAO_GLFunctions& GL, GLuint& Shader)
    {
        if (Shader)
        {
            GL.glDeleteShader(Shader);
            Shader = 0;
        }
    }

    static void DeleteProgram(const GFSDK_SSAO_GLFunctions& GL, GLuint& Program)
    {
        if (Program)
        {
            GL.glDeleteProgram(Program);
            Program = 0;
        }
    }

    void Release(const GFSDK_SSAO_GLFunctions& GL)
    {
        DeleteShader(GL, m_VS);
        DeleteShader(GL, m_GS);
        DeleteShader(GL, m_FS);
        DeleteProgram(GL, m_Program);
    }

    void Enable(const GFSDK_SSAO_GLFunctions& GL)
    {
        GL.glUseProgram(m_Program);
    }

protected:
    static void CheckCompileError(const GFSDK_SSAO_GLFunctions& GL, GLuint Shader)
    {
        GLint Compiled = 0;
        GL.glGetShaderiv(Shader, GL_COMPILE_STATUS, &Compiled);
        ASSERT_GL_ERROR(GL);

#if _DEBUG
        char temp[1024];
        GL.glGetShaderInfoLog(Shader, sizeof(temp), 0, temp);
        if (!Compiled || strstr(temp, "deprecated"))
        {
            fprintf(stderr, "%s\n", temp);
            ASSERT(0);
        }
#endif

        THROW_IF(!Compiled);
    }

    static void CheckLinkError(const GFSDK_SSAO_GLFunctions& GL, GLuint Program)
    {
        GLint Linked = 0;
        GL.glGetProgramiv(Program, GL_LINK_STATUS, &Linked);
        ASSERT_GL_ERROR(GL);

#if _DEBUG
        char temp[1024];
        GL.glGetProgramInfoLog(Program, sizeof(temp), 0, temp);
        if (!Linked || strstr(temp, "deprecated"))
        {
            fprintf(stderr, "%s\n", temp);
            ASSERT(0);
        }
#endif

        THROW_IF(!Linked);
    }

    static GLuint AttachShader(const GFSDK_SSAO_GLFunctions& GL, GLuint Program, GLenum ShaderType, const char *ShaderSource)
    {
        GLuint Shader = GL.glCreateShader(ShaderType);

        const GLchar *VersionDirective = "#version";
        if (!strncmp(ShaderSource, VersionDirective, strlen(VersionDirective)))
        {
            GL.glShaderSource(Shader, 1, &ShaderSource, 0);
        }
        else
        {
#if USE_GLES
            const GLchar* ShaderVersion = GLSL_VERSION_300ES;
#else
            const GLchar* ShaderVersion = GLSL_VERSION_150;
#endif
            const GLchar* Strings[] = { ShaderVersion, ShaderSource };
            GL.glShaderSource(Shader, SIZEOF_ARRAY(Strings), Strings, 0);
        }

        GL.glCompileShader(Shader);
        CheckCompileError(GL, Shader);

        GL.glAttachShader(Program, Shader);

        return Shader;
    }

    GLuint CompileProgram(const GFSDK_SSAO_GLFunctions& GL, const char *pVertexShaderGLSL, const char *pFragmentShaderGLSL)
    {
        GLuint Program = GL.glCreateProgram();
        m_VS = AttachShader(GL, Program, GL_VERTEX_SHADER, pVertexShaderGLSL);
        m_FS = AttachShader(GL, Program, GL_FRAGMENT_SHADER, pFragmentShaderGLSL);

        for (GLuint ColorIndex = 0; ColorIndex < 8; ++ColorIndex)
        {
            char VariableName[32];
            snprintf(VariableName, sizeof(VariableName), "PixOutput%d", ColorIndex);
            GL.glBindFragDataLocation(Program, ColorIndex, VariableName);
            ASSERT_GL_ERROR(GL);
        }

        GL.glLinkProgram(Program);
        CheckLinkError(GL, Program);

        return Program;
    }

    GLuint CompileProgram(const GFSDK_SSAO_GLFunctions& GL, const char *pVertexShaderGLSL, const char* pGeometryShaderGLSL, const char *pFragmentShaderGLSL)
    {
        GLuint Program = GL.glCreateProgram();
        m_VS = AttachShader(GL, Program, GL_VERTEX_SHADER, pVertexShaderGLSL);
        m_GS = AttachShader(GL, Program, GL_GEOMETRY_SHADER, pGeometryShaderGLSL);
        m_FS = AttachShader(GL, Program, GL_FRAGMENT_SHADER, pFragmentShaderGLSL);

        GL.glLinkProgram(Program);
        CheckLinkError(GL, Program);

        return Program;
    }

    GLint GetUniformLocation(const GFSDK_SSAO_GLFunctions& GL, const char* pUniform)
    {
        GLint Result = GL.glGetUniformLocation(m_Program, pUniform);
        return Result;
    }

    GLuint GetUniformBlockIndex(const GFSDK_SSAO_GLFunctions& GL, const char* UniformBlock)
    {
        GLuint Result = GL.glGetUniformBlockIndex(m_Program, UniformBlock);
        return Result;
    }

    void SetTexture(const GFSDK_SSAO_GLFunctions& GL, GLenum Target, GLint UniformLocation, GLint TexId, GLint TexUnit, GLint Filter = GL_NEAREST, GLint WrapMode = GL_CLAMP_TO_EDGE)
    {
        ASSERT(UniformLocation != -1);

        GL.glUseProgram(m_Program);
        GL.glUniform1i(UniformLocation, TexUnit);

        GL.glActiveTexture(GL_TEXTURE0 + TexUnit);
        GL.glBindTexture(Target, TexId);

        if (Target != GL_TEXTURE_2D_MULTISAMPLE)
        {
            GL.glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, Filter);
            GL.glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, Filter);
            GL.glTexParameteri(Target, GL_TEXTURE_WRAP_S, WrapMode);
            GL.glTexParameteri(Target, GL_TEXTURE_WRAP_T, WrapMode);
        }

        if (WrapMode == GL_CLAMP_TO_BORDER)
        {
            const GLfloat BorderColor[4] = { -FLT_MAX };
            GL.glTexParameterfv(Target, GL_TEXTURE_BORDER_COLOR, BorderColor);
        }

        ASSERT_GL_ERROR(GL);
    }

protected:
    GLuint m_Program;
    GLuint m_VS;
    GLuint m_GS;
    GLuint m_FS;
};

} // namespace GL
} // namespace SSAO
} // namespace GFSDK
