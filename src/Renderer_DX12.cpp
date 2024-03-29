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
#include "Renderer_DX12.h"

//--------------------------------------------------------------------------------
struct RenderTargetBarrierScope
{
    RenderTargetBarrierScope(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pResource)
        : m_pCmdList(pCmdList)
        , m_pResource(pResource)
    {
        GFSDK_D3D12_SetResourceBarrier(pCmdList, pResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    ~RenderTargetBarrierScope()
    {
        GFSDK_D3D12_SetResourceBarrier(m_pCmdList, m_pResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
private:
    ID3D12GraphicsCommandList* m_pCmdList;
    ID3D12Resource* m_pResource;
};

#define RT_BARRIER_SCOPE(pCmdList, pResource) RenderTargetBarrierScope RTScope(pCmdList, pResource)

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::CreateResources(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
    m_Shaders.Create(pGraphicsContext->pDevice);

    m_GlobalCB.Create(pGraphicsContext, eGlobalCB, 0);
    m_PerPassCBs.Create(pGraphicsContext, ePerPassCB);

    m_LinearDepthPSO.Create(pGraphicsContext);
    m_DebugNormalsPSO.Create(pGraphicsContext, m_States);
    m_DeinterleavedDepthPSO.Create(pGraphicsContext, m_States);
    m_CoarseAOPSO.Create(pGraphicsContext, m_States);
    m_ReinterleavedAOPSO.Create(pGraphicsContext, m_States);
    m_ReinterleavedAOBlurPSO.Create(pGraphicsContext, m_States);
    m_ReconstructNormalPSO.Create(pGraphicsContext, m_States);
    m_BlurXPSO.Create(pGraphicsContext, m_States);
    m_BlurYPSO.Create(pGraphicsContext, m_States);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::ReleaseResources()
{
    // Test in case Release is called without ever calling RenderAO or PreCreateRTs
    if (m_GraphicsContext.pCmdQueue)
    {
        m_GraphicsContext.WaitGPUIdle();

        SAFE_RELEASE(m_GraphicsContext.pCmdQueue);
    }

    m_GlobalCB.Release();
    m_PerPassCBs.Release();

    m_LinearDepthPSO.Release();
    m_DebugNormalsPSO.Release();
    m_DeinterleavedDepthPSO.Release();
    m_CoarseAOPSO.Release();
    m_ReinterleavedAOPSO.Release();
    m_ReinterleavedAOBlurPSO.Release();
    m_ReconstructNormalPSO.Release();
    m_BlurXPSO.Release();
    m_BlurYPSO.Release();

    m_Shaders.Release();
    m_RTs.Release();
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::ValidateDescriptorHeap(
    const GFSDK_SSAO_DescriptorHeapRange_D3D12& DescHeapRange,
    D3D12_DESCRIPTOR_HEAP_TYPE RequiredHeapType,
    UINT RequiredNumDescriptors)
{
    if (!DescHeapRange.pDescHeap)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = DescHeapRange.pDescHeap->GetDesc();

    if (HeapDesc.Type != RequiredHeapType)
    {
        return GFSDK_SSAO_D3D12_INVALID_HEAP_TYPE;
    }

    if (HeapDesc.NumDescriptors < DescHeapRange.BaseIndex + RequiredNumDescriptors)
    {
        return GFSDK_SSAO_D3D12_INSUFFICIENT_DESCRIPTORS;
    }

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::ValidateDescriptorHeaps(const GFSDK_SSAO_DescriptorHeaps_D3D12& DescriptorHeaps)
{
    GFSDK_SSAO_Status Status;

    Status = ValidateDescriptorHeap(DescriptorHeaps.CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CBVSRVUAVLayoutBase::eCBVSRVUAVLayoutBaseMax);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    Status = ValidateDescriptorHeap(DescriptorHeaps.RTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RTVLayoutBase::eRTVLayoutBaseMax);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::Create(
    ID3D12Device* pDevice, 
    GFSDK_SSAO_UINT NodeMask, 
    const GFSDK_SSAO_DescriptorHeaps_D3D12& DescriptorHeaps,
    GFSDK_SSAO_Version HeaderVersion)
{
    if (!m_BuildVersion.Match(HeaderVersion))
    {
        return GFSDK_SSAO_VERSION_MISMATCH;
    }

    if (!pDevice)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    // Make sure that exactly one bit is set in NodeMask
    if (NodeMask == 0 || (NodeMask & (NodeMask - 1)) != 0)
    {
        return GFSDK_SSAO_D3D12_INVALID_NODE_MASK;
    }

    // Make sure that NodeMask points to an existing node
    if (NodeMask >= (1u << pDevice->GetNodeCount()))
    {
        return GFSDK_SSAO_D3D12_INVALID_NODE_MASK;
    }

    GFSDK_SSAO_Status Status = ValidateDescriptorHeaps(DescriptorHeaps);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    m_GraphicsContext.Init(pDevice, DescriptorHeaps, NodeMask);

    D3D12_FEATURE_DATA_FEATURE_LEVELS FeatureLevelsInfo = {};
    D3D_FEATURE_LEVEL Level = D3D_FEATURE_LEVEL_11_0;
    FeatureLevelsInfo.NumFeatureLevels = 1;
    FeatureLevelsInfo.pFeatureLevelsRequested = &Level;

    HRESULT Hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &FeatureLevelsInfo, sizeof(FeatureLevelsInfo));
    if ((Hr != S_OK) || (FeatureLevelsInfo.MaxSupportedFeatureLevel < D3D_FEATURE_LEVEL_11_0))
    {
        return GFSDK_SSAO_D3D_FEATURE_LEVEL_NOT_SUPPORTED;
    }

#if ENABLE_EXCEPTIONS
    try
    {
        CreateResources(&m_GraphicsContext);
    }
    catch (...)
    {
        ReleaseResources();

        return GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED;
    }
#else
    CreateResources(pContext);
#endif

    m_RTs.SetGraphicsContext(&m_GraphicsContext);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::Release()
{
    ReleaseResources();
    m_GraphicsContext.Release();
    m_NewDelete.delete_(this);
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::RenderAO(
    ID3D12CommandQueue* pCmdQueue,
    ID3D12GraphicsCommandList* pCmdList,
    const GFSDK_SSAO_InputData_D3D12& InputData,
    const GFSDK_SSAO_Parameters& Parameters,
    const GFSDK_SSAO_Output_D3D12& Output,
    GFSDK_SSAO_RenderMask RenderMask)
{
    GFSDK_SSAO_Status Status;

    if (!pCmdList || !pCmdQueue)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    if (pCmdQueue->GetDesc().NodeMask != m_GraphicsContext.NodeMask)
    {
        return GFSDK_SSAO_D3D12_INVALID_NODE_MASK;
    }

    SAFE_RELEASE(m_GraphicsContext.pCmdQueue);

    m_GraphicsContext.pCmdList = pCmdList;
    m_GraphicsContext.pCmdQueue = pCmdQueue;
    m_GraphicsContext.pCmdQueue->AddRef();

    Status = SetDataFlow(InputData, Parameters, Output);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    Status = m_RTs.PreCreate(m_Options);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

#if ENABLE_EXCEPTIONS
    try
    {
        Render(&m_GraphicsContext, RenderMask);
    }
    catch (...)
    {
        ReleaseResources();

        return GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED;
    }
#else
    Render(&m_GraphicsContext, RenderMask);
#endif

    m_GraphicsContext.IncrFenceValue();

    // Make sure that pCmdList is not used outside of RenderAO
    m_GraphicsContext.pCmdList = NULL;

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::PreCreateRTs(ID3D12CommandQueue* pCmdQueue,
    const GFSDK_SSAO_Parameters& Parameters,
    UINT ViewportWidth,
    UINT ViewportHeight)
{
    if (!pCmdQueue)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

    if (pCmdQueue->GetDesc().NodeMask != m_GraphicsContext.NodeMask)
    {
        return GFSDK_SSAO_D3D12_INVALID_NODE_MASK;
    }

    SAFE_RELEASE(m_GraphicsContext.pCmdQueue);

    m_GraphicsContext.pCmdQueue = pCmdQueue;
    m_GraphicsContext.pCmdQueue->AddRef();

    SetAOResolution(ViewportWidth, ViewportHeight);

    m_Options = Parameters;

    return m_RTs.PreCreate(m_Options);
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::GetProjectionMatrixDepthRange(
    const GFSDK_SSAO_InputData_D3D12& InputData,
    GFSDK_SSAO_ProjectionMatrixDepthRange& OutputDepthRange)
{
    GFSDK_SSAO_Status Status;

    SSAO::ProjectionMatrixInfo ProjectionMatrixInfo;
    Status = ProjectionMatrixInfo.Init(InputData.DepthData.ProjectionMatrix, API_D3D11);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    ProjectionMatrixInfo.GetDepthRange(&OutputDepthRange);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawLinearDepthPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawLinearDepthPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    pCmdList->RSSetViewports(1, &m_Viewports.FullRes);

    pCmdList->SetPipelineState(m_LinearDepthPSO.GetPSO(pGraphicsContext, m_Shaders, GetResolveDepthPermutation(), GetDepthLayerCountPermutation(), m_InputDepth.DepthTextureType));

    pCmdList->SetGraphicsRootSignature(m_LinearDepthPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(LinearDepthPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));

    RT_BARRIER_SCOPE(pCmdList, m_RTs.GetFullResViewDepthTexture()->RTV.pResource);

    if (m_Options.EnableDualLayerAO)
    {
        RT_BARRIER_SCOPE(pCmdList, m_RTs.GetFullResViewDepthTexture2()->RTV.pResource);

        D3D12_CPU_DESCRIPTOR_HANDLE RTHandles[] = 
        {
            m_RTs.GetFullResViewDepthTexture()->RTV.CpuHandle,
            m_RTs.GetFullResViewDepthTexture2()->RTV.CpuHandle
        };
        pCmdList->OMSetRenderTargets(SIZEOF_ARRAY(RTHandles), RTHandles, false, nullptr);

        pCmdList->SetGraphicsRootDescriptorTable(LinearDepthPSO::RootParameters::Texture0, m_InputDepth.Texture0.SRV.GpuHandle);
        pCmdList->SetGraphicsRootDescriptorTable(LinearDepthPSO::RootParameters::Texture1, m_InputDepth.Texture1.SRV.GpuHandle);

        pCmdList->DrawInstanced(3, 1, 0, 0);
    }
    else
    {
        pCmdList->OMSetRenderTargets(1, &m_RTs.GetFullResViewDepthTexture()->RTV.CpuHandle, false, nullptr);

        pCmdList->SetGraphicsRootDescriptorTable(LinearDepthPSO::RootParameters::Texture0, m_InputDepth.Texture0.SRV.GpuHandle);

        pCmdList->DrawInstanced(3, 1, 0, 0);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawDeinterleavedDepthPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawDeinterleavedDepthPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    pCmdList->RSSetViewports(1, &m_Viewports.QuarterRes);

    pCmdList->SetGraphicsRootSignature(m_DeinterleavedDepthPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(DeinterleavedDepthPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));
    pCmdList->SetGraphicsRootDescriptorTable(DeinterleavedDepthPSO::RootParameters::Texture0, m_GraphicsContext.DescHeaps.GetGpuHandle(eFullResViewDepthTexture, 0));

    if (m_Options.EnableDualLayerAO)
    {
        pCmdList->SetGraphicsRootDescriptorTable(DeinterleavedDepthPSO::RootParameters::Texture1, m_GraphicsContext.DescHeaps.GetGpuHandle(eFullResViewDepthTexture2, 0));
    }

    pCmdList->SetPipelineState(m_DeinterleavedDepthPSO.GetPSO(pGraphicsContext, m_Shaders, m_States, m_RTs, m_Options.DepthStorage, GetDepthLayerCountPermutation()));

    RT_BARRIER_SCOPE(pCmdList, m_RTs.GetQuarterResViewDepthTextureArray(m_Options)->pResource);

    for (UINT SliceIndex = 0; SliceIndex < 16; SliceIndex += DeinterleavedDepthPSO::MRT_COUNT)
    {
        pCmdList->SetGraphicsRootDescriptorTable(DeinterleavedDepthPSO::RootParameters::Buffer1, m_GraphicsContext.DescHeaps.GetGpuHandle(ePerPassCB, SliceIndex));

        D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = m_RTs.GetQuarterResViewDepthTextureArray(m_Options)->RTVs[SliceIndex].CpuHandle;
        BOOL RTsSingleHandleToDescriptorRange = true;

        pCmdList->OMSetRenderTargets(DeinterleavedDepthPSO::MRT_COUNT, &RTVHandle, RTsSingleHandleToDescriptorRange, nullptr);

        pCmdList->DrawInstanced(3, 1, 0, 0);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawReconstructedNormalPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawReconstructedNormalPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    RT_BARRIER_SCOPE(pCmdList, m_RTs.GetFullResNormalTexture()->RTV.pResource);

    pCmdList->OMSetRenderTargets(1, &m_RTs.GetFullResNormalTexture()->RTV.CpuHandle, false, nullptr);
    pCmdList->RSSetViewports(1, &m_Viewports.FullRes);

    pCmdList->SetGraphicsRootSignature(m_ReconstructNormalPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(ReconstructNormalPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));
    pCmdList->SetGraphicsRootDescriptorTable(ReconstructNormalPSO::RootParameters::Texture0, m_RTs.GetFullResViewDepthTexture()->SRV.GpuHandle);

    pCmdList->SetPipelineState(m_ReconstructNormalPSO.GetPSO(pGraphicsContext, m_Shaders));

    pCmdList->DrawInstanced(3, 1, 0, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawDebugNormalsPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawDebugNormalsPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    pCmdList->OMSetRenderTargets(1, &m_Output.RenderTarget.RTV.CpuHandle, false, nullptr);
    pCmdList->RSSetViewports(1, &m_InputDepth.Viewport);

    pCmdList->SetGraphicsRootSignature(m_DebugNormalsPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(DebugNormalsPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));
    pCmdList->SetGraphicsRootDescriptorTable(DebugNormalsPSO::RootParameters::Texture0, m_RTs.GetFullResViewDepthTexture()->SRV.GpuHandle);

    if (m_InputNormal.Texture.IsSet())
    {
        pCmdList->SetGraphicsRootDescriptorTable(DebugNormalsPSO::RootParameters::Texture1, m_InputNormal.Texture.SRV.GpuHandle);
    }

    pCmdList->SetPipelineState(m_DebugNormalsPSO.GetPSO(pGraphicsContext, m_Shaders, m_States, m_Output, GetFetchNormalPermutation()));

    pCmdList->DrawInstanced(3, 1, 0, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawCoarseAOPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawCoarseAOPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    pCmdList->SetPipelineState(m_CoarseAOPSO.GetPSO(pGraphicsContext, m_Shaders, GetFetchNormalPermutation(), GetDepthLayerCountPermutation(), GetNumStepsPermutation()));

    RT_BARRIER_SCOPE(pCmdList, m_RTs.GetQuarterResAOTextureArray()->pResource);

    pCmdList->OMSetRenderTargets(1, &m_RTs.GetQuarterResAOTextureArray()->RTV.CpuHandle, false, nullptr);
    pCmdList->RSSetViewports(1, &m_Viewports.QuarterRes);

    pCmdList->SetGraphicsRootSignature(m_CoarseAOPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(CoarseAOPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));
    pCmdList->SetGraphicsRootDescriptorTable(CoarseAOPSO::RootParameters::Texture1, GetFullResNormalBufferSRV()->GpuHandle);

    for (UINT SliceIndex = 0; SliceIndex < 16; ++SliceIndex)
    {
        pCmdList->SetGraphicsRootDescriptorTable(CoarseAOPSO::RootParameters::Buffer1, m_GraphicsContext.DescHeaps.GetGpuHandle(ePerPassCB, SliceIndex));
        pCmdList->SetGraphicsRootDescriptorTable(CoarseAOPSO::RootParameters::Texture0, m_RTs.GetQuarterResViewDepthTextureArray(m_Options)->SRVs[SliceIndex].GpuHandle);

        pCmdList->DrawInstanced(3, 1, 0, 0);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawReinterleavedAOPS_PreBlur(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawReinterleavedAOPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    RT_BARRIER_SCOPE(pCmdList, m_RTs.GetFullResAOZTexture2()->RTV.pResource);

    pCmdList->OMSetRenderTargets(1, &m_RTs.GetFullResAOZTexture2()->RTV.CpuHandle, false, nullptr);
    pCmdList->RSSetViewports(1, &m_Viewports.FullRes);

    pCmdList->SetPipelineState(m_ReinterleavedAOBlurPSO.GetPSO(pGraphicsContext, m_Shaders, GetDepthLayerCountPermutation()));

    pCmdList->SetGraphicsRootSignature(m_ReinterleavedAOBlurPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(ReinterleavedAOBlurPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));
    pCmdList->SetGraphicsRootDescriptorTable(ReinterleavedAOBlurPSO::RootParameters::Texture0, m_RTs.GetQuarterResAOTextureArray()->SRV.GpuHandle);
    pCmdList->SetGraphicsRootDescriptorTable(ReinterleavedAOBlurPSO::RootParameters::Texture1, m_RTs.GetFullResViewDepthTexture()->SRV.GpuHandle);

    if (m_Options.EnableDualLayerAO)
    {
        pCmdList->SetGraphicsRootDescriptorTable(ReinterleavedAOBlurPSO::RootParameters::Texture2, m_RTs.GetFullResViewDepthTexture2()->SRV.GpuHandle);
    }

    pCmdList->DrawInstanced(3, 1, 0, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawReinterleavedAOPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawReinterleavedAOPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    pCmdList->OMSetRenderTargets(1, &m_Output.RenderTarget.RTV.CpuHandle, false, nullptr);
    pCmdList->RSSetViewports(1, &m_InputDepth.Viewport);

    if (GetOutputBlendFactor())
    {
        pCmdList->OMSetBlendFactor(GetOutputBlendFactor());
    }

    pCmdList->SetPipelineState(m_ReinterleavedAOPSO.GetPSO(pGraphicsContext, m_Shaders, m_States, m_Output, GetDepthLayerCountPermutation()));

    pCmdList->SetGraphicsRootSignature(m_ReinterleavedAOPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(ReinterleavedAOPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));
    pCmdList->SetGraphicsRootDescriptorTable(ReinterleavedAOPSO::RootParameters::Texture0, m_RTs.GetQuarterResAOTextureArray()->SRV.GpuHandle);

    pCmdList->DrawInstanced(3, 1, 0, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawBlurXPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawBlurXPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    RT_BARRIER_SCOPE(pCmdList, m_RTs.GetFullResAOZTexture()->RTV.pResource);

    pCmdList->OMSetRenderTargets(1, &m_RTs.GetFullResAOZTexture()->RTV.CpuHandle, false, nullptr);
    pCmdList->RSSetViewports(1, &m_Viewports.FullRes);

    pCmdList->SetPipelineState(m_BlurXPSO.GetPSO(pGraphicsContext, m_Shaders, GetEnableSharpnessProfilePermutation(), GetBlurKernelRadiusPermutation()));

    pCmdList->SetGraphicsRootSignature(m_BlurXPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(BlurXPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));
    pCmdList->SetGraphicsRootDescriptorTable(BlurXPSO::RootParameters::Texture0, m_RTs.GetFullResAOZTexture2()->SRV.GpuHandle);

    pCmdList->DrawInstanced(3, 1, 0, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::DrawBlurYPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawBlurYPS");
#endif

    ID3D12GraphicsCommandList* pCmdList = pGraphicsContext->pCmdList;

    pCmdList->RSSetViewports(1, &m_InputDepth.Viewport);

    pCmdList->OMSetRenderTargets(1, &m_Output.RenderTarget.RTV.CpuHandle, false, nullptr);

    if (GetOutputBlendFactor())
    {
        pCmdList->OMSetBlendFactor(GetOutputBlendFactor());
    }

    pCmdList->SetPipelineState(m_BlurYPSO.GetPSO(pGraphicsContext, m_Shaders, m_States, m_Output, GetEnableSharpnessProfilePermutation(), GetBlurKernelRadiusPermutation()));

    pCmdList->SetGraphicsRootSignature(m_BlurYPSO.GetRS());
    pCmdList->SetGraphicsRootDescriptorTable(BlurYPSO::RootParameters::Buffer0, m_GraphicsContext.DescHeaps.GetGpuHandle(eGlobalCB, 0));
    pCmdList->SetGraphicsRootDescriptorTable(BlurYPSO::RootParameters::Texture0, m_RTs.GetFullResAOZTexture()->SRV.GpuHandle);

    pCmdList->DrawInstanced(3, 1, 0, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::RenderHBAOPlus(GFSDK_D3D12_GraphicsContext* pGraphicsContext, GFSDK_SSAO_RenderMask RenderMask)
{
    if (RenderMask & GFSDK_SSAO_DRAW_Z)
    {
        DrawLinearDepthPS(pGraphicsContext);
    }

    if (RenderMask & GFSDK_SSAO_DRAW_DEBUG_N)
    {
        DrawDebugNormalsPS(pGraphicsContext);
    }

    if (RenderMask & GFSDK_SSAO_DRAW_AO)
    {
        DrawDeinterleavedDepthPS(pGraphicsContext);

        if (!m_InputNormal.Texture.IsSet())
        {
            DrawReconstructedNormalPS(pGraphicsContext);
        }

        DrawCoarseAOPS(pGraphicsContext);

        if (m_Options.Blur.Enable)
        {
            DrawReinterleavedAOPS_PreBlur(pGraphicsContext);
            DrawBlurXPS(pGraphicsContext);
            DrawBlurYPS(pGraphicsContext);
        }
        else
        {
            DrawReinterleavedAOPS(pGraphicsContext);
        }
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::Render(GFSDK_D3D12_GraphicsContext* pGraphicsContext, GFSDK_SSAO_RenderMask RenderMask)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"HBAO+");
#endif

    m_GraphicsContext.pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    D3D12_RECT ScissorRect = {};
    ScissorRect.left = 0;
    ScissorRect.top = 0;
    ScissorRect.right = 16384;
    ScissorRect.bottom = 16384;
    m_GraphicsContext.pCmdList->RSSetScissorRects(1, &ScissorRect);

    m_GlobalCB.UpdateBuffer(pGraphicsContext, RenderMask);

    RenderHBAOPlus(pGraphicsContext, RenderMask);
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::SetInputData(const GFSDK_SSAO_InputData_D3D12& InputData)
{
    GFSDK_SSAO_Status Status;

    Status = SetInputDepths(InputData.DepthData);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    Status = SetInputNormals(InputData.NormalData);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::SetInputDepths(const GFSDK_SSAO_InputDepthData_D3D12& DepthData)
{
    m_InputDepth = GFSDK::SSAO::D3D12::InputDepthInfo();

    GFSDK_SSAO_Status Status = m_InputDepth.SetData(DepthData);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    m_GlobalCB.SetDepthData(m_InputDepth);

    SetAOResolution(m_InputDepth.Viewport);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::SetInputNormals(const GFSDK_SSAO_InputNormalData_D3D12& NormalData)
{
    m_InputNormal = GFSDK::SSAO::D3D12::InputNormalInfo();

    if (!NormalData.Enable)
    {
        // Input normals disabled. In this case, the lib reconstructs normals from depths.
        return GFSDK_SSAO_OK;
    }

    GFSDK_SSAO_Status Status = m_InputNormal.SetData(NormalData);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    m_GlobalCB.SetNormalData(NormalData);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::SetOutput(const GFSDK_SSAO_Output_D3D12& Output)
{
    m_Output = GFSDK::SSAO::D3D12::OutputInfo();

    GFSDK_SSAO_Status Status = m_Output.Init(Output);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::ValidateDataFlow()
{
    if (m_InputNormal.Texture.IsSet())
    {
        if (m_InputNormal.Texture.Width  != m_InputDepth.Texture0.Width ||
            m_InputNormal.Texture.Height != m_InputDepth.Texture0.Height)
        {
            return GFSDK_SSAO_INVALID_NORMAL_TEXTURE_RESOLUTION;
        }
        if (m_InputNormal.Texture.SampleCount != m_InputDepth.Texture0.SampleCount)
        {
            return GFSDK_SSAO_INVALID_NORMAL_TEXTURE_SAMPLE_COUNT;
        }
    }
    if (m_Options.EnableDualLayerAO && !m_InputDepth.Texture1.IsSet())
    {
        return GFSDK_SSAO_NO_SECOND_LAYER_PROVIDED;
    }
    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::SetAOParameters(const GFSDK_SSAO_Parameters& Params)
{
    if (Params.DepthClampMode == GFSDK_SSAO_CLAMP_TO_BORDER)
    {
        return GFSDK_SSAO_D3D12_UNSUPPORTED_DEPTH_CLAMP_MODE;
    }

    if (Params.Blur.Enable != m_Options.Blur.Enable ||
        Params.DepthStorage != m_Options.DepthStorage ||
        Params.EnableDualLayerAO != m_Options.EnableDualLayerAO)
    {
        m_GraphicsContext.WaitGPUIdle();
        m_RTs.ReleaseResources();
    }

    m_GlobalCB.SetAOParameters(Params, m_InputDepth);
    m_Options = Params;

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D12::Renderer::SetDataFlow(
    const GFSDK_SSAO_InputData_D3D12& InputData,
    const GFSDK_SSAO_Parameters& Parameters,
    const GFSDK_SSAO_Output_D3D12& Output)
{
    GFSDK_SSAO_Status Status;

    Status = SetInputData(InputData);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    Status = SetAOParameters(Parameters);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    Status = SetOutput(Output);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    Status = ValidateDataFlow();
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::SetAOResolution(UINT Width, UINT Height)
{
    if (Width != m_RTs.GetFullWidth() ||
        Height != m_RTs.GetFullHeight())
    {
        m_GraphicsContext.WaitGPUIdle();
        m_RTs.ReleaseResources();
        m_RTs.SetFullResolution(Width, Height);
        m_Viewports.SetFullResolution(Width, Height);
        m_GlobalCB.SetResolutionConstants(m_Viewports);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D12::Renderer::SetAOResolution(const SSAO::InputViewport &Viewport)
{
    SetAOResolution(UINT(Viewport.Width), UINT(Viewport.Height));
}

//--------------------------------------------------------------------------------
UINT GFSDK::SSAO::D3D12::Renderer::GetAllocatedVideoMemoryBytes()
{
    return m_RTs.GetCurrentAllocatedVideoMemoryBytes();
}

#endif // SUPPORT_D3D12
