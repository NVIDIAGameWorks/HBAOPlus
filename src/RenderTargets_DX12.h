/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if SUPPORT_D3D12

#pragma once
#include "Common.h"
#include "RenderOptions.h"

namespace GFSDK
{
namespace SSAO
{
namespace D3D12
{

//--------------------------------------------------------------------------------
class RTTexture2D
{
public:
    ID3D12Resource* pResource; //  ID3D12Texture2D* pTexture;
    ShaderResourceView SRV;
    RenderTargetView RTV;

    UINT m_AllocatedSizeInBytes;

    RTTexture2D()
        : pResource(nullptr)
        , m_AllocatedSizeInBytes(0)
    {
        RTV.pResource = NULL;
        SRV.pResource = NULL;
#if _WIN64
        RTV.CpuHandle.ptr = ~0ull;
        SRV.GpuHandle.ptr = ~0ull;
#else
        RTV.CpuHandle.ptr = ~0ul;
        SRV.GpuHandle.ptr = ~0ul;
#endif
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

    void CreateOnce(GFSDK_D3D12_GraphicsContext* pContext, UINT Width, UINT Height, DXGI_FORMAT Format, UINT BaseSRVHeapIndex, UINT BaseRTVHeapIndex, UINT ArraySize = 1)
    {
        if (!pResource)
        {
            // Create a texture 2D
            D3D12_RESOURCE_DESC Desc = {};
            Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            Desc.Width = Width;
            Desc.Height = Height;
            Desc.MipLevels = 1;
            Desc.DepthOrArraySize = ArraySize;
            Desc.SampleDesc.Count = 1;
            Desc.SampleDesc.Quality = 0;
            Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            Desc.Format = Format;
            Desc.Alignment = 0;

            THROW_IF_FAILED(pContext->pDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, pContext->NodeMask, pContext->NodeMask), D3D12_HEAP_FLAG_NONE,
                &Desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pResource)));

            // SRV
            SRV.pResource = pResource;
            D3D12_SHADER_RESOURCE_VIEW_DESC DescSRV = {};
            DescSRV.Format = Format;
            if (ArraySize > 1)
            {
                DescSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                DescSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                DescSRV.Texture2DArray.ArraySize = ArraySize;
                DescSRV.Texture2DArray.FirstArraySlice = 0;
                DescSRV.Texture2DArray.MipLevels = 1;
                DescSRV.Texture2DArray.MostDetailedMip = 0;
                DescSRV.Texture2DArray.PlaneSlice = 0;
                DescSRV.Texture2DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                DescSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                DescSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                DescSRV.Texture2D.MipLevels = 1;
                DescSRV.Texture2D.MostDetailedMip = 0; // No MIP
                DescSRV.Texture2D.PlaneSlice = 0;
                DescSRV.Texture2D.ResourceMinLODClamp = 0.0f;
            }
            D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = pContext->DescHeaps.CBVSRVUAV.GetCPUHandle(BaseSRVHeapIndex);
            SRV.GpuHandle = pContext->DescHeaps.CBVSRVUAV.GetGPUHandle(BaseSRVHeapIndex);
            pContext->pDevice->CreateShaderResourceView(pResource, &DescSRV, CpuHandle);

            // RTV
            RTV.pResource = pResource;
            D3D12_RENDER_TARGET_VIEW_DESC DescRTV = {};
            DescRTV.Format = Format;
            if (ArraySize > 1)
            {
                DescRTV.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                DescRTV.Texture2DArray.ArraySize = ArraySize;
                DescRTV.Texture2DArray.FirstArraySlice = 0;
                DescRTV.Texture2DArray.MipSlice = 0;
                DescRTV.Texture2DArray.PlaneSlice = 0;
            }
            else
            {
                DescRTV.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                DescRTV.Texture2D.MipSlice = 0;
                DescRTV.Texture2D.PlaneSlice = 0;
            }

            RTV.CpuHandle = pContext->DescHeaps.RTV.GetCPUHandle(BaseRTVHeapIndex);

            // Create the render target view
            pContext->pDevice->CreateRenderTargetView(pResource, &DescRTV, RTV.CpuHandle);

            m_AllocatedSizeInBytes = Width * Height * ArraySize * GetFormatSizeInBytes(Format);
        }
    }

