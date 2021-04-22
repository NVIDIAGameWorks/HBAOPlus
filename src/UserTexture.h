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
#include "TextureUtil.h"

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
struct UserTextureDesc
{
    UserTextureDesc()
      : Width(0)
      , Height(0)
      , SampleCount(0)
    {
    }
    UINT Width;
    UINT Height;
    UINT SampleCount;
};

//--------------------------------------------------------------------------------
#if SUPPORT_D3D11
namespace D3D11
{

struct UserTextureSRV : public SSAO::UserTextureDesc
{
    UserTextureSRV()
        : pSRV(NULL)
    {
    }

    GFSDK_SSAO_Status Init(ID3D11ShaderResourceView* pInputSRV)
    {
        if (!pInputSRV)
        {
            return GFSDK_SSAO_NULL_ARGUMENT;
        }

        D3D11_TEXTURE2D_DESC TextureDesc;
        GFSDK::SSAO::D3D11::TextureUtil::GetDesc(pInputSRV, &TextureDesc);

        pSRV = pInputSRV;
        Width = TextureDesc.Width;
        Height = TextureDesc.Height;
        SampleCount = TextureDesc.SampleDesc.Count;

        return GFSDK_SSAO_OK;
    }

    bool IsSet()
    {
        return (pSRV != NULL);
    }

    ID3D11ShaderResourceView* pSRV;
};

struct UserTextureRTV : public SSAO::UserTextureDesc
{
    UserTextureRTV()
        : pRTV(NULL)
    {
    }

    GFSDK_SSAO_Status Init(ID3D11RenderTargetView* pInputRTV)
    {
        if (!pInputRTV)
        {
            return GFSDK_SSAO_NULL_ARGUMENT;
        }

        D3D11_TEXTURE2D_DESC TextureDesc;
        GFSDK::SSAO::D3D11::TextureUtil::GetDesc(pInputRTV, &TextureDesc);

        pRTV = pInputRTV;
        Width = TextureDesc.Width;
        Height = TextureDesc.Height;
        SampleCount = TextureDesc.SampleDesc.Count;

        return GFSDK_SSAO_OK;
    }

    bool IsSet()
    {
        return (pRTV != NULL);
    }

    ID3D11RenderTargetView* pRTV;
};

struct UserTextureDSV : public SSAO::UserTextureDesc
{
    UserTextureDSV()
        : pDSV(NULL)
    {
    }

    GFSDK_SSAO_Status Init(ID3D11DepthStencilView* pInputDSV)
    {
        if (!pInputDSV)
        {
            return GFSDK_SSAO_NULL_ARGUMENT;
        }

        D3D11_TEXTURE2D_DESC TextureDesc;
        GFSDK::SSAO::D3D11::TextureUtil::GetDesc(pInputDSV, &TextureDesc);

        pDSV = pInputDSV;
        Width = TextureDesc.Width;
        Height = TextureDesc.Height;
        SampleCount = TextureDesc.SampleDesc.Count;

        return GFSDK_SSAO_OK;
    }

    bool IsSet()
    {
        return (pDSV != NULL);
    }

    ID3D11DepthStencilView* pDSV;
};

} // namespace D3D11
#endif // SUPPORT_D3D11

//--------------------------------------------------------------------------------
#if SUPPORT_D3D12
namespace D3D12
{

struct UserTextureSRV : public SSAO::UserTextureDesc
{
    UserTextureSRV()
    {
        SRV.pResource = NULL;
        SRV.GpuHandle.ptr = ~0ull;
    }

    GFSDK_SSAO_Status Init(const GFSDK_SSAO_ShaderResourceView_D3D12* pInputSRV)
    {
        if (!pInputSRV)
        {
            return GFSDK_SSAO_NULL_ARGUMENT;
        }

        D3D12_RESOURCE_DESC TextureDesc;
        GFSDK::SSAO::D3D12::TextureUtil::GetDesc(pInputSRV->pResource, &TextureDesc);

        SRV = *pInputSRV;
        Width = (UINT)TextureDesc.Width;
        Height = (UINT)TextureDesc.Height;
        SampleCount = TextureDesc.SampleDesc.Count;

        return GFSDK_SSAO_OK;
    }

    bool IsSet()
    {
        return (SRV.pResource != NULL);
    }

    ShaderResourceView SRV;
};

struct UserTextureRTV : public SSAO::UserTextureDesc
{
    UserTextureRTV()
        : Format(DXGI_FORMAT_UNKNOWN)
    {
    }

    GFSDK_SSAO_Status Init(const GFSDK_SSAO_RenderTargetView_D3D12* pRenderTargetView)
    {
        if (!pRenderTargetView || !pRenderTargetView->CpuHandle)
        {
            return GFSDK_SSAO_NULL_ARGUMENT;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE Handle = { pRenderTargetView->CpuHandle };
        D3D12_RESOURCE_DESC ResourceDesc;
        SSAO::D3D12::TextureUtil::GetDesc(pRenderTargetView->pResource, &ResourceDesc);

        RTV = *pRenderTargetView;
        Width = (UINT)ResourceDesc.Width;
        Height = (UINT)ResourceDesc.Height;
        SampleCount = ResourceDesc.SampleDesc.Count;
        Format = ResourceDesc.Format;

        return GFSDK_SSAO_OK;
    }

    RenderTargetView RTV;
    DXGI_FORMAT Format;
};

} // namespace D3D12
#endif // SUPPORT_D3D12

} // namespace SSAO
} // namespace GFSDK
