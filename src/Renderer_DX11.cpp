/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if SUPPORT_D3D11

#include "Renderer_DX11.h"

#if ENABLE_RENDER_TIMES
GFSDK::SSAO::RenderTimes GFSDK::SSAO::D3D11::Renderer::s_RenderTimes;
#endif

#if ENABLE_DEBUG_MODES
GFSDK::SSAO::D3D11::Renderer::ShaderType GFSDK::SSAO::D3D11::Renderer::s_AOShaderType = NVSDK_HBAO_PLUS_PS;
#endif

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::CreateResources(ID3D11Device* pD3DDevice)
{
    m_States.Create(pD3DDevice);
    m_Shaders.Create(pD3DDevice);
#if ENABLE_RENDER_TIMES
    m_TimestampQueries.Create(pD3DDevice);
#endif
#if ENABLE_DEBUG_MODES
    m_RandomTexture.Create(pD3DDevice);
    m_DebugShaders.Create(pD3DDevice);
#endif
    m_GlobalCB.Create(pD3DDevice);
    m_PerPassCBs.Create(pD3DDevice);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::ReleaseResources()
{
    m_States.Release();
    m_Shaders.Release();
    m_RTs.Release();
#if ENABLE_RENDER_TIMES
    m_TimestampQueries.Release();
#endif
#if ENABLE_DEBUG_MODES
    m_RandomTexture.Release();
    m_DebugShaders.Release();
#endif
    m_GlobalCB.Release();
    m_PerPassCBs.Release();
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawLinearDepthPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawLinearDepthPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(LINEAR_Z);
#endif

    if (m_InputDepth.DepthTextureType == GFSDK_SSAO_VIEW_DEPTHS &&
        m_InputDepth.Texture.SampleCount == 1 &&
        m_InputDepth.Viewport.RectCoversFullInputTexture)
    {
        m_FullResViewDepthSRV = m_InputDepth.Texture.pSRV;
        return;
    }

    {
        ID3D11PixelShader* pPS = (m_InputDepth.DepthTextureType == GFSDK_SSAO_VIEW_DEPTHS) ?
            m_Shaders.CopyDepth_PS.Get(GetResolveDepthPermutation()) :
            m_Shaders.LinearizeDepth_PS.Get(GetResolveDepthPermutation());

        pDeviceContext->OMSetRenderTargets(1, &m_RTs.GetFullResViewDepthTexture()->pRTV, NULL);
        pDeviceContext->RSSetViewports(1, &m_Viewports.FullRes);
        pDeviceContext->PSSetShader(pPS, NULL, 0);
        pDeviceContext->PSSetShaderResources(0, 1, &m_InputDepth.Texture.pSRV);

        pDeviceContext->Draw(3, 0);

        m_FullResViewDepthSRV = m_RTs.GetFullResViewDepthTexture()->pSRV;
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawDeinterleavedDepthPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawDeinterleavedDepthPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(DEINTERLEAVE_Z);
#endif

    pDeviceContext->RSSetViewports(1, &m_Viewports.QuarterRes);
    pDeviceContext->PSSetShader(m_Shaders.DeinterleaveDepth_PS.Get(), NULL, 0);
    pDeviceContext->PSSetSamplers(0, 1, &m_States.GetSamplerStatePointClamp());

    for (UINT SliceIndex = 0; SliceIndex < 16; SliceIndex += MAX_NUM_MRTS)
    {
        pDeviceContext->OMSetRenderTargets(MAX_NUM_MRTS, &m_RTs.GetQuarterResViewDepthTextureArray(m_Options)->pRTVs[SliceIndex], NULL);
        pDeviceContext->PSSetShaderResources(0, 1, &m_FullResViewDepthSRV);
        pDeviceContext->PSSetConstantBuffers(1, 1, &m_PerPassCBs.GetCB(SliceIndex));

        pDeviceContext->Draw(3, 0);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawReconstructedNormalPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawReconstructedNormalPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(NORMAL);
#endif

    pDeviceContext->OMSetRenderTargets(1, &m_RTs.GetFullResNormalTexture()->pRTV, NULL);
    pDeviceContext->RSSetViewports(1, &m_Viewports.FullRes);

    pDeviceContext->PSSetShader(m_Shaders.ReconstructNormal_PS.Get(), NULL, 0);
    pDeviceContext->PSSetSamplers(0, 1, &GetAODepthSamplerState());
    pDeviceContext->PSSetShaderResources(0, 1, &m_FullResViewDepthSRV);

    pDeviceContext->Draw(3, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawDebugNormalsPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawDebugNormalsPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(NORMAL);
#endif

    ID3D11ShaderResourceView* pSRVs[] = 
    {
        m_FullResViewDepthSRV,
        m_InputNormal.Texture.pSRV
    };

    pDeviceContext->OMSetBlendState(m_States.GetBlendStateDisabled(), NULL, GetOutputMSAASampleMask());

    pDeviceContext->OMSetRenderTargets(1, &m_Output.RenderTarget.pRTV, NULL);
    pDeviceContext->RSSetViewports(1, &m_InputDepth.Viewport);

    pDeviceContext->PSSetShader(m_Shaders.DebugNormals_PS.Get(GetFetchNormalPermutation()), NULL, 0);
    pDeviceContext->PSSetSamplers(0, 1, &GetAODepthSamplerState());
    pDeviceContext->PSSetShaderResources(0, SIZEOF_ARRAY(pSRVs), pSRVs);

    pDeviceContext->Draw(3, 0);
}

#if ENABLE_DEBUG_MODES

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawFullResAOPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(COARSE_AO);
#endif

    if (m_Options.Blur.Enable)
    {
        pDeviceContext->OMSetRenderTargets(1, &m_RTs.GetFullResAOZTexture2()->pRTV, NULL);
        pDeviceContext->RSSetViewports(1, &m_Viewports.FullRes);
    }
    else
    {
        pDeviceContext->OMSetDepthStencilState(GetOutputDepthStencilState(), GetOutputDepthStencilRef());
        pDeviceContext->OMSetBlendState(GetOutputBlendState(), GetOutputBlendFactor(), GetOutputMSAASampleMask());
        pDeviceContext->OMSetRenderTargets(1, &m_Output.RenderTarget.pRTV, GetOutputDepthStencilView());
        pDeviceContext->RSSetViewports(1, &m_InputDepth.Viewport);
    }

    ID3D11ShaderResourceView* pSRVs[] =
    {
        m_FullResViewDepthSRV,
        m_RandomTexture.pSRV
    };

    ID3D11SamplerState* pSamplers[] =
    {
        pSamplers[0] = m_States.GetSamplerStatePointClamp(),
        pSamplers[1] = m_States.GetSamplerStatePointWrap()
    };

    pDeviceContext->PSSetShader(m_DebugShaders.DebugAO_PS.Get(GetEnableBlurPermutation()), NULL, 0);
    pDeviceContext->PSSetShaderResources(0, SIZEOF_ARRAY(pSRVs), pSRVs);
    pDeviceContext->PSSetSamplers(0, SIZEOF_ARRAY(pSamplers), pSamplers);
    pDeviceContext->Draw(3, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::RenderDebugAO(ID3D11DeviceContext* pDeviceContext)
{
    DrawLinearDepthPS(pDeviceContext);
    DrawFullResAOPS(pDeviceContext);

    if (m_Options.Blur.Enable)
    {
        DrawBlurXPS(pDeviceContext);
        DrawBlurYPS(pDeviceContext);
    }
}

#endif // ENABLE_DEBUG_MODES

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawCoarseAOPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawCoarseAOPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(COARSE_AO);
#endif

    pDeviceContext->GSSetShader(m_Shaders.CoarseAO_GS.Get(), NULL, 0);

    ID3D11RenderTargetView* pOutputBufferRTV = m_RTs.GetQuarterResAOTextureArray()->pRTV;
    pDeviceContext->OMSetRenderTargets(1, &pOutputBufferRTV, NULL);

    pDeviceContext->RSSetViewports(1, &m_Viewports.QuarterRes);
    pDeviceContext->PSSetSamplers(0, 1, &GetAODepthSamplerState());
    pDeviceContext->PSSetShader(m_Shaders.CoarseAO_PS.Get(GetEnableForegroundAOPermutation(), GetEnableBackgroundAOPermutation(), GetEnableDepthThresholdPermutation(), GetFetchNormalPermutation()), NULL, 0);

    for (UINT SliceIndex = 0; SliceIndex < 16; ++SliceIndex)
    {
        ID3D11ShaderResourceView* pSRVs[] =
        {
            m_RTs.GetQuarterResViewDepthTextureArray(m_Options)->pSRVs[SliceIndex],
            GetFullResNormalBufferSRV()
        };
        pDeviceContext->PSSetShaderResources(0, SIZEOF_ARRAY(pSRVs), pSRVs);

        ID3D11Buffer *PerPassCB = m_PerPassCBs.GetCB(SliceIndex);
        pDeviceContext->GSSetConstantBuffers(1, 1, &PerPassCB);
        pDeviceContext->PSSetConstantBuffers(1, 1, &PerPassCB);

        pDeviceContext->Draw(3, 0);
    }

    pDeviceContext->GSSetShader(NULL, NULL, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawReinterleavedAOPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawReinterleavedAOPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(INTERLEAVE_AO);
#endif

    ASSERT(!m_Options.Blur.Enable);

    for (UINT PassIndex = 0; PassIndex < m_Output.BlendPassCount; ++PassIndex)
    {
        const BlendPassEnumType BlendPass = GetBlendPassEnum(PassIndex);

        pDeviceContext->OMSetDepthStencilState(GetOutputDepthStencilState(BlendPass), GetOutputDepthStencilRef(BlendPass));
        pDeviceContext->OMSetBlendState(GetOutputBlendState(BlendPass), GetOutputBlendFactor(BlendPass), GetOutputMSAASampleMask());
        pDeviceContext->OMSetRenderTargets(1, &m_Output.RenderTarget.pRTV, GetOutputDepthStencilView());
        pDeviceContext->RSSetViewports(1, &m_InputDepth.Viewport);

        pDeviceContext->PSSetShaderResources(0, 1, &m_RTs.GetQuarterResAOTextureArray()->pSRV);
        pDeviceContext->PSSetSamplers(0, 1, &m_States.GetSamplerStatePointClamp());
        pDeviceContext->PSSetShader(m_Shaders.ReinterleaveAO_PS.Get(GetEnableBlurPermutation()), NULL, 0);

        pDeviceContext->Draw(3, 0);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawReinterleavedAOPS_PreBlur(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawReinterleavedAOPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(INTERLEAVE_AO);
#endif

    ASSERT(m_Options.Blur.Enable);

    ID3D11ShaderResourceView* pSRVs[] =
    {
        m_RTs.GetQuarterResAOTextureArray()->pSRV,
        m_FullResViewDepthSRV
    };

    pDeviceContext->OMSetRenderTargets(1, &m_RTs.GetFullResAOZTexture2()->pRTV, NULL);
    pDeviceContext->RSSetViewports(1, &m_Viewports.FullRes);

    pDeviceContext->PSSetShaderResources(0, SIZEOF_ARRAY(pSRVs), pSRVs);
    pDeviceContext->PSSetSamplers(0, 1, &m_States.GetSamplerStatePointClamp());
    pDeviceContext->PSSetShader(m_Shaders.ReinterleaveAO_PS.Get(GetEnableBlurPermutation()), NULL, 0);

    pDeviceContext->Draw(3, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawBlurXPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawBlurXPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(BLURX);
#endif

    ID3D11SamplerState* pSamplers[] =
    {
        m_States.GetSamplerStatePointClamp(),
        m_States.GetSamplerStateLinearClamp()
    };

    pDeviceContext->OMSetRenderTargets(1, &m_RTs.GetFullResAOZTexture()->pRTV, NULL);
    pDeviceContext->RSSetViewports(1, &m_Viewports.FullRes);

    pDeviceContext->PSSetShader(m_Shaders.BlurX_PS.Get(GetEnableSharpnessProfilePermutation(), GetBlurKernelRadiusPermutation()), NULL, 0);
    pDeviceContext->PSSetShaderResources(0, 1, &m_RTs.GetFullResAOZTexture2()->pSRV);
    pDeviceContext->PSSetSamplers(0, SIZEOF_ARRAY(pSamplers), pSamplers);

    pDeviceContext->Draw(3, 0);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::DrawBlurYPS(ID3D11DeviceContext* pDeviceContext)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"DrawBlurYPS");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(BLURY);
#endif

    for (UINT PassIndex = 0; PassIndex < m_Output.BlendPassCount; ++PassIndex)
    {
        const BlendPassEnumType BlendPass = GetBlendPassEnum(PassIndex);

        pDeviceContext->OMSetDepthStencilState(GetOutputDepthStencilState(BlendPass), GetOutputDepthStencilRef(BlendPass));
        pDeviceContext->OMSetBlendState(GetOutputBlendState(BlendPass), GetOutputBlendFactor(BlendPass), GetOutputMSAASampleMask());

        pDeviceContext->OMSetRenderTargets(1, &m_Output.RenderTarget.pRTV, GetOutputDepthStencilView());
        pDeviceContext->RSSetViewports(1, &m_InputDepth.Viewport);

        pDeviceContext->PSSetShader(m_Shaders.BlurY_PS.Get(GetEnableSharpnessProfilePermutation(), GetBlurKernelRadiusPermutation()), NULL, 0);
        pDeviceContext->PSSetShaderResources(0, 1, &m_RTs.GetFullResAOZTexture()->pSRV);

        pDeviceContext->Draw(3, 0);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::SetFullscreenState(ID3D11DeviceContext* pDeviceContext)
{
    GFSDK::SSAO::D3D11::AppState::UnbindSRVs(pDeviceContext);

    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pDeviceContext->IASetInputLayout(NULL);

    pDeviceContext->RSSetState(m_States.GetRasterizerStateFullscreenNoScissor());
    pDeviceContext->RSSetViewports(1, &m_Viewports.FullRes);

    pDeviceContext->VSSetShader(m_Shaders.FullScreenTriangle_VS.Get(), NULL, 0);
    pDeviceContext->HSSetShader(NULL, NULL, 0);
    pDeviceContext->DSSetShader(NULL, NULL, 0);
    pDeviceContext->GSSetShader(NULL, NULL, 0);
    pDeviceContext->CSSetShader(NULL, NULL, 0);

    pDeviceContext->PSSetConstantBuffers(0, 1, &m_GlobalCB.GetCB());
    pDeviceContext->PSSetSamplers(0, 1, &m_States.GetSamplerStatePointClamp());

    pDeviceContext->OMSetDepthStencilState(m_States.GetDepthStencilStateDisabled(), 0x0);
    pDeviceContext->OMSetBlendState(m_States.GetBlendStateDisabled(), NULL, 0xFFFFFFFF);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::RenderHBAOPlus(ID3D11DeviceContext* pDeviceContext, GFSDK_SSAO_RenderMask RenderMask)
{
    if (RenderMask & GFSDK_SSAO_DRAW_Z)
    {
        DrawLinearDepthPS(pDeviceContext);
    }

    if (RenderMask & GFSDK_SSAO_DRAW_DEBUG_N)
    {
        DrawDebugNormalsPS(pDeviceContext);
    }

    if (RenderMask & GFSDK_SSAO_DRAW_AO)
    {
        DrawDeinterleavedDepthPS(pDeviceContext);

        if (!m_InputNormal.Texture.IsSet())
        {
            DrawReconstructedNormalPS(pDeviceContext);
        }

        DrawCoarseAOPS(pDeviceContext);

        if (m_Options.Blur.Enable)
        {
            DrawReinterleavedAOPS_PreBlur(pDeviceContext);
            DrawBlurXPS(pDeviceContext);
            DrawBlurYPS(pDeviceContext);
        }
        else
        {
            DrawReinterleavedAOPS(pDeviceContext);
        }
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::Render(ID3D11DeviceContext* pDeviceContext, GFSDK_SSAO_RenderMask RenderMask)
{
#if ENABLE_PERF_MARKERS
    PERF_MARKER_SCOPE(L"HBAO+");
#endif

#if ENABLE_RENDER_TIMES
    GPU_TIMER_SCOPE(TOTAL);
#endif

    m_GlobalCB.UpdateBuffer(pDeviceContext, RenderMask);

    SetFullscreenState(pDeviceContext);

#if ENABLE_DEBUG_MODES
    if (s_AOShaderType != NVSDK_HBAO_PLUS_PS)
    {
        RenderDebugAO(pDeviceContext);
    }
    else
#endif
    {
        RenderHBAOPlus(pDeviceContext, RenderMask);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::SetAOResolution(UINT Width, UINT Height)
{
    if (Width  != m_RTs.GetFullWidth() ||
        Height != m_RTs.GetFullHeight())
    {
        m_RTs.ReleaseResources();
        m_RTs.SetFullResolution(Width, Height);
        m_Viewports.SetFullResolution(Width, Height);
        m_GlobalCB.SetResolutionConstants(m_Viewports);
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::SetAOResolution(const SSAO::InputViewport &Viewport)
{
    SetAOResolution(UINT(Viewport.Width), UINT(Viewport.Height));
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::PreCreateRTs(
    const GFSDK_SSAO_Parameters& Parameters,
    UINT ViewportWidth,
    UINT ViewportHeight)
{
    SetAOResolution(ViewportWidth, ViewportHeight);

    m_Options.SetRenderOptions(Parameters);

    return m_RTs.PreCreate(m_Options);
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::GetProjectionMatrixDepthRange(
    const GFSDK_SSAO_InputData_D3D11& InputData,
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
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::RenderAO(
    ID3D11DeviceContext* pDeviceContext,
    const GFSDK_SSAO_InputData_D3D11& InputData,
    const GFSDK_SSAO_Parameters& Parameters,
    const GFSDK_SSAO_Output_D3D11& Output,
    GFSDK_SSAO_RenderMask RenderMask)
{
    GFSDK_SSAO_Status Status;

    if (!pDeviceContext)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

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

    GFSDK::SSAO::D3D11::AppState AppState;
    AppState.Save(pDeviceContext);

#if ENABLE_RENDER_TIMES
    m_TimestampQueries.Begin(pDeviceContext);
#endif

    Render(pDeviceContext, RenderMask);

#if ENABLE_RENDER_TIMES
    m_TimestampQueries.End(pDeviceContext, &s_RenderTimes);
#endif

    AppState.Restore(pDeviceContext);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::SetInputData(const GFSDK_SSAO_InputData_D3D11& InputData)
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
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::SetDataFlow(
    const GFSDK_SSAO_InputData_D3D11& InputData,
    const GFSDK_SSAO_Parameters& Parameters,
    const GFSDK_SSAO_Output_D3D11& Output)
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
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::SetInputDepths(const GFSDK_SSAO_InputDepthData_D3D11& DepthData)
{
    m_InputDepth = GFSDK::SSAO::D3D11::InputDepthInfo();

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
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::SetInputNormals(const GFSDK_SSAO_InputNormalData_D3D11& NormalData)
{
    m_InputNormal = GFSDK::SSAO::D3D11::InputNormalInfo();

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
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::SetOutput(const GFSDK_SSAO_Output_D3D11& Output)
{
    m_Output = GFSDK::SSAO::D3D11::OutputInfo();

    GFSDK_SSAO_Status Status = m_Output.Init(Output);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::ValidateDataFlow()
{
    if (m_InputNormal.Texture.IsSet())
    {
        if (m_InputNormal.Texture.Width    != m_InputDepth.Texture.Width ||
            m_InputNormal.Texture.Height   != m_InputDepth.Texture.Height)
        {
            return GFSDK_SSAO_INVALID_NORMAL_TEXTURE_RESOLUTION;
        }
        if (m_InputNormal.Texture.SampleCount != m_InputDepth.Texture.SampleCount)
        {
            return GFSDK_SSAO_INVALID_NORMAL_TEXTURE_SAMPLE_COUNT;
        }
    }

    if (m_Output.DepthStencilBuffer.IsSet())
    {
        if (m_Output.RenderTarget.Width  != m_Output.DepthStencilBuffer.Width ||
            m_Output.RenderTarget.Height != m_Output.DepthStencilBuffer.Height)
        {
            return GFSDK_SSAO_INVALID_DEPTH_STENCIL_RESOLUTION;
        }
        if (m_Output.RenderTarget.SampleCount != m_Output.DepthStencilBuffer.SampleCount)
        {
            return GFSDK_SSAO_INVALID_DEPTH_STENCIL_SAMPLE_COUNT;
        }
    }

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::SetAOParameters(const GFSDK_SSAO_Parameters& Params)
{
    if (Params.Blur.Enable != m_Options.Blur.Enable ||
        Params.DepthStorage != m_Options.DepthStorage)
    {
        m_RTs.ReleaseResources();
    }

    m_GlobalCB.SetAOParameters(Params, m_InputDepth);
    m_Options.SetRenderOptions(Params);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::D3D11::Renderer::Create(ID3D11Device* pD3DDevice, GFSDK_SSAO_Version HeaderVersion)
{
    if (!m_BuildVersion.Match(HeaderVersion))
    {
        return GFSDK_SSAO_VERSION_MISMATCH;
    }

    if (pD3DDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
    {
        return GFSDK_SSAO_D3D_FEATURE_LEVEL_NOT_SUPPORTED;
    }

    if (!pD3DDevice)
    {
        return GFSDK_SSAO_NULL_ARGUMENT;
    }

#if ENABLE_EXCEPTIONS
    try
    {
        CreateResources(pD3DDevice);
    }
    catch (...)
    {
        ReleaseResources();

        return GFSDK_SSAO_D3D_RESOURCE_CREATION_FAILED;
    }
#else
    CreateResources(pD3DDevice);
#endif

    m_RTs.SetDevice(pD3DDevice);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::Renderer::Release()
{
    ReleaseResources();

    m_NewDelete.delete_(this);
}

//--------------------------------------------------------------------------------
UINT GFSDK::SSAO::D3D11::Renderer::GetAllocatedVideoMemoryBytes()
{
    return m_RTs.GetCurrentAllocatedVideoMemoryBytes();
}

#endif // SUPPORT_D3D11