    void SafeRelease()
    {
        SAFE_RELEASE(pResource);

        m_AllocatedSizeInBytes = 0;
    }
};

//--------------------------------------------------------------------------------
template<UINT ARRAY_SIZE>
class RTTexture2DArray : public RTTexture2D
{
public:
    ShaderResourceView SRVs[ARRAY_SIZE];
    RenderTargetView RTVs[ARRAY_SIZE];

    RTTexture2DArray()
        : RTTexture2D()
    {
        ZERO_ARRAY(SRVs);
        ZERO_ARRAY(RTVs);
    }

    void CreateOnce(GFSDK_D3D12_GraphicsContext* pContext, UINT Width, UINT Height, DXGI_FORMAT Format, UINT BaseSRVHeapIndex, UINT BaseRTVHeapIndex);
    void SafeRelease();
};

//--------------------------------------------------------------------------------
template<UINT ARRAY_SIZE>
void RTTexture2DArray<ARRAY_SIZE>::CreateOnce(GFSDK_D3D12_GraphicsContext* pContext, UINT Width, UINT Height, DXGI_FORMAT Format, UINT BaseSRVHeapIndex, UINT BaseRTVHeapIndex)
{
    if (!pResource)
    {
        RTTexture2D::CreateOnce(pContext, Width, Height, Format, BaseSRVHeapIndex, BaseRTVHeapIndex, ARRAY_SIZE);

        for (UINT SliceId = 0; SliceId < SIZEOF_ARRAY(RTVs); ++SliceId)
        {
            RTVs[SliceId].pResource = pResource;
#if _WIN64
            RTVs[SliceId].CpuHandle.ptr = ~0ull;
#else
            RTVs[SliceId].CpuHandle.ptr = ~0ul;
#endif

            D3D12_RENDER_TARGET_VIEW_DESC Desc = {};
            Desc.Format = Format;
            Desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            Desc.Texture2DArray.MipSlice = 0;
            Desc.Texture2DArray.FirstArraySlice = SliceId;
            Desc.Texture2DArray.ArraySize = 1;

            RTVs[SliceId].CpuHandle = pContext->DescHeaps.RTV.GetCPUHandle(BaseRTVHeapIndex + 1 + SliceId);
            pContext->pDevice->CreateRenderTargetView(pResource, &Desc, RTVs[SliceId].CpuHandle);
        }

        for (UINT SliceId = 0; SliceId < SIZEOF_ARRAY(SRVs); ++SliceId)
        {
            SRVs[SliceId].pResource = pResource;
            //SRVs[SliceId].CpuHandle.ptr = ~0ull;
            SRVs[SliceId].GpuHandle.ptr = ~0ull;

            D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
            Desc.Format = Format;
            Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            Desc.Texture2DArray.MostDetailedMip = 0;
            Desc.Texture2DArray.MipLevels = 1;
            Desc.Texture2DArray.ArraySize = 1;
            Desc.Texture2DArray.FirstArraySlice = SliceId;

            SRVs[SliceId].GpuHandle = pContext->DescHeaps.CBVSRVUAV.GetGPUHandle(BaseSRVHeapIndex + 1 + SliceId);
            D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = pContext->DescHeaps.CBVSRVUAV.GetCPUHandle(BaseSRVHeapIndex + 1 + SliceId);
            pContext->pDevice->CreateShaderResourceView(pResource, &Desc, CpuHandle);
        }
    }
}

//--------------------------------------------------------------------------------
template<UINT ARRAY_SIZE>
void RTTexture2DArray<ARRAY_SIZE>::SafeRelease()
{
    RTTexture2D::SafeRelease();
}

//--------------------------------------------------------------------------------
class RenderTargets
{
public:
    RenderTargets() :
        m_pContext(nullptr)
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
        m_QuarterResAOTextureArray.SafeRelease();
        m_QuarterResViewDepthTextureArray.SafeRelease();
    }

    void Release()
    {
        ReleaseResources();
        SAFE_RELEASE(m_pContext->pDevice);
    }

