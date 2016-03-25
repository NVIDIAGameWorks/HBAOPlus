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
#include "Shaders_DX12.h"
#include "RenderTargets_DX12.h"
#include "States_DX12.h"
#include "OutputInfo.h"

namespace GFSDK
{
namespace SSAO
{
namespace D3D12
{

//--------------------------------------------------------------------------------
struct GraphicsPSO
{
    ID3D12PipelineState* pPSO;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;

    GraphicsPSO()
        : pPSO(nullptr)
    {
        Desc = {};
        Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        Desc.InputLayout.NumElements = 0;
        Desc.InputLayout.pInputElementDescs = nullptr;
        Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        Desc.pRootSignature = nullptr;
        Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        Desc.DepthStencilState.DepthEnable = false;
        Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        Desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        Desc.DepthStencilState.StencilEnable = false;
        Desc.SampleMask = UINT_MAX;
        Desc.NumRenderTargets = 1;
        Desc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
        Desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        Desc.SampleDesc.Count = 1;
    }

    void SetGeometryShader(GFSDK_D3D12_GeometryShader* GS)
    {
        Desc.GS.BytecodeLength = GS->BytecodeLength;
        Desc.GS.pShaderBytecode = GS->pShaderBytecode;
    }

    void SetVertexShader(GFSDK_D3D12_VertexShader* VS)
    {
        Desc.VS.BytecodeLength = VS->BytecodeLength;
        Desc.VS.pShaderBytecode = VS->pShaderBytecode;
    }

    void SetPixelShader(GFSDK_D3D12_PixelShader* PS)
    {
        Desc.PS.BytecodeLength = PS->BytecodeLength;
        Desc.PS.pShaderBytecode = PS->pShaderBytecode;
    }
};

//--------------------------------------------------------------------------------
class BasePSO
{
public:
    static D3D12_ROOT_SIGNATURE_FLAGS GetRootSignatureFlags()
    {
        D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
        Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        return Flags;
    }

    static void CreateRootSignature(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        const D3D12_ROOT_SIGNATURE_DESC* pDesc,
        REFIID riid,
        void **ppvRootSignature)
    {
        ID3D10Blob *SignatureBlob = nullptr;
        THROW_IF_FAILED(D3D12SerializeRootSignature(pDesc, D3D_ROOT_SIGNATURE_VERSION_1, &SignatureBlob, nullptr));
        THROW_IF_FAILED(pGraphicsContext->pDevice->CreateRootSignature(pGraphicsContext->NodeMask, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), riid, ppvRootSignature));
        SignatureBlob->Release();
    }

    static void CreateGraphicsPipelineState(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        const D3D12_GRAPHICS_PIPELINE_STATE_DESC *pDesc,
        REFIID riid,
        void **ppPipelineState)
    {
        THROW_IF_FAILED(pGraphicsContext->pDevice->CreateGraphicsPipelineState(pDesc, riid, ppPipelineState));
    }

    static void CopySamplerFrom(D3D12_STATIC_SAMPLER_DESC* Dest, D3D12_SAMPLER_DESC* Src)
    {
        Dest->Filter = Src->Filter;
        Dest->AddressU = Src->AddressU;
        Dest->AddressV = Src->AddressV;
        Dest->AddressW = Src->AddressW;
        Dest->MipLODBias = Src->MipLODBias;
        Dest->MaxAnisotropy = Src->MaxAnisotropy;
        Dest->ComparisonFunc = Src->ComparisonFunc;
        Dest->MinLOD = Src->MinLOD;
        Dest->MaxLOD = Src->MaxLOD;
    }

    D3D12_SAMPLER_DESC& GetAODepthSamplerState(GFSDK::SSAO::D3D12::States &States)
    {
        return States.GetSamplerStatePointClamp();
    }

    UINT GetOutputMSAASampleMask()
    {
        return D3D12_DEFAULT_SAMPLE_MASK;
    }

    D3D12_BLEND_DESC* GetOutputBlendState(OutputInfo& Output, States& States)
    {
        return (Output.Blend.Mode == GFSDK_SSAO_OVERWRITE_RGB) ? States.GetBlendStateDisabledPreserveAlpha() :
               (Output.Blend.Mode == GFSDK_SSAO_MULTIPLY_RGB) ? States.GetBlendStateMultiplyPreserveAlpha() :
                Output.Blend.CustomState.pBlendState;
    }
};

//--------------------------------------------------------------------------------
class LinearDepthPSO: public BasePSO
{
private:
    GraphicsPSO m_LinearDepthPSO;
    ID3D12RootSignature* m_LinearDepthRS;
    int m_ResolveDepthPermutation;
    int m_InputDepthTextureType;

public:
    LinearDepthPSO()
        : m_LinearDepthRS(nullptr)
        , m_ResolveDepthPermutation(-1)
        , m_InputDepthTextureType(-1)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Texture0,
            Count
        };
    };

