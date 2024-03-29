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

#if SUPPORT_D3D11

#pragma once
#include "Common.h"

#if USE_NVAPI
#include "nvapi.h"
#endif

namespace GFSDK
{
namespace SSAO
{
namespace D3D11
{

//--------------------------------------------------------------------------------
class RTTexture2D
{
public:
    ID3D11Texture2D* pTexture;
    ID3D11RenderTargetView* pRTV;
    ID3D11ShaderResourceView* pSRV;
    UINT m_AllocatedSizeInBytes;

    RTTexture2D()
        : pTexture(NULL)
        , pRTV(NULL)
        , pSRV(NULL)
        , m_AllocatedSizeInBytes(0)
    {
    }

    UINT GetAllocatedSizeInBytes()
    {
        return m_AllocatedSizeInBytes;
    }

    UINT GetFormatSizeInBytes(DXGI_FORMAT Format)
    {
        UINT NumBytes = 0;
        switch (Format)
        {
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R32G32_FLOAT:
                NumBytes = 8;
                break;
            case DXGI_FORMAT_R32_FLOAT:
            case DXGI_FORMAT_R16G16_FLOAT:
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R11G11B10_FLOAT:
                NumBytes = 4;
                break;
            case DXGI_FORMAT_R16_FLOAT:
                NumBytes = 2;
                break;
            case DXGI_FORMAT_R8_UNORM:
                NumBytes = 1;
                break;
            default:
                ASSERT(0);
                break;
        }
        return NumBytes;
    }

    void CreateOnce(ID3D11Device* pDevice, UINT Width, UINT Height, DXGI_FORMAT Format, UINT ArraySize=1)
    {
        if (!pTexture)
        {
            D3D11_TEXTURE2D_DESC Desc;
            Desc.Width              = Width;
            Desc.Height             = Height;
            Desc.Format             = Format;
            Desc.MipLevels          = 1;
            Desc.ArraySize          = ArraySize;
            Desc.SampleDesc.Count   = 1;
            Desc.SampleDesc.Quality = 0;
            Desc.Usage              = D3D11_USAGE_DEFAULT;
            Desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            Desc.CPUAccessFlags     = 0;
            Desc.MiscFlags          = 0;

            THROW_IF_FAILED(pDevice->CreateTexture2D(&Desc, NULL, &pTexture));
            THROW_IF_FAILED(pDevice->CreateShaderResourceView(pTexture, NULL, &pSRV));
            THROW_IF_FAILED(pDevice->CreateRenderTargetView(pTexture, NULL, &pRTV));

#if USE_NVAPI
            NvAPI_Status Status = NvAPI_Initialize();
            NVDX_ObjectHandle NVHandle = nullptr;
            if (Status == NVAPI_OK)
            {
                Status = NvAPI_D3D_GetObjectHandleForResource(pDevice, pTexture, &NVHandle);
                if (NVHandle)
                {
                    NvU32 HintValue = 1;
                    Status = NvAPI_D3D_SetResourceHint(pDevice, NVHandle, NVAPI_D3D_SRH_CATEGORY_SLI, NVAPI_D3D_SRH_SLI_APP_CONTROLLED_INTERFRAME_CONTENT_SYNC, &HintValue);
                }
            }
#endif

            m_AllocatedSizeInBytes = Width * Height * ArraySize * GetFormatSizeInBytes(Format);
        }
    }

    void SafeRelease()
    {
        SAFE_RELEASE(pTexture);
        SAFE_RELEASE(pRTV);
        SAFE_RELEASE(pSRV);

        m_AllocatedSizeInBytes = 0;
    }

    static UINT FormatSizeInBytes(DXGI_FORMAT Format);
};

//--------------------------------------------------------------------------------
template<UINT ARRAY_SIZE>
class RTTexture2DArray : public RTTexture2D
{
public:
    ID3D11RenderTargetView* pRTVs[ARRAY_SIZE];
    ID3D11ShaderResourceView* pSRVs[ARRAY_SIZE];