    void SetGraphicsContext(GFSDK_D3D12_GraphicsContext* pContext)
    {
        m_pContext = pContext;
        m_pContext->pDevice->AddRef();
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

#if _DEBUG
#define SET_TEXTURE_DEBUG_NAME(Name) \
	m_##Name.pResource->SetName(L#Name);
#else
#define SET_TEXTURE_DEBUG_NAME(Name)
#endif

    const RTTexture2D* GetFullResAOZTexture()
    {
        m_FullResAOZTexture.CreateOnce(m_pContext, m_FullWidth, m_FullHeight, DXGI_FORMAT_R16G16_FLOAT, eFullResAOZTexture, eFullResAOZTextureRTV);
        SET_TEXTURE_DEBUG_NAME(FullResAOZTexture);
        return &m_FullResAOZTexture;
    }

    const RTTexture2D* GetFullResAOZTexture2()
    {
        m_FullResAOZTexture2.CreateOnce(m_pContext, m_FullWidth, m_FullHeight, DXGI_FORMAT_R16G16_FLOAT, eFullResAOZTexture2, eFullResAOZTexture2RTV);
        SET_TEXTURE_DEBUG_NAME(FullResAOZTexture2);
        return &m_FullResAOZTexture2;
    }

    const RTTexture2D* GetFullResViewDepthTexture()
    {
        m_FullResViewDepthTexture.CreateOnce(m_pContext, m_FullWidth, m_FullHeight, DXGI_FORMAT_R32_FLOAT, eFullResViewDepthTexture, eFullResViewDepthTextureRTV);
        SET_TEXTURE_DEBUG_NAME(FullResViewDepthTexture);
        return &m_FullResViewDepthTexture;
    }

    DXGI_FORMAT GetViewDepthTextureFormat(GFSDK_SSAO_DepthStorage DepthStorage)
    {
        return (DepthStorage == GFSDK_SSAO_FP16_VIEW_DEPTHS) ? DXGI_FORMAT_R16_FLOAT : DXGI_FORMAT_R32_FLOAT;
    }

    const RTTexture2DArray<16>* GetQuarterResViewDepthTextureArray(const RenderOptions &Options)
    {
        m_QuarterResViewDepthTextureArray.CreateOnce(m_pContext, iDivUp(m_FullWidth, 4), iDivUp(m_FullHeight, 4), GetViewDepthTextureFormat(Options.DepthStorage), eQuarterResViewDepthTextureArray, eQuarterResViewDepthTextureArrayRTV);
        SET_TEXTURE_DEBUG_NAME(QuarterResViewDepthTextureArray);
        return &m_QuarterResViewDepthTextureArray;
    }

    const RTTexture2DArray<16>* GetQuarterResAOTextureArray()
    {
        m_QuarterResAOTextureArray.CreateOnce(m_pContext, iDivUp(m_FullWidth, 4), iDivUp(m_FullHeight, 4), DXGI_FORMAT_R8_UNORM, eQuarterResAOTextureArray, eQuarterResAOTextureArrayRTV);
        SET_TEXTURE_DEBUG_NAME(QuarterResAOTextureArray);
        return &m_QuarterResAOTextureArray;
    }

    const RTTexture2D* GetFullResNormalTexture()
    {
        m_FullResNormalTexture.CreateOnce(m_pContext, m_FullWidth, m_FullHeight, DXGI_FORMAT_R8G8B8A8_UNORM, eFullResNormalTexture, eFullResNormalTextureRTV);
        SET_TEXTURE_DEBUG_NAME(FullResNormalTexture);
        return &m_FullResNormalTexture;
    }

    void CreateOnceAll(const RenderOptions &Options)
    {
        GetFullResViewDepthTexture();
        GetFullResNormalTexture();
        GetQuarterResViewDepthTextureArray(Options);
        GetQuarterResAOTextureArray();

        if (Options.Blur.Enable)
        {
            GetFullResAOZTexture();
            GetFullResAOZTexture2();
        }
    }

    GFSDK_SSAO_Status PreCreate(const RenderOptions &Options)
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
            m_QuarterResAOTextureArray.GetAllocatedSizeInBytes() +
            m_QuarterResViewDepthTextureArray.GetAllocatedSizeInBytes();
    }

private:
    GFSDK_D3D12_GraphicsContext* m_pContext;
    UINT m_FullWidth;
    UINT m_FullHeight;
    RTTexture2D m_FullResAOZTexture;
    RTTexture2D m_FullResAOZTexture2;
    RTTexture2D m_FullResNormalTexture;
    RTTexture2D m_FullResViewDepthTexture;
    RTTexture2DArray<16> m_QuarterResAOTextureArray;
    RTTexture2DArray<16> m_QuarterResViewDepthTextureArray;
};

} // namespace D3D12
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_D3D12
