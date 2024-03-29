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

#include "PipelineStateObjects_DX12.h"

//--------------------------------------------------------------------------------
#if ENABLE_DEBUG_NAMES
#define SET_PSO_DEBUG_NAME(Name) \
    m_##Name.pPSO->SetName(L#Name);
#define SET_RS_DEBUG_NAME(Name) \
    m_##Name->SetName(L#Name);
#else
#define SET_PSO_DEBUG_NAME(Name) 
#define SET_RS_DEBUG_NAME(Name)
#endif

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::LinearDepthPSO::Create(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
    ASSERT(!m_LinearDepthRS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    DescRanges[RootParameters::Texture1] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], D3D12_SHADER_VISIBILITY_PIXEL);
    }

    D3D12_ROOT_SIGNATURE_DESC Desc = {};
    Desc.NumParameters = SIZEOF_ARRAY(RootParams);
    Desc.NumStaticSamplers = 0;
    Desc.pParameters = RootParams;
    Desc.pStaticSamplers = nullptr;
    Desc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &Desc, IID_PPV_ARGS(&m_LinearDepthRS));
    SET_RS_DEBUG_NAME(LinearDepthRS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::LinearDepthPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    Shaders &Shaders,
    Generated::ShaderPermutations::RESOLVE_DEPTH ResolveDepthPermutation,
    Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation,
    GFSDK_SSAO_DepthTextureType InputDepthTextureType)
{
    if (!m_LinearDepthPSO.pPSO ||
        m_ResolveDepthPermutation != ResolveDepthPermutation ||
        m_DepthLayerCountPermutation != DepthLayerCountPermutation ||
        m_InputDepthTextureType != InputDepthTextureType)
    {
        m_ResolveDepthPermutation = ResolveDepthPermutation;
        m_DepthLayerCountPermutation = DepthLayerCountPermutation;
        m_InputDepthTextureType = InputDepthTextureType;

        GFSDK_D3D12_PixelShader* pPS = (m_InputDepthTextureType == GFSDK_SSAO_VIEW_DEPTHS) ?
            Shaders.CopyDepth_PS.Get(ResolveDepthPermutation, DepthLayerCountPermutation) :
            Shaders.LinearizeDepth_PS.Get(ResolveDepthPermutation, DepthLayerCountPermutation);

        GFSDK_D3D12_VertexShader* pVS = Shaders.FullScreenTriangle_VS.Get();

        if (DepthLayerCountPermutation == Generated::ShaderPermutations::DEPTH_LAYER_COUNT_2)
        {
            m_LinearDepthPSO.Desc.NumRenderTargets = 2;
            m_LinearDepthPSO.Desc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
            m_LinearDepthPSO.Desc.RTVFormats[1] = DXGI_FORMAT_R32_FLOAT;
        }
        else
        {
            m_LinearDepthPSO.Desc.NumRenderTargets = 1;
            m_LinearDepthPSO.Desc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
            m_LinearDepthPSO.Desc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
        }

        m_LinearDepthPSO.Desc.pRootSignature = m_LinearDepthRS;
        m_LinearDepthPSO.Desc.SampleDesc.Count = 1;
        m_LinearDepthPSO.Desc.NodeMask = pGraphicsContext->NodeMask;
        m_LinearDepthPSO.SetVertexShader(pVS);
        m_LinearDepthPSO.SetPixelShader(pPS);

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_LinearDepthPSO);
        SET_PSO_DEBUG_NAME(LinearDepthPSO);
    }

    return m_LinearDepthPSO.pPSO;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::DebugNormalsPSO::Create(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    States &States)
{
    ASSERT(!m_DebugNormalsRS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    DescRanges[RootParameters::Texture1] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], D3D12_SHADER_VISIBILITY_PIXEL);
    }

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
    StaticSamplers[0].Init(0);
    CopySamplerFrom(&StaticSamplers[0], &GetAODepthSamplerState(States));

    D3D12_ROOT_SIGNATURE_DESC Desc = {};
    Desc.NumParameters = SIZEOF_ARRAY(RootParams);
    Desc.NumStaticSamplers = SIZEOF_ARRAY(StaticSamplers);
    Desc.pParameters = RootParams;
    Desc.pStaticSamplers = StaticSamplers;
    Desc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &Desc, IID_PPV_ARGS(&m_DebugNormalsRS));
    SET_RS_DEBUG_NAME(DebugNormalsRS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::DebugNormalsPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    Shaders &Shaders,
    States &States,
    OutputInfo& Output,
    Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL FetchNormalPermutation)
{
    if (!m_DebugNormalsPSO.pPSO ||
        m_RTSampleCount != Output.RenderTarget.SampleCount ||
        m_RTFormat != Output.RenderTarget.Format ||
        m_FetchNormalPermutation != FetchNormalPermutation)
    {
        m_RTSampleCount = Output.RenderTarget.SampleCount;
        m_RTFormat = Output.RenderTarget.Format;
        m_FetchNormalPermutation = FetchNormalPermutation;

        GFSDK_D3D12_PixelShader* pPS = Shaders.DebugNormals_PS.Get(FetchNormalPermutation);
        GFSDK_D3D12_PixelShader* pVS = Shaders.FullScreenTriangle_VS.Get();

        m_DebugNormalsPSO.Desc.pRootSignature = m_DebugNormalsRS;
        m_DebugNormalsPSO.Desc.RTVFormats[0] = Output.RenderTarget.Format;
        m_DebugNormalsPSO.Desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        m_DebugNormalsPSO.Desc.BlendState = *States.GetBlendStateDisabled();
        m_DebugNormalsPSO.Desc.SampleDesc.Count = Output.RenderTarget.SampleCount;
        m_DebugNormalsPSO.Desc.SampleMask = GetOutputMSAASampleMask();
        m_DebugNormalsPSO.Desc.NodeMask = pGraphicsContext->NodeMask;

        m_DebugNormalsPSO.SetVertexShader(pVS);
        m_DebugNormalsPSO.SetPixelShader(pPS);

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_DebugNormalsPSO);
        SET_PSO_DEBUG_NAME(DebugNormalsPSO);
    }

    return m_DebugNormalsPSO.pPSO;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::DeinterleavedDepthPSO::Create(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    GFSDK::SSAO::D3D12::States &States)
{
    ASSERT(!m_DeinterleavedDepthRS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Buffer1] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    DescRanges[RootParameters::Texture1] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], D3D12_SHADER_VISIBILITY_PIXEL);
    }

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
    StaticSamplers[0].Init(0);
    CopySamplerFrom(&StaticSamplers[0], &States.GetSamplerStatePointClamp());

    D3D12_ROOT_SIGNATURE_DESC Desc = {};
    Desc.NumParameters = SIZEOF_ARRAY(RootParams);
    Desc.NumStaticSamplers = SIZEOF_ARRAY(StaticSamplers);
    Desc.pParameters = RootParams;
    Desc.pStaticSamplers = StaticSamplers;
    Desc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &Desc, IID_PPV_ARGS(&m_DeinterleavedDepthRS));
    SET_RS_DEBUG_NAME(DeinterleavedDepthRS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::DeinterleavedDepthPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    GFSDK::SSAO::D3D12::Shaders &Shaders,
    GFSDK::SSAO::D3D12::States &States,
    GFSDK::SSAO::D3D12::RenderTargets& RTs,
    GFSDK_SSAO_DepthStorage DepthStorage,
    Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation)
{
    if (!m_DeinterleavedDepthPSO.pPSO ||
        m_DepthStorage != DepthStorage ||
        m_DepthLayerCountPermutation != DepthLayerCountPermutation)
    {
        m_DepthStorage = DepthStorage;
        m_DepthLayerCountPermutation = DepthLayerCountPermutation;

        m_DeinterleavedDepthPSO.Desc.pRootSignature = m_DeinterleavedDepthRS;
        m_DeinterleavedDepthPSO.Desc.NumRenderTargets = MRT_COUNT;
        DXGI_FORMAT Format = (DepthLayerCountPermutation == Generated::ShaderPermutations::DEPTH_LAYER_COUNT_2) ? DXGI_FORMAT_R16G16_FLOAT : RTs.GetViewDepthTextureFormat(DepthStorage);
        for (UINT Idx = 0; Idx < MRT_COUNT; ++Idx)
        {
            m_DeinterleavedDepthPSO.Desc.RTVFormats[Idx] = Format;
        }
        m_DeinterleavedDepthPSO.Desc.SampleDesc.Count = 1;
        m_DeinterleavedDepthPSO.Desc.NodeMask = pGraphicsContext->NodeMask;
        m_DeinterleavedDepthPSO.SetVertexShader(Shaders.FullScreenTriangle_VS.Get());
        m_DeinterleavedDepthPSO.SetPixelShader(Shaders.DeinterleaveDepth_PS.Get(DepthLayerCountPermutation));

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_DeinterleavedDepthPSO);
        SET_PSO_DEBUG_NAME(DeinterleavedDepthPSO);
    }

    return m_DeinterleavedDepthPSO.pPSO;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::ReconstructNormalPSO::Create(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    GFSDK::SSAO::D3D12::States &States)
{
    ASSERT(!m_ReconstructNormalRS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    DescRanges[RootParameters::Texture1] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], D3D12_SHADER_VISIBILITY_PIXEL);
    }

    RootParams[RootParameters::Buffer0].InitAsDescriptorTable(1, &DescRanges[RootParameters::Buffer0]);
    RootParams[RootParameters::Texture0].InitAsDescriptorTable(1, &DescRanges[RootParameters::Texture0], D3D12_SHADER_VISIBILITY_PIXEL);
    RootParams[RootParameters::Texture1].InitAsDescriptorTable(1, &DescRanges[RootParameters::Texture1], D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
    StaticSamplers[0].Init(0);
    CopySamplerFrom(&StaticSamplers[0], &GetAODepthSamplerState(States));

    D3D12_ROOT_SIGNATURE_DESC Desc = {};
    Desc.NumParameters = SIZEOF_ARRAY(RootParams);
    Desc.NumStaticSamplers = SIZEOF_ARRAY(StaticSamplers);
    Desc.pParameters = RootParams;
    Desc.pStaticSamplers = StaticSamplers;
    Desc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &Desc, IID_PPV_ARGS(&m_ReconstructNormalRS));
    SET_RS_DEBUG_NAME(ReconstructNormalRS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::ReconstructNormalPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    Shaders &Shaders)
{
    if (!m_ReconstructNormalPSO.pPSO)
    {
        m_ReconstructNormalPSO.Desc.pRootSignature = m_ReconstructNormalRS;
        m_ReconstructNormalPSO.Desc.NumRenderTargets = 1;
        m_ReconstructNormalPSO.Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_ReconstructNormalPSO.Desc.SampleDesc.Count = 1;
        m_ReconstructNormalPSO.Desc.NodeMask = pGraphicsContext->NodeMask;

        m_ReconstructNormalPSO.SetVertexShader(Shaders.FullScreenTriangle_VS.Get());
        m_ReconstructNormalPSO.SetPixelShader(Shaders.ReconstructNormal_PS.Get());

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_ReconstructNormalPSO);
        SET_PSO_DEBUG_NAME(ReconstructNormalPSO);
    }

    return m_ReconstructNormalPSO.pPSO;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::CoarseAOPSO::Create(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    GFSDK::SSAO::D3D12::States &States)
{
    ASSERT(!m_CoarseAORS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Buffer1] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    DescRanges[RootParameters::Texture1] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        D3D12_SHADER_VISIBILITY Visibility = (ParamIndex == RootParameters::Buffer1) ? D3D12_SHADER_VISIBILITY_ALL : D3D12_SHADER_VISIBILITY_PIXEL;
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], Visibility);
    }

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
    StaticSamplers[0].Init(0);
    CopySamplerFrom(&StaticSamplers[0], &GetAODepthSamplerState(States));

    D3D12_ROOT_SIGNATURE_DESC Desc = {};
    Desc.NumParameters = SIZEOF_ARRAY(RootParams);
    Desc.NumStaticSamplers = SIZEOF_ARRAY(StaticSamplers);
    Desc.pParameters = RootParams;
    Desc.pStaticSamplers = StaticSamplers;
    Desc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &Desc, IID_PPV_ARGS(&m_CoarseAORS));
    SET_RS_DEBUG_NAME(CoarseAORS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::CoarseAOPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    Shaders &Shaders,
    Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL FetchNormalPermutation,
    Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation,
    Generated::ShaderPermutations::NUM_STEPS NumStepsPermutation)
{
    if (!m_CoarseAOPSO.pPSO ||
        m_FetchNormalPermutation != FetchNormalPermutation ||
        m_DepthLayerCountPermutation != DepthLayerCountPermutation ||
        m_NumStepsPermutation != NumStepsPermutation)
    {
        m_FetchNormalPermutation = FetchNormalPermutation;
        m_DepthLayerCountPermutation = DepthLayerCountPermutation;
        m_NumStepsPermutation = NumStepsPermutation;

        m_CoarseAOPSO.Desc.pRootSignature = m_CoarseAORS;
        m_CoarseAOPSO.Desc.NumRenderTargets = 1;
        m_CoarseAOPSO.Desc.RTVFormats[0] = DXGI_FORMAT_R8_UNORM;
        m_CoarseAOPSO.Desc.SampleDesc.Count = 1;
        m_CoarseAOPSO.Desc.NodeMask = pGraphicsContext->NodeMask;
        m_CoarseAOPSO.SetVertexShader(Shaders.FullScreenTriangle_VS.Get());
        m_CoarseAOPSO.SetPixelShader(Shaders.CoarseAO_PS.Get(FetchNormalPermutation, DepthLayerCountPermutation, NumStepsPermutation));
        m_CoarseAOPSO.SetGeometryShader(Shaders.CoarseAO_GS.Get());

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_CoarseAOPSO);
        SET_PSO_DEBUG_NAME(CoarseAOPSO);
    }

    return m_CoarseAOPSO.pPSO;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::ReinterleavedAOBlurPSO::Create(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    GFSDK::SSAO::D3D12::States &States)
{
    ASSERT(!m_ReinterleavedAOBlurRS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    DescRanges[RootParameters::Texture1] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    DescRanges[RootParameters::Texture2] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], D3D12_SHADER_VISIBILITY_PIXEL);
    }

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
    StaticSamplers[0].Init(0);
    CopySamplerFrom(&StaticSamplers[0], &States.GetSamplerStatePointClamp());

    D3D12_ROOT_SIGNATURE_DESC Desc = {};
    Desc.NumParameters = SIZEOF_ARRAY(RootParams);
    Desc.NumStaticSamplers = SIZEOF_ARRAY(StaticSamplers);
    Desc.pParameters = RootParams;
    Desc.pStaticSamplers = StaticSamplers;
    Desc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &Desc, IID_PPV_ARGS(&m_ReinterleavedAOBlurRS));
    SET_RS_DEBUG_NAME(ReinterleavedAOBlurRS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::ReinterleavedAOBlurPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    Shaders &Shaders,
    Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation)
{
    if (!m_ReinterleavedAOBlurPSO.pPSO || 
        m_DepthLayerCountPermutation != DepthLayerCountPermutation)
    {
        m_DepthLayerCountPermutation = DepthLayerCountPermutation;

        m_ReinterleavedAOBlurPSO.Desc.pRootSignature = m_ReinterleavedAOBlurRS;
        m_ReinterleavedAOBlurPSO.Desc.NumRenderTargets = 1;
        m_ReinterleavedAOBlurPSO.Desc.SampleDesc.Count = 1;
        m_ReinterleavedAOBlurPSO.Desc.RTVFormats[0] = DXGI_FORMAT_R16G16_FLOAT;
        m_ReinterleavedAOBlurPSO.Desc.NodeMask = pGraphicsContext->NodeMask;

        m_ReinterleavedAOBlurPSO.SetVertexShader(Shaders.FullScreenTriangle_VS.Get());
        m_ReinterleavedAOBlurPSO.SetPixelShader(Shaders.ReinterleaveAO_PS.Get(Generated::ShaderPermutations::ENABLE_BLUR_1, DepthLayerCountPermutation));

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_ReinterleavedAOBlurPSO);
        SET_PSO_DEBUG_NAME(ReinterleavedAOBlurPSO);
    }

    return m_ReinterleavedAOBlurPSO.pPSO;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::ReinterleavedAOPSO::Create(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    GFSDK::SSAO::D3D12::States &States)
{
    ASSERT(!m_ReinterleavedAORS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], D3D12_SHADER_VISIBILITY_PIXEL);
    }

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[1];
    StaticSamplers[0].Init(0);
    CopySamplerFrom(&StaticSamplers[0], &States.GetSamplerStatePointClamp());

    D3D12_ROOT_SIGNATURE_DESC Desc = {};
    Desc.NumParameters = SIZEOF_ARRAY(RootParams);
    Desc.NumStaticSamplers = SIZEOF_ARRAY(StaticSamplers);
    Desc.pParameters = RootParams;
    Desc.pStaticSamplers = StaticSamplers;
    Desc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &Desc, IID_PPV_ARGS(&m_ReinterleavedAORS));
    SET_RS_DEBUG_NAME(ReinterleavedAORS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::ReinterleavedAOPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    Shaders &Shaders,
    States& States,
    OutputInfo& Output,
    Generated::ShaderPermutations::DEPTH_LAYER_COUNT DepthLayerCountPermutation)
{
    if (!m_ReinterleavedAOPSO.pPSO ||
        m_RTSampleCount != Output.RenderTarget.SampleCount ||
        m_RTFormat != Output.RenderTarget.Format ||
        m_BlendMode != Output.Blend.Mode ||
        m_DepthLayerCountPermutation != DepthLayerCountPermutation)
    {
        m_RTSampleCount = Output.RenderTarget.SampleCount;
        m_RTFormat = Output.RenderTarget.Format;
        m_BlendMode = Output.Blend.Mode;
        m_DepthLayerCountPermutation = DepthLayerCountPermutation;

        m_ReinterleavedAOPSO.Desc.pRootSignature = m_ReinterleavedAORS;
        m_ReinterleavedAOPSO.Desc.NumRenderTargets = 1;
        m_ReinterleavedAOPSO.Desc.SampleDesc.Count = Output.RenderTarget.SampleCount;
        m_ReinterleavedAOPSO.Desc.BlendState = *GetOutputBlendState(Output, States);
        m_ReinterleavedAOPSO.Desc.SampleMask = GetOutputMSAASampleMask();
        m_ReinterleavedAOPSO.Desc.RTVFormats[0] = Output.RenderTarget.Format;
        m_ReinterleavedAOPSO.Desc.NodeMask = pGraphicsContext->NodeMask;

        m_ReinterleavedAOPSO.SetVertexShader(Shaders.FullScreenTriangle_VS.Get());
        m_ReinterleavedAOPSO.SetPixelShader(Shaders.ReinterleaveAO_PS.Get(Generated::ShaderPermutations::ENABLE_BLUR_0, DepthLayerCountPermutation));

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_ReinterleavedAOPSO);
        SET_PSO_DEBUG_NAME(ReinterleavedAOPSO);
    }

    return m_ReinterleavedAOPSO.pPSO;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::BlurXPSO::Create(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    GFSDK::SSAO::D3D12::States &States)
{
    ASSERT(!m_BlurXRS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], D3D12_SHADER_VISIBILITY_PIXEL);
    }

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[2];
    StaticSamplers[0].Init(0);
    StaticSamplers[1].Init(1);

    CopySamplerFrom(&StaticSamplers[0], &States.GetSamplerStatePointClamp());
    CopySamplerFrom(&StaticSamplers[1], &States.GetSamplerStateLinearClamp());

    D3D12_ROOT_SIGNATURE_DESC Desc = {};
    Desc.NumParameters = SIZEOF_ARRAY(RootParams);
    Desc.NumStaticSamplers = SIZEOF_ARRAY(StaticSamplers);
    Desc.pParameters = RootParams;
    Desc.pStaticSamplers = StaticSamplers;
    Desc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &Desc, IID_PPV_ARGS(&m_BlurXRS));
    SET_RS_DEBUG_NAME(BlurXRS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::BlurXPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    Shaders &Shaders,
    Generated::ShaderPermutations::ENABLE_SHARPNESS_PROFILE EnableSharpnessProfilePermutation,
    Generated::ShaderPermutations::KERNEL_RADIUS BlurKernelRadiusPermutation)
{
    if (!m_BlurXPSO.pPSO ||
        m_EnableSharpnessProfilePermutation != EnableSharpnessProfilePermutation ||
        m_BlurKernelRadiusPermutation != BlurKernelRadiusPermutation)
    {
        m_EnableSharpnessProfilePermutation = EnableSharpnessProfilePermutation;
        m_BlurKernelRadiusPermutation = BlurKernelRadiusPermutation;

        m_BlurXPSO.Desc.pRootSignature = m_BlurXRS;
        m_BlurXPSO.Desc.NumRenderTargets = 1;
        m_BlurXPSO.Desc.RTVFormats[0] = DXGI_FORMAT_R16G16_FLOAT;
        m_BlurXPSO.Desc.SampleDesc.Count = 1;
        m_BlurXPSO.Desc.NodeMask = pGraphicsContext->NodeMask;

        m_BlurXPSO.SetVertexShader(Shaders.FullScreenTriangle_VS.Get());
        m_BlurXPSO.SetPixelShader(Shaders.BlurX_PS.Get(EnableSharpnessProfilePermutation, BlurKernelRadiusPermutation));

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_BlurXPSO);
        SET_PSO_DEBUG_NAME(BlurXPSO);
    }

    return m_BlurXPSO.pPSO;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::BlurYPSO::Create(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    GFSDK::SSAO::D3D12::States &States)
{
    ASSERT(!m_BlurYRS);

    CD3DX12_DESCRIPTOR_RANGE DescRanges[RootParameters::Count];
    DescRanges[RootParameters::Buffer0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    DescRanges[RootParameters::Texture0] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER RootParams[RootParameters::Count];
    for (UINT ParamIndex = 0; ParamIndex < SIZEOF_ARRAY(RootParams); ++ParamIndex)
    {
        RootParams[ParamIndex].InitAsDescriptorTable(1, &DescRanges[ParamIndex], D3D12_SHADER_VISIBILITY_PIXEL);
    }

    CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[2];
    StaticSamplers[0].Init(0);
    StaticSamplers[1].Init(1);

    CopySamplerFrom(&StaticSamplers[0], &States.GetSamplerStatePointClamp());
    CopySamplerFrom(&StaticSamplers[1], &States.GetSamplerStateLinearClamp());

    D3D12_ROOT_SIGNATURE_DESC RootSigDesc = {};
    RootSigDesc.NumParameters = SIZEOF_ARRAY(RootParams);
    RootSigDesc.NumStaticSamplers = SIZEOF_ARRAY(StaticSamplers);
    RootSigDesc.pParameters = RootParams;
    RootSigDesc.pStaticSamplers = StaticSamplers;
    RootSigDesc.Flags = GetRootSignatureFlags();

    CreateRootSignature(pGraphicsContext, &RootSigDesc, IID_PPV_ARGS(&m_BlurYRS));
    SET_RS_DEBUG_NAME(BlurYRS);
}

//--------------------------------------------------------------------------------
ID3D12PipelineState* GFSDK::SSAO::D3D12::BlurYPSO::GetPSO(
    GFSDK_D3D12_GraphicsContext* pGraphicsContext,
    Shaders &Shaders,
    States& States,
    OutputInfo& Output,
    Generated::ShaderPermutations::ENABLE_SHARPNESS_PROFILE EnableSharpnessProfilePermutation,
    Generated::ShaderPermutations::KERNEL_RADIUS BlurKernelRadiusPermutation)
{
    if (!m_BlurYPSO.pPSO ||
        m_EnableSharpnessProfilePermutation != EnableSharpnessProfilePermutation ||
        m_BlurKernelRadiusPermutation != BlurKernelRadiusPermutation ||
        m_RTSampleCount != Output.RenderTarget.SampleCount ||
        m_RTFormat != Output.RenderTarget.Format ||
        m_BlendMode != Output.Blend.Mode)
    {
        m_EnableSharpnessProfilePermutation = EnableSharpnessProfilePermutation;
        m_BlurKernelRadiusPermutation = BlurKernelRadiusPermutation;
        m_RTSampleCount = Output.RenderTarget.SampleCount;
        m_RTFormat = Output.RenderTarget.Format;
        m_BlendMode = Output.Blend.Mode;

        m_BlurYPSO.Desc.pRootSignature = m_BlurYRS;
        m_BlurYPSO.Desc.NumRenderTargets = 1;
        m_BlurYPSO.Desc.RTVFormats[0] = Output.RenderTarget.Format;
        m_BlurYPSO.Desc.SampleDesc.Count = Output.RenderTarget.SampleCount;
        m_BlurYPSO.Desc.BlendState = *GetOutputBlendState(Output, States);
        m_BlurYPSO.Desc.SampleMask = GetOutputMSAASampleMask();
        m_BlurYPSO.Desc.NodeMask = pGraphicsContext->NodeMask;

        m_BlurYPSO.SetVertexShader(Shaders.FullScreenTriangle_VS.Get());
        m_BlurYPSO.SetPixelShader(Shaders.BlurY_PS.Get(EnableSharpnessProfilePermutation, BlurKernelRadiusPermutation));

        SafeCreateGraphicsPipelineState(pGraphicsContext, m_BlurYPSO);
        SET_PSO_DEBUG_NAME(BlurYPSO);
    }

    return m_BlurYPSO.pPSO;
}

#endif
