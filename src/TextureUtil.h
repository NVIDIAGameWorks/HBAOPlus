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

#if SUPPORT_D3D11

namespace D3D11
{

//--------------------------------------------------------------------------------
class TextureUtil
{
public:
    static void GetDesc(ID3D11RenderTargetView* pRTV, D3D11_TEXTURE2D_DESC* pTextureDesc)
    {
        ID3D11Texture2D* pBaseTexture;
        pRTV->GetResource((ID3D11Resource**)&pBaseTexture);

        pBaseTexture->GetDesc(pTextureDesc);
        SAFE_RELEASE(pBaseTexture);
    }
    static void GetDesc(ID3D11ShaderResourceView* pSRV, D3D11_TEXTURE2D_DESC* pTextureDesc)
    {
        ID3D11Texture2D* pBaseTexture;
        pSRV->GetResource((ID3D11Resource**)&pBaseTexture);

        pBaseTexture->GetDesc(pTextureDesc);
        SAFE_RELEASE(pBaseTexture);
    }
    static void GetDesc(ID3D11DepthStencilView* pDSV, D3D11_TEXTURE2D_DESC* pTextureDesc)
    {
        ID3D11Texture2D* pBaseTexture;
        pDSV->GetResource((ID3D11Resource**)&pBaseTexture);

        pBaseTexture->GetDesc(pTextureDesc);
        SAFE_RELEASE(pBaseTexture);
    }
};

} // namespace D3D11

#endif

#if SUPPORT_D3D12

namespace D3D12
{

//--------------------------------------------------------------------------------
class TextureUtil
{
public:
    static void GetDesc(ID3D12Resource* pResource, D3D12_RESOURCE_DESC* pResourceDesc)
    {
        ZeroMemory(pResourceDesc, sizeof(D3D12_RESOURCE_DESC));
        *pResourceDesc = pResource->GetDesc();
    }
};

} // namespace D3D12

#endif

#if SUPPORT_GL

namespace GL
{

//--------------------------------------------------------------------------------
struct TextureDesc2D
{
    TextureDesc2D()
        : Width(0)
        , Height(0)
        , SampleCount(1)
    {
    }

    GLint Width;
    GLint Height;
    GLint SampleCount;
};

//--------------------------------------------------------------------------------
class TextureUtil
{
public:
   static bool HasValidTextureTarget(GFSDK_SSAO_Texture_GL Texture)
   {
       return (Texture.Target == GL_TEXTURE_2D || Texture.Target == GL_TEXTURE_2D_MULTISAMPLE);
   }

   static void GetDesc2D(const GFSDK_SSAO_GLFunctions& GL, GFSDK_SSAO_Texture_GL Texture, TextureDesc2D* pTextureDesc)
   {
        ASSERT(HasValidTextureTarget(Texture));
        ASSERT_GL_ERROR(GL);

        GL.glBindTexture(Texture.Target, Texture.TextureId);
        ASSERT_GL_ERROR(GL);

        GL.glGetTexLevelParameteriv(Texture.Target, 0, GL_TEXTURE_WIDTH, &pTextureDesc->Width);
        GL.glGetTexLevelParameteriv(Texture.Target, 0, GL_TEXTURE_HEIGHT, &pTextureDesc->Height);
        ASSERT_GL_ERROR(GL);

        if (Texture.Target == GL_TEXTURE_2D_MULTISAMPLE)
        {
            GL.glGetTexLevelParameteriv(Texture.Target, 0, GL_TEXTURE_SAMPLES, &pTextureDesc->SampleCount);
            ASSERT_GL_ERROR(GL);
        }

        GL.glBindTexture(Texture.Target, 0);
        ASSERT_GL_ERROR(GL);
   }
};

} // namespace GL

#endif

} // namespace SSAO
} // namespace GFSDK