    RTTexture2DArray()
        : RTTexture2D()
    {
        ZERO_ARRAY(pRTVs);
        ZERO_ARRAY(pSRVs);
    }

    void CreateOnce(ID3D11Device* pDevice, UINT Width, UINT Height, DXGI_FORMAT Format);
    void SafeRelease();
};

//--------------------------------------------------------------------------------
template<UINT ARRAY_SIZE>
void RTTexture2DArray<ARRAY_SIZE>::CreateOnce(ID3D11Device* pDevice, UINT Width, UINT Height, DXGI_FORMAT Format)
{
    if (!pTexture)
    {
        RTTexture2D::CreateOnce(pDevice, Width, Height, Format, ARRAY_SIZE);

        for (UINT SliceId = 0; SliceId < SIZEOF_ARRAY(pRTVs); ++SliceId)
        {
            D3D11_RENDER_TARGET_VIEW_DESC Desc;
            Desc.Format = Format;
            Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            Desc.Texture2DArray.MipSlice = 0;
            Desc.Texture2DArray.FirstArraySlice = SliceId;
            Desc.Texture2DArray.ArraySize = 1;

            THROW_IF_FAILED(pDevice->CreateRenderTargetView(pTexture, &Desc, &pRTVs[SliceId]));
        }

        for (UINT SliceId = 0; SliceId < SIZEOF_ARRAY(pSRVs); ++SliceId)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC Desc;
            Desc.Format = Format;
            Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            Desc.Texture2DArray.MostDetailedMip = 0;
            Desc.Texture2DArray.MipLevels = 1;
            Desc.Texture2DArray.ArraySize = 1;
            Desc.Texture2DArray.FirstArraySlice = SliceId;

            THROW_IF_FAILED(pDevice->CreateShaderResourceView(pTexture, &Desc, &pSRVs[SliceId]));
        }
    }
}

//--------------------------------------------------------------------------------
template<UINT ARRAY_SIZE>
void RTTexture2DArray<ARRAY_SIZE>::SafeRelease()
{
    RTTexture2D::SafeRelease();

    SAFE_RELEASE_ARRAY(pRTVs);
    SAFE_RELEASE_ARRAY(pSRVs);
}

//--------------------------------------------------------------------------------
class RenderTargets
{
public:
    RenderTargets() :
          m_pDevice(NULL)
        , m_FullWidth(0)
        , m_FullHeight(0)
    {
    }

    void ReleaseResources()
    {
        m_FullResAOZTexture.SafeRelease();
        m_FullResAOZTexture2.SafeRelease();
        m_FullResNormalTexture.SafeRelease();
        m_FullResViewDepthTexture.SafeRelease();
        m_FullResViewDepthTexture2.SafeRelease();
        m_QuarterResAOTextureArray.SafeRelease();
        m_QuarterResViewDepthTextureArray.SafeRelease();
    }

    void Release()
    {
        ReleaseResources();
        SAFE_RELEASE(m_pDevice);
    }

    void SetDevice(ID3D11Device* pDevice)
    {
        m_pDevice = pDevice;
        m_pDevice->AddRef();
    }

    void SetFullResolution(UINT Width, UINT Height)
    {
        m_FullWidth = Width;
        m_FullHeight = Height;
    }

    UINT GetFullWidth()
    {
        return m_FullWidth;
    }

    UINT GetFullHeight()
    {
        return m_FullHeight;
    }

    const RTTexture2D* GetFullResAOZTexture()
    {
        m_FullResAOZTexture.CreateOnce(m_pDevice, m_FullWidth, m_FullHeight, DXGI_FORMAT_R16G16_FLOAT);
        return &m_FullResAOZTexture;
    }

    const RTTexture2D* GetFullResAOZTexture2()
    {
        m_FullResAOZTexture2.CreateOnce(m_pDevice, m_FullWidth, m_FullHeight, DXGI_FORMAT_R16G16_FLOAT);
        return &m_FullResAOZTexture2;
    }