    void Create(GFSDK_D3D12_GraphicsContext* pGraphicsContext);

    void Release()
    {
        SAFE_RELEASE(m_LinearDepthRS);
        SAFE_RELEASE(m_LinearDepthPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_LinearDepthRS);
        return m_LinearDepthRS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        Shaders &Shaders,
        Generated::ShaderPermutations::RESOLVE_DEPTH ResolveDepthPermutation,
        GFSDK_SSAO_DepthTextureType InputDepthTextureType);
};

//--------------------------------------------------------------------------------
class DebugNormalsPSO : public BasePSO
{
private:
    GraphicsPSO m_DebugNormalsPSO;
    ID3D12RootSignature* m_DebugNormalsRS;
    UINT m_RTSampleCount;
    DXGI_FORMAT m_RTFormat;
    int m_FetchNormalPermutation;

public:
    DebugNormalsPSO()
        : m_DebugNormalsRS(nullptr)
        , m_RTSampleCount(0)
        , m_RTFormat(DXGI_FORMAT_UNKNOWN)
        , m_FetchNormalPermutation(-1)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Texture0,
            Texture1,
            Count
        };
    };

    void Create(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        States &States);

    void Release()
    {
        SAFE_RELEASE(m_DebugNormalsRS);
        SAFE_RELEASE(m_DebugNormalsPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_DebugNormalsRS);
        return m_DebugNormalsRS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        Shaders &Shaders,
        States &States,
        OutputInfo& Output,
        Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL FetchNormalPermutation);
};

//--------------------------------------------------------------------------------
class DeinterleavedDepthPSO : public BasePSO
{
private:
    GraphicsPSO m_DeinterleavedDepthPSO;
    ID3D12RootSignature* m_DeinterleavedDepthRS;
    int m_DepthStorage;

public:
    DeinterleavedDepthPSO()
        : m_DeinterleavedDepthRS(nullptr)
        , m_DepthStorage(-1)
    {
    }

    static const UINT MRT_COUNT = MAX_NUM_MRTS;

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Buffer1,
            Texture0,
            Count
        };
    };

    void Create(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        GFSDK::SSAO::D3D12::States &States);

    void Release()
    {
        SAFE_RELEASE(m_DeinterleavedDepthRS);
        SAFE_RELEASE(m_DeinterleavedDepthPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_DeinterleavedDepthRS);
        return m_DeinterleavedDepthRS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        GFSDK::SSAO::D3D12::Shaders &Shaders,
        GFSDK::SSAO::D3D12::States &States,
        GFSDK::SSAO::D3D12::RenderTargets& RTs,
        GFSDK_SSAO_DepthStorage DepthStorage);
};

//--------------------------------------------------------------------------------
class ReconstructNormalPSO : public BasePSO
{
private:
    GraphicsPSO m_ReconstructNormalPSO;
    ID3D12RootSignature* m_ReconstructNormalRS;

public:
    ReconstructNormalPSO()
        : m_ReconstructNormalRS(nullptr)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Texture0,
            Count
        };
    };

    void Create(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        GFSDK::SSAO::D3D12::States &States);

    void Release()
    {
        SAFE_RELEASE(m_ReconstructNormalRS);
        SAFE_RELEASE(m_ReconstructNormalPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_ReconstructNormalRS);
        return m_ReconstructNormalRS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        Shaders &Shaders);
};

//--------------------------------------------------------------------------------
class CoarseAOPSO : public BasePSO
{
private:
    GraphicsPSO m_CoarseAOPSO;
    ID3D12RootSignature* m_CoarseAORS;
    int m_EnableForegroundAOPermutation;
    int m_EnableBackgroundAOPermutation;
    int m_EnableDepthThresholdPermutation;
    int m_FetchNormalPermutation;

public:
    CoarseAOPSO()
        : m_CoarseAORS(nullptr)
        , m_EnableForegroundAOPermutation(-1)
        , m_EnableBackgroundAOPermutation(-1)
        , m_EnableDepthThresholdPermutation(-1)
        , m_FetchNormalPermutation(-1)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Buffer1,
            Texture0,
            Texture1,
            Count
        };
    };

    void Create(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        GFSDK::SSAO::D3D12::States &States);

    void Release()
    {
        SAFE_RELEASE(m_CoarseAORS);
        SAFE_RELEASE(m_CoarseAOPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_CoarseAORS);
        return m_CoarseAORS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        Shaders &Shaders,
        Generated::ShaderPermutations::ENABLE_FOREGROUND_AO EnableForegroundAOPermutation,
        Generated::ShaderPermutations::ENABLE_BACKGROUND_AO EnableBackgroundAOPermutation,
        Generated::ShaderPermutations::ENABLE_DEPTH_THRESHOLD EnableDepthThresholdPermutation,
        Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL FetchNormalPermutation);
};

