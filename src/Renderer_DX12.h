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
#include "ConstantBuffers.h"
#include "InputDepthInfo.h"
#include "InputNormalInfo.h"
#include "OutputInfo.h"
#include "ProjectionMatrixInfo.h"
#include "RandomTexture.h"
#include "RenderTargets_DX12.h"
#include "Shaders_DX12.h"
#include "PipelineStateObjects_DX12.h"
#include "States_DX12.h"
#include "TimestampQueries.h"
#include "BuildVersion.h"
#include "PerfMarkers.h"

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
#if SUPPORT_D3D12
namespace D3D12
{

//--------------------------------------------------------------------------------
class Renderer : public GFSDK_SSAO_Context_D3D12
{
public:
    Renderer(GFSDK_SSAO_CustomHeap NewDelete)
        : m_NewDelete(NewDelete)
    {
    }

    //
    // API
    //

    GFSDK_SSAO_Status Create(
        ID3D12Device* pDevice, 
        GFSDK_SSAO_UINT NodeMask, 
        const GFSDK_SSAO_DescriptorHeaps_D3D12& DescriptorHeaps,
        GFSDK_SSAO_Version HeaderVersion);

    void Release();

    GFSDK_SSAO_Status PreCreateRTs(
        ID3D12CommandQueue* pCmdQueue,
        const GFSDK_SSAO_Parameters& Parameters,
        UINT ViewportWidth,
        UINT ViewportHeight);

     GFSDK_SSAO_Status GetProjectionMatrixDepthRange(
         const GFSDK_SSAO_InputData_D3D12& InputData,
         GFSDK_SSAO_ProjectionMatrixDepthRange& OutputDepthRange);

    GFSDK_SSAO_Status RenderAO(
        ID3D12CommandQueue* pCmdQueue,
        ID3D12GraphicsCommandList* pCmdList,
        const GFSDK_SSAO_InputData_D3D12& InputData,
        const GFSDK_SSAO_Parameters& Parameters,
        const GFSDK_SSAO_Output_D3D12& Output,
        GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO);

    UINT GetAllocatedVideoMemoryBytes();

    //
    // Internals
    //

private:
    Generated::ShaderPermutations::NUM_STEPS GetNumStepsPermutation()
    {
        return (m_Options.StepCount == GFSDK_SSAO_STEP_COUNT_4) ? Generated::ShaderPermutations::NUM_STEPS_4 :
                                                                  Generated::ShaderPermutations::NUM_STEPS_8;
    }
    Generated::ShaderPermutations::DEPTH_LAYER_COUNT GetDepthLayerCountPermutation()
    {
        return (m_Options.EnableDualLayerAO) ? Generated::ShaderPermutations::DEPTH_LAYER_COUNT_2 :
                                               Generated::ShaderPermutations::DEPTH_LAYER_COUNT_1;
    }
    Generated::ShaderPermutations::RESOLVE_DEPTH GetResolveDepthPermutation()
    {
        return (m_InputDepth.Texture0.SampleCount == 1) ? Generated::ShaderPermutations::RESOLVE_DEPTH_0 :
                                                          Generated::ShaderPermutations::RESOLVE_DEPTH_1;
    }
    Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL GetFetchNormalPermutation()
    {
        return (!m_InputNormal.Texture.IsSet()) ?           Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL_0 :
               (m_InputNormal.Texture.SampleCount == 1) ?   Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL_1 :
                                                            Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL_2;
    }
    Generated::ShaderPermutations::ENABLE_BLUR GetEnableBlurPermutation()
    {
        return (m_Options.Blur.Enable) ? Generated::ShaderPermutations::ENABLE_BLUR_1 :
                                         Generated::ShaderPermutations::ENABLE_BLUR_0;
    }
    Generated::ShaderPermutations::KERNEL_RADIUS GetBlurKernelRadiusPermutation()
    {
        return (m_Options.Blur.Radius == GFSDK_SSAO_BLUR_RADIUS_2) ? Generated::ShaderPermutations::KERNEL_RADIUS_2 :
                                                                     Generated::ShaderPermutations::KERNEL_RADIUS_4;
    }
    Generated::ShaderPermutations::ENABLE_SHARPNESS_PROFILE GetEnableSharpnessProfilePermutation()
    {
        return (m_Options.Blur.SharpnessProfile.Enable) ? Generated::ShaderPermutations::ENABLE_SHARPNESS_PROFILE_1 :
                                                          Generated::ShaderPermutations::ENABLE_SHARPNESS_PROFILE_0;
    }