    const RTTexture2D* GetFullResViewDepthTexture()
    {
        m_FullResViewDepthTexture.CreateOnce(m_pDevice, m_FullWidth, m_FullHeight, DXGI_FORMAT_R32_FLOAT);
        return &m_FullResViewDepthTexture;
    }

    const RTTexture2D* GetFullResViewDepthTexture2()
    {
        m_FullResViewDepthTexture2.CreateOnce(m_pDevice, m_FullWidth, m_FullHeight, DXGI_FORMAT_R32_FLOAT);
        return &m_FullResViewDepthTexture2;
    }

    DXGI_FORMAT GetViewDepthTextureFormat(GFSDK_SSAO_DepthStorage DepthStorage)
    {
        return (DepthStorage == GFSDK_SSAO_FP16_VIEW_DEPTHS) ? DXGI_FORMAT_R16_FLOAT : DXGI_FORMAT_R32_FLOAT;
    }

    const RTTexture2DArray<16>* GetQuarterResViewDepthTextureArray(const GFSDK_SSAO_Parameters &Options)
    {
        m_QuarterResViewDepthTextureArray.CreateOnce(m_pDevice, iDivUp(m_FullWidth,4), iDivUp(m_FullHeight,4), Options.EnableDualLayerAO ? DXGI_FORMAT_R16G16_FLOAT : GetViewDepthTextureFormat(Options.DepthStorage));
        return &m_QuarterResViewDepthTextureArray;
    }

    const RTTexture2DArray<16>* GetQuarterResAOTextureArray()
    {
        m_QuarterResAOTextureArray.CreateOnce(m_pDevice, iDivUp(m_FullWidth,4), iDivUp(m_FullHeight,4), DXGI_FORMAT_R8_UNORM);
        return &m_QuarterResAOTextureArray;
    }

    const RTTexture2D* GetFullResNormalTexture()
    {
        m_FullResNormalTexture.CreateOnce(m_pDevice, m_FullWidth, m_FullHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
        return &m_FullResNormalTexture;
    }

    void CreateOnceAll(const GFSDK_SSAO_Parameters &Options)
    {
        GetFullResViewDepthTexture();
        GetFullResNormalTexture();
        GetQuarterResViewDepthTextureArray(Options);
        GetQuarterResAOTextureArray();

        if (Options.EnableDualLayerAO)
        {
            GetFullResViewDepthTexture2();
        }

        if (Options.Blur.Enable)
        {
            GetFullResAOZTexture();
            GetFullResAOZTexture2();
        }
    }

    GFSDK_SSAO_Status PreCreate(const GFSDK_SSAO_Parameters &Options)
    {
#if ENABLE_EXCEPTIONS
        try
        {
            CreateOnceAll(Options);
        }
        catch (...)
        {
            ReleaseResources();

            return GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED;
        }
#else
        CreateOnceAll(Options);
#endif

        return GFSDK_SSAO_OK;
    }

    UINT GetCurrentAllocatedVideoMemoryBytes()
    {
        return m_FullResAOZTexture.GetAllocatedSizeInBytes() +
               m_FullResAOZTexture2.GetAllocatedSizeInBytes() +
               m_FullResNormalTexture.GetAllocatedSizeInBytes() +
               m_FullResViewDepthTexture.GetAllocatedSizeInBytes() +
               m_FullResViewDepthTexture2.GetAllocatedSizeInBytes() +
               m_QuarterResAOTextureArray.GetAllocatedSizeInBytes() +
               m_QuarterResViewDepthTextureArray.GetAllocatedSizeInBytes();
    }

private:
    ID3D11Device* m_pDevice;
    UINT m_FullWidth;
    UINT m_FullHeight;
    RTTexture2D m_FullResAOZTexture;
    RTTexture2D m_FullResAOZTexture2;
    RTTexture2D m_FullResNormalTexture;
    RTTexture2D m_FullResViewDepthTexture;
    RTTexture2D m_FullResViewDepthTexture2;
    RTTexture2DArray<16> m_QuarterResAOTextureArray;
    RTTexture2DArray<16> m_QuarterResViewDepthTextureArray;
};

} // namespace D3D11
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_D3D11
