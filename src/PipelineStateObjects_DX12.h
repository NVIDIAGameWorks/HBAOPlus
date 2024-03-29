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

    static void SafeCreateGraphicsPipelineState(GFSDK_D3D12_GraphicsContext* pGraphicsContext, GraphicsPSO &PSO)
    {
        //To avoid this debug runtime error:
        //D3D12 ERROR: ID3D12PipelineState::<final-release>: CORRUPTION: An ID3D12PipelineState object (0x00000243251F2850:'Unnamed Object') is referenced by GPU operations in-flight on Command Queue (0x000002431EAF1B20:'Unnamed ID3D12CommandQueue Object').  It is not safe to final-release objects that may have GPU operations pending.  This can result in application instability. 
        pGraphicsContext->WaitGPUIdle();

        SAFE_RELEASE(PSO.pPSO);

        THROW_IF_FAILED(pGraphicsContext->pDevice->CreateGraphicsPipelineState(&PSO.Desc, IID_PPV_ARGS(&PSO.pPSO)));
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
class LinearDepthPSO : public BasePSO
{
private:
    GraphicsPSO m_LinearDepthPSO;
    ID3D12RootSignature* m_LinearDepthRS;
    int m_ResolveDepthPermutation;
    int m_DepthLayerCountPermutation;
    int m_InputDepthTextureType;

public:
    LinearDepthPSO()
        : m_LinearDepthRS(nullptr)
        , m_ResolveDepthPermutation(-1)
        , m_DepthLayerCountPermutation(-1)
        , m_InputDepthTextureType(-1)
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
        Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation,
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
    int m_DepthLayerCountPermutation;

public:
    DeinterleavedDepthPSO()
        : m_DeinterleavedDepthRS(nullptr)
        , m_DepthStorage(-1)
        , m_DepthLayerCountPermutation(-1)
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
            Texture1,
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
        GFSDK_SSAO_DepthStorage DepthStorage,
        Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation);
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
            Texture1,
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
    int m_FetchNormalPermutation;
    int m_DepthLayerCountPermutation;
    int m_NumStepsPermutation;

public:
    CoarseAOPSO()
        : m_CoarseAORS(nullptr)
        , m_FetchNormalPermutation(-1)
        , m_DepthLayerCountPermutation(-1)
        , m_NumStepsPermutation(-1)
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
        Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL FetchNormalPermutation,
        Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerPermutation,
        Generated::ShaderPermutations::NUM_STEPS NumStepsPermutation);
};

//--------------------------------------------------------------------------------
class ReinterleavedAOBlurPSO : public BasePSO
{
private:
    GraphicsPSO m_ReinterleavedAOBlurPSO;
    ID3D12RootSignature* m_ReinterleavedAOBlurRS;
    int m_DepthLayerCountPermutation;

public:
    ReinterleavedAOBlurPSO()
        : m_ReinterleavedAOBlurRS(nullptr)
        , m_DepthLayerCountPermutation(-1)
    {
    }

    struct RootParameters
    {
        enum Indices
        {
            Buffer0,
            Texture0,
            Texture1,
            Texture2,
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
        Shaders &Shaders,
        Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation);
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
    int m_DepthLayerCountPermutation;

public:
    ReinterleavedAOPSO()
        : m_ReinterleavedAORS(nullptr)
        , m_RTSampleCount(0)
        , m_RTFormat(DXGI_FORMAT_UNKNOWN)
        , m_BlendMode(-1)
        , m_DepthLayerCountPermutation(-1)
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
        OutputInfo& Output,
        Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation);
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
    int m_BlendMode;

public:
    BlurYPSO()
        : m_BlurYRS(nullptr)
        , m_EnableSharpnessProfilePermutation(-1)
        , m_BlurKernelRadiusPermutation(-1)
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
