/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if SUPPORT_D3D11
#include "Renderer_DX11.h"
#endif

#if SUPPORT_D3D12
#include "Renderer_DX12.h"
#endif

#if SUPPORT_GL
#include "Renderer_GL.h"
#endif

//--------------------------------------------------------------------------------
inline static
GFSDK_SSAO_CustomHeap InitNewDelete(const GFSDK_SSAO_CustomHeap* pCustomHeap)
{
    GFSDK_SSAO_CustomHeap NewDelete;

    if (pCustomHeap)
    {
        NewDelete = *pCustomHeap;
    }
    else
    {
        NewDelete.new_ = ::operator new;
        NewDelete.delete_ = ::operator delete;
    }

    return NewDelete;
}

//--------------------------------------------------------------------------------
inline static
void* AllocateAndZeroMemory(const GFSDK_SSAO_CustomHeap& NewDelete, std::size_t SizeInBytes)
{
    void *pStorage = NewDelete.new_(SizeInBytes);
    if (!pStorage)
    {
        return NULL;
    }

    memset(pStorage, 0, SizeInBytes);

    return pStorage;
}

#if SUPPORT_D3D11

//--------------------------------------------------------------------------------
GFSDK_SSAO_DECL(GFSDK_SSAO_Status, GFSDK_SSAO_CreateContext_D3D11,
    ID3D11Device* pD3DDevice,
    GFSDK_SSAO_Context_D3D11** ppAOContext,
    const GFSDK_SSAO_CustomHeap* pCustomHeap,
    GFSDK_SSAO_Version HeaderVersion)
{
    if (!ppAOContext)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    GFSDK_SSAO_CustomHeap NewDelete = InitNewDelete(pCustomHeap);

    void *pStorage = AllocateAndZeroMemory(NewDelete, sizeof(GFSDK::SSAO::D3D11::Renderer));
    if (!pStorage)
    {
        return GFSDK_SSAO_MEMORY_ALLOCATION_FAILED;
    }

    GFSDK::SSAO::D3D11::Renderer* pAOContext = new(pStorage) GFSDK::SSAO::D3D11::Renderer(NewDelete);

    *ppAOContext = pAOContext;

    return pAOContext->Create(pD3DDevice, HeaderVersion);
}

#endif

#if SUPPORT_GL

//--------------------------------------------------------------------------------
inline static
bool CheckGLFunctionPointers(const GFSDK_SSAO_GLFunctions& GL)
{
    return (
        GL.glActiveTexture &&
        GL.glAttachShader &&
        GL.glBindBuffer &&
        GL.glBindBufferBase &&
        GL.glBindFramebuffer &&
        GL.glBindFragDataLocation &&
        GL.glBindTexture &&
        GL.glBindVertexArray &&
        GL.glBlendColor &&
        GL.glBlendEquationSeparate &&
        GL.glBlendFuncSeparate &&
        GL.glBufferData &&
        GL.glBufferSubData &&
        GL.glColorMaski &&
        GL.glCompileShader &&
        GL.glCreateShader &&
        GL.glCreateProgram &&
        GL.glDeleteBuffers &&
        GL.glDeleteFramebuffers &&
        GL.glDeleteProgram &&
        GL.glDeleteShader &&
        GL.glDeleteTextures &&
        GL.glDeleteVertexArrays &&
        GL.glDisable &&
        GL.glDrawBuffers &&
        GL.glEnable &&
        GL.glDrawArrays &&
        GL.glFramebufferTexture &&
        GL.glFramebufferTexture2D &&
        GL.glFramebufferTextureLayer &&
        GL.glGenBuffers &&
        GL.glGenFramebuffers &&
        GL.glGenTextures &&
        GL.glGenVertexArrays &&
        GL.glGetError &&
        GL.glGetBooleani_v &&
        GL.glGetFloatv &&
        GL.glGetIntegerv &&
        GL.glGetIntegeri_v &&
        GL.glGetProgramiv &&
        GL.glGetProgramInfoLog &&
        GL.glGetShaderiv &&
        GL.glGetShaderInfoLog &&
        GL.glGetString &&
        GL.glGetUniformBlockIndex &&
        GL.glGetUniformLocation &&
        GL.glGetTexLevelParameteriv &&
        GL.glIsEnabled &&
        GL.glIsEnabledi &&
        GL.glLinkProgram &&
        GL.glPolygonOffset &&
        GL.glShaderSource &&
        GL.glTexImage2D &&
        GL.glTexImage3D &&
        GL.glTexParameteri &&
        GL.glUniform1i &&
        GL.glUniformBlockBinding &&
        GL.glUseProgram &&
        GL.glViewport);
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_DECL(GFSDK_SSAO_Status, GFSDK_SSAO_CreateContext_GL,
    GFSDK_SSAO_Context_GL** ppAOContext,
    const GFSDK_SSAO_GLFunctions* pGLFunctions,
    const GFSDK_SSAO_CustomHeap* pCustomHeap,
    GFSDK_SSAO_Version HeaderVersion)
{
    if (!ppAOContext)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    if (!pGLFunctions)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    if (!CheckGLFunctionPointers(*pGLFunctions))
    {
        return GFSDK_SSAO_GL_NULL_FUNCTION_POINTER;
    }

    GFSDK_SSAO_CustomHeap NewDelete = InitNewDelete(pCustomHeap);

    void *pStorage = AllocateAndZeroMemory(NewDelete, sizeof(GFSDK::SSAO::GL::Renderer));
    if (!pStorage)
    {
        return GFSDK_SSAO_MEMORY_ALLOCATION_FAILED;
    }

    GFSDK::SSAO::GL::Renderer* pAOContext = new(pStorage) GFSDK::SSAO::GL::Renderer(*pGLFunctions, NewDelete);

    *ppAOContext = pAOContext;

    return pAOContext->Create(HeaderVersion);
}

#endif

#if SUPPORT_D3D12

//--------------------------------------------------------------------------------
GFSDK_SSAO_DECL(GFSDK_SSAO_Status, GFSDK_SSAO_CreateContext_D3D12,
    ID3D12Device* pD3DDevice,
    GFSDK_SSAO_UINT NodeMask,
    const GFSDK_SSAO_DescriptorHeaps_D3D12& DescriptorHeaps,
    GFSDK_SSAO_Context_D3D12** ppAOContext,
    const GFSDK_SSAO_CustomHeap* pCustomHeap,
    GFSDK_SSAO_Version HeaderVersion)
{
    if (!ppAOContext)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    GFSDK_SSAO_CustomHeap NewDelete = InitNewDelete(pCustomHeap);

    void *pStorage = AllocateAndZeroMemory(NewDelete, sizeof(GFSDK::SSAO::D3D12::Renderer));
    if (!pStorage)
    {
        return GFSDK_SSAO_MEMORY_ALLOCATION_FAILED;
    }

    GFSDK::SSAO::D3D12::Renderer* pAOContext = new(pStorage) GFSDK::SSAO::D3D12::Renderer(NewDelete);

    *ppAOContext = pAOContext;

    return pAOContext->Create(pD3DDevice, NodeMask, DescriptorHeaps, HeaderVersion);
}

#endif

GFSDK_SSAO_DECL(GFSDK_SSAO_Status, GFSDK_SSAO_GetVersion,
    GFSDK_SSAO_Version* pVersion)
{
    if (!pVersion)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    GFSDK::SSAO::BuildVersion BuildVersion;
    *pVersion = BuildVersion;

    return GFSDK_SSAO_OK;
}