//--------------------------------------------------------------------------------
class ReinterleavedAOBlurPSO : public BasePSO
{
private:
    GraphicsPSO m_ReinterleavedAOBlurPSO;
    ID3D12RootSignature* m_ReinterleavedAOBlurRS;

public:
    ReinterleavedAOBlurPSO()
        : m_ReinterleavedAOBlurRS(nullptr)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Texture0,
            Texture1,
            Count
        };
    };

    void Create(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        GFSDK::SSAO::D3D12::States &States);

    void Release()
    {
        SAFE_RELEASE(m_ReinterleavedAOBlurRS);
        SAFE_RELEASE(m_ReinterleavedAOBlurPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_ReinterleavedAOBlurRS);
        return m_ReinterleavedAOBlurRS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        Shaders &Shaders);
};

//--------------------------------------------------------------------------------
class ReinterleavedAOPSO : public BasePSO
{
private:
    GraphicsPSO m_ReinterleavedAOPSO;
    ID3D12RootSignature* m_ReinterleavedAORS;
    UINT m_RTSampleCount;
    DXGI_FORMAT m_RTFormat;
    int m_BlendMode;

public:
    ReinterleavedAOPSO()
        : m_ReinterleavedAORS(nullptr)
        , m_RTSampleCount(0)
        , m_RTFormat(DXGI_FORMAT_UNKNOWN)
        , m_BlendMode(-1)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Texture0,
            Count
        };
    };

    void Create(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        GFSDK::SSAO::D3D12::States &States);

    void Release()
    {
        SAFE_RELEASE(m_ReinterleavedAORS);
        SAFE_RELEASE(m_ReinterleavedAOPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_ReinterleavedAORS);
        return m_ReinterleavedAORS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        Shaders &Shaders,
        States& States,
        OutputInfo& Output);
};

//--------------------------------------------------------------------------------
class BlurXPSO : public BasePSO
{
private:
    GraphicsPSO m_BlurXPSO;
    ID3D12RootSignature* m_BlurXRS;
    int m_EnableSharpnessProfilePermutation;
    int m_BlurKernelRadiusPermutation;

public:
    BlurXPSO()
        : m_BlurXRS(nullptr)
        , m_EnableSharpnessProfilePermutation(-1)
        , m_BlurKernelRadiusPermutation(-1)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Texture0,
            Count
        };
    };

    void Create(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        GFSDK::SSAO::D3D12::States &States);

    void Release()
    {
        SAFE_RELEASE(m_BlurXRS);
        SAFE_RELEASE(m_BlurXPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_BlurXRS);
        return m_BlurXRS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        Shaders &Shaders,
        Generated::ShaderPermutations::ENABLE_SHARPNESS_PROFILE EnableSharpnessProfilePermutation,
        Generated::ShaderPermutations::KERNEL_RADIUS BlurKernelRadiusPermutation);
};

//--------------------------------------------------------------------------------
class BlurYPSO : public BasePSO
{
private:
    GraphicsPSO m_BlurYPSO;
    ID3D12RootSignature* m_BlurYRS;
    int m_EnableSharpnessProfilePermutation;
    int m_BlurKernelRadiusPermutation;
    UINT m_RTSampleCount;
    DXGI_FORMAT m_RTFormat;

public:
    BlurYPSO()
        : m_BlurYRS(nullptr)
        , m_EnableSharpnessProfilePermutation(-1)
        , m_BlurKernelRadiusPermutation(-1)
        , m_RTSampleCount(0)
        , m_RTFormat(DXGI_FORMAT_UNKNOWN)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Texture0,
            Count
        };
    };

    void Create(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        GFSDK::SSAO::D3D12::States &States);

    void Release()
    {
        SAFE_RELEASE(m_BlurYRS);
        SAFE_RELEASE(m_BlurYPSO.pPSO);
    }

    ID3D12RootSignature* GetRS()
    {
        ASSERT(m_BlurYRS);
        return m_BlurYRS;
    }

    ID3D12PipelineState* GetPSO(
        GFSDK_D3D12_GraphicsContext* pGraphicsContext,
        Shaders &Shaders,
        States& States,
        OutputInfo& Output,
        Generated::ShaderPermutations::ENABLE_SHARPNESS_PROFILE EnableSharpnessProfilePermutation,
        Generated::ShaderPermutations::KERNEL_RADIUS BlurKernelRadiusPermutation);
};

} // namespace D3D12
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_D3D12
