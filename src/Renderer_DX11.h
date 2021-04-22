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
#include "RenderTargets_DX11.h"
#include "Shaders_DX11.h"
#include "States_DX11.h"
#include "AppState_DX11.h"
#include "TimestampQueries.h"
#include "BuildVersion.h"
#include "PerfMarkers.h"

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
#if SUPPORT_D3D11

namespace D3D11
{

class Renderer : public GFSDK_SSAO_Context_D3D11
{
public:
    Renderer(GFSDK_SSAO_CustomHeap NewDelete)
        : m_NewDelete(NewDelete)
    {
    }

    //
    // API
    //

    GFSDK_SSAO_Status Create(ID3D11Device* pD3DDevice, GFSDK_SSAO_Version HeaderVersion);

    void Release();

    GFSDK_SSAO_Status PreCreateRTs(
        const GFSDK_SSAO_Parameters& Parameters,
        UINT ViewportWidth,
        UINT ViewportHeight);

    GFSDK_SSAO_Status GetProjectionMatrixDepthRange(
        const GFSDK_SSAO_InputData_D3D11& InputData,
        GFSDK_SSAO_ProjectionMatrixDepthRange& OutputDepthRange);

    GFSDK_SSAO_Status RenderAO(
        ID3D11DeviceContext* pDeviceContext,
        const GFSDK_SSAO_InputData_D3D11& InputData,
        const GFSDK_SSAO_Parameters& Parameters,
        const GFSDK_SSAO_Output_D3D11& Output,
        GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO);

    UINT GetAllocatedVideoMemoryBytes();

    //
    // Internals
    //

#if ENABLE_RENDER_TIMES
    static GFSDK::SSAO::RenderTimes s_RenderTimes;
#endif

#if ENABLE_DEBUG_MODES
    enum ShaderType
    {
        NVSDK_HBAO_PLUS_PS,
        NVSDK_DEBUG_HBAO_PLUS_PS,
    };
    static ShaderType s_AOShaderType;
#endif

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
               (m_InputNormal.Texture.SampleCount == 1)  ?  Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL_1 :
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

    ID3D11SamplerState*& GetAODepthSamplerState()
    {
        return (m_Options.DepthClampMode == GFSDK_SSAO_CLAMP_TO_EDGE) ? m_States.GetSamplerStatePointClamp() :
                                                                        m_States.GetSamplerStatePointBorder();
    }
    ID3D11ShaderResourceView* GetFullResNormalBufferSRV()
    {
        return (m_InputNormal.Texture.IsSet()) ? m_InputNormal.Texture.pSRV : m_RTs.GetFullResNormalTexture()->pSRV;
    }
    ID3D11ShaderResourceView* GetFullResViewDepthTexture2SRV()
    {
        return m_Options.EnableDualLayerAO ? m_RTs.GetFullResViewDepthTexture2()->pSRV : NULL;
    }

    UINT GetOutputMSAASampleMask()
    {
        return D3D11_DEFAULT_SAMPLE_MASK;
    }
    ID3D11BlendState* GetOutputBlendState()
    {
        return
            (m_Output.Blend.Mode == GFSDK_SSAO_OVERWRITE_RGB) ? m_States.GetBlendStateDisabledPreserveAlpha() :
            (m_Output.Blend.Mode == GFSDK_SSAO_MULTIPLY_RGB)  ? m_States.GetBlendStateMultiplyPreserveAlpha() :
             m_Output.Blend.CustomState.pBlendState;
    }
    const FLOAT* GetOutputBlendFactor()
    {
        return (m_Output.Blend.Mode == GFSDK_SSAO_CUSTOM_BLEND) ? m_Output.Blend.CustomState.pBlendFactor : NULL;
    }

    ID3D11DepthStencilView* GetOutputDepthStencilView()
    {
        return m_Output.DepthStencilBuffer.pDSV;
    }
    ID3D11DepthStencilState* GetOutputDepthStencilState()
    {
        return (m_Output.DepthStencil.Mode == GFSDK_SSAO_CUSTOM_DEPTH_STENCIL) ? m_Output.DepthStencil.CustomState.pDepthStencilState : m_States.GetDepthStencilStateDisabled();
    }
    UINT GetOutputDepthStencilRef()
    {
        return (m_Output.DepthStencil.Mode == GFSDK_SSAO_CUSTOM_DEPTH_STENCIL) ? m_Output.DepthStencil.CustomState.StencilRef : 0;
    }

