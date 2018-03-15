/* 
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved. 
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

} // namespace SSAO
} // namespace GFSDK