    const ShaderResourceView* GetFullResNormalBufferSRV()
    {
        return m_InputNormal.Texture.IsSet() ? &m_InputNormal.Texture.SRV : &m_RTs.GetFullResNormalTexture()->SRV;
    }

    const FLOAT* GetOutputBlendFactor()
    {
        return (m_Output.Blend.Mode == GFSDK_SSAO_CUSTOM_BLEND) ? m_Output.Blend.CustomState.pBlendFactor : NULL;
    }

    void CreateResources(GFSDK_D3D12_GraphicsContext* pD3DDevice);
    void ReleaseResources();

    void SetAOResolution(UINT Width, UINT Height);
    void SetAOResolution(const SSAO::InputViewport &Viewport);

    GFSDK_SSAO_Status SetDataFlow(
        const GFSDK_SSAO_InputData_D3D12& InputData,
        const GFSDK_SSAO_Parameters& Parameters,
        const GFSDK_SSAO_Output_D3D12& Output);

    GFSDK_SSAO_Status SetInputData(const GFSDK_SSAO_InputData_D3D12& InputData);
    GFSDK_SSAO_Status SetInputDepths(const GFSDK_SSAO_InputDepthData_D3D12& DepthData);
    GFSDK_SSAO_Status SetInputNormals(const GFSDK_SSAO_InputNormalData_D3D12& NormalData);
    GFSDK_SSAO_Status SetAOParameters(const GFSDK_SSAO_Parameters& Parameters);
    GFSDK_SSAO_Status SetOutput(const GFSDK_SSAO_Output_D3D12& Output);
    GFSDK_SSAO_Status ValidateDataFlow();
    GFSDK_SSAO_Status ValidateDescriptorHeap(const GFSDK_SSAO_DescriptorHeapRange_D3D12& DescHeapRange, D3D12_DESCRIPTOR_HEAP_TYPE RequiredHeapType, UINT RequiredNumDescriptors);
    GFSDK_SSAO_Status ValidateDescriptorHeaps(const GFSDK_SSAO_DescriptorHeaps_D3D12& DescriptorHeaps);

    void DrawLinearDepthPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext);
    void DrawDeinterleavedDepthPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext);
    void DrawReconstructedNormalPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext);
    void DrawCoarseAOPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext);
    void DrawReinterleavedAOPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext);
    void DrawReinterleavedAOPS_PreBlur(GFSDK_D3D12_GraphicsContext* pGraphicsContext);

    void Render(GFSDK_D3D12_GraphicsContext* pGraphicsContext, GFSDK_SSAO_RenderMask RenderMask);
    void RenderHBAOPlus(GFSDK_D3D12_GraphicsContext* pGraphicsContext, GFSDK_SSAO_RenderMask RenderMask);

    void DrawBlurXPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext);
    void DrawBlurYPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext);
    void DrawDebugNormalsPS(GFSDK_D3D12_GraphicsContext* pGraphicsContext);

    GFSDK::SSAO::D3D12::GlobalConstantBuffer m_GlobalCB;
    GFSDK::SSAO::D3D12::PerPassConstantBuffers m_PerPassCBs;
    GFSDK::SSAO::D3D12::InputDepthInfo m_InputDepth;
    GFSDK::SSAO::D3D12::InputNormalInfo m_InputNormal;
    GFSDK::SSAO::D3D12::OutputInfo m_Output;
    GFSDK::SSAO::D3D12::RenderTargets m_RTs;
    GFSDK::SSAO::D3D12::Shaders m_Shaders;
    GFSDK::SSAO::D3D12::States m_States;
    GFSDK::SSAO::D3D12::RandomTexture m_RandomTexture;
    GFSDK_SSAO_Parameters m_Options;
    GFSDK::SSAO::Viewports m_Viewports;
    GFSDK::SSAO::BuildVersion m_BuildVersion;
    GFSDK_SSAO_CustomHeap m_NewDelete;

    LinearDepthPSO m_LinearDepthPSO;
    DebugNormalsPSO m_DebugNormalsPSO;
    DeinterleavedDepthPSO m_DeinterleavedDepthPSO;
    ReconstructNormalPSO m_ReconstructNormalPSO;
    CoarseAOPSO m_CoarseAOPSO;
    ReinterleavedAOBlurPSO m_ReinterleavedAOBlurPSO;
    ReinterleavedAOPSO m_ReinterleavedAOPSO;
    BlurXPSO m_BlurXPSO;
    BlurYPSO m_BlurYPSO;

    GFSDK_D3D12_GraphicsContext m_GraphicsContext;
};

} // namespace D3D12
#endif // SUPPORT_D3D12

} // namespace SSAO
} // namespace GFSDK