    void CreateResources(ID3D11Device* pD3DDevice);
    void ReleaseResources();

    void SetFullscreenState(ID3D11DeviceContext* pDeviceContext);
    void SetAOResolution(UINT Width, UINT Height);
    void SetAOResolution(const SSAO::InputViewport &Viewport);

    GFSDK_SSAO_Status SetDataFlow(
        const GFSDK_SSAO_InputData_D3D11& InputData,
        const GFSDK_SSAO_Parameters& Parameters,
        const GFSDK_SSAO_Output_D3D11& Output);

    GFSDK_SSAO_Status SetInputData(const GFSDK_SSAO_InputData_D3D11& InputData);
    GFSDK_SSAO_Status SetInputDepths(const GFSDK_SSAO_InputDepthData_D3D11& DepthData);
    GFSDK_SSAO_Status SetInputNormals(const GFSDK_SSAO_InputNormalData_D3D11& NormalData);
    GFSDK_SSAO_Status SetAOParameters(const GFSDK_SSAO_Parameters& Parameters);
    GFSDK_SSAO_Status SetOutput(const GFSDK_SSAO_Output_D3D11& Output);
    GFSDK_SSAO_Status ValidateDataFlow();

    void DrawLinearDepthPS(ID3D11DeviceContext* pDeviceContext);
    void DrawDeinterleavedDepthPS(ID3D11DeviceContext* pDeviceContext);
    void DrawReconstructedNormalPS(ID3D11DeviceContext* pDeviceContext);
    void DrawCoarseAOPS(ID3D11DeviceContext* pDeviceContext);
    void DrawReinterleavedAOPS(ID3D11DeviceContext* pDeviceContext);
    void DrawReinterleavedAOPS_PreBlur(ID3D11DeviceContext* pDeviceContext);

    void Render(ID3D11DeviceContext* pDeviceContext, GFSDK_SSAO_RenderMask RenderMask);
    void RenderHBAOPlus(ID3D11DeviceContext* pDeviceContext, GFSDK_SSAO_RenderMask RenderMask);

    void DrawBlurXPS(ID3D11DeviceContext* pDeviceContext);
    void DrawBlurYPS(ID3D11DeviceContext* pDeviceContext);
    void DrawDebugNormalsPS(ID3D11DeviceContext* pDeviceContext);

#if ENABLE_DEBUG_MODES
    void DrawFullResAOPS(ID3D11DeviceContext* pDeviceContext);
    void RenderDebugAO(ID3D11DeviceContext* pDeviceContext);
    GFSDK::SSAO::D3D11::RandomTexture m_RandomTexture;
#endif

    GFSDK::SSAO::D3D11::GlobalConstantBuffer m_GlobalCB;
    GFSDK::SSAO::D3D11::PerPassConstantBuffers m_PerPassCBs;
    GFSDK::SSAO::D3D11::InputDepthInfo m_InputDepth;
    GFSDK::SSAO::D3D11::InputNormalInfo m_InputNormal;
    GFSDK::SSAO::D3D11::OutputInfo m_Output;
    GFSDK::SSAO::D3D11::RenderTargets m_RTs;
    GFSDK::SSAO::D3D11::Shaders m_Shaders;
    GFSDK::SSAO::D3D11::States m_States;
    GFSDK_SSAO_Parameters m_Options;
    GFSDK::SSAO::Viewports m_Viewports;
    GFSDK::SSAO::BuildVersion m_BuildVersion;
    GFSDK_SSAO_CustomHeap m_NewDelete;
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::D3D11::TimestampQueries m_TimestampQueries;
#endif
#if ENABLE_DEBUG_MODES
    GFSDK::SSAO::D3D11::DebugShaders m_DebugShaders;
#endif
};

} // namespace D3D11

#endif // SUPPORT_D3D11

} // namespace SSAO
} // namespace GFSDK
