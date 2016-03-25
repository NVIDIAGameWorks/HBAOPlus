/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "Renderer_GL.h"

#if SUPPORT_GL

#if ENABLE_RENDER_TIMES
GFSDK::SSAO::RenderTimes GFSDK::SSAO::GL::Renderer::s_RenderTimes;
#endif

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::Create(GFSDK_SSAO_Version HeaderVersion)
{
    if (!m_BuildVersion.Match(HeaderVersion))
    {
        return GFSDK_SSAO_VERSION_MISMATCH;
    }

#if ENABLE_EXCEPTIONS
    try
    {
        CreateResources();
    }
    catch(...)
    {
        ReleaseResources();

        return GFSDK_SSAO_GL_RESOURCE_CREATION_FAILED;
    }
#else
    CreateResources();
#endif

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::Release()
{
    ReleaseResources();

    m_NewDelete.delete_(this);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::CreateResources()
{
    m_GlobalCB.Create(m_GL);
    m_PerPassCBs.Create(m_GL);
    m_Shaders.Create(m_GL);
    m_VAO.Create(m_GL);
#if ENABLE_RENDER_TIMES
    m_TimestampQueries.Create();
#endif
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::ReleaseResources()
{
    m_GlobalCB.Release(m_GL);
    m_PerPassCBs.Release(m_GL);
    m_Shaders.Release(m_GL);
    m_VAO.Release(m_GL);
    m_RTs.Release(m_GL);
#if ENABLE_RENDER_TIMES
    m_TimestampQueries.Release();
#endif
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::PreCreateFBOs(
    const GFSDK_SSAO_Parameters& Parameters,
    UINT ViewportWidth,
    UINT ViewportHeight)
{
    SetAOResolution(ViewportWidth, ViewportHeight);

    m_Options.SetRenderOptions(Parameters);

    return m_RTs.PreCreate(m_GL, m_Options);
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::GetProjectionMatrixDepthRange(
    const GFSDK_SSAO_InputData_GL& InputData,
    GFSDK_SSAO_ProjectionMatrixDepthRange& OutputDepthRange)
{
    GFSDK_SSAO_Status Status;

    SSAO::ProjectionMatrixInfo ProjectionMatrixInfo;
    Status = ProjectionMatrixInfo.Init(InputData.DepthData.ProjectionMatrix, API_GL);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    ProjectionMatrixInfo.GetDepthRange(&OutputDepthRange);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::RenderAO(
    const GFSDK_SSAO_InputData_GL& InputData,
    const GFSDK_SSAO_Parameters& Parameters,
    const GFSDK_SSAO_Output_GL& Output,
    GFSDK_SSAO_RenderMask RenderMask)
{
    GFSDK_SSAO_Status Status;

    Status = SetDataFlow(InputData, Parameters, Output);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    Status = m_RTs.PreCreate(m_GL, m_Options);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    GFSDK::SSAO::GL::AppState AppState;
    AppState.Save(m_GL);

    Render(RenderMask);

    AppState.Restore(m_GL);

#if ENABLE_RENDER_TIMES
    m_TimestampQueries.GetAvailableTimers(&s_RenderTimes);
#endif

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::SetInputData(const GFSDK_SSAO_InputData_GL& InputData)
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
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::SetDataFlow(
    const GFSDK_SSAO_InputData_GL& InputData,
    const GFSDK_SSAO_Parameters& Parameters,
    const GFSDK_SSAO_Output_GL& Output)
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
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::SetInputDepths(const GFSDK_SSAO_InputDepthData_GL& DepthData)
{
    m_InputDepth = GFSDK::SSAO::GL::InputDepthInfo();

    GFSDK_SSAO_Status Status = m_InputDepth.SetData(m_GL, DepthData);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    m_GlobalCB.SetDepthData(m_InputDepth);

    SetAOResolution(UINT(m_InputDepth.Texture.Width), UINT(m_InputDepth.Texture.Height));

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::SetInputNormals(const GFSDK_SSAO_InputNormalData_GL& NormalData)
{
    m_InputNormal = GFSDK::SSAO::GL::InputNormalInfo();

    if (!NormalData.Enable)
    {
        // Input normals disabled. In this case, the lib reconstructs normals from depths.
        return GFSDK_SSAO_OK;
    }

    GFSDK_SSAO_Status Status = m_InputNormal.SetData(m_GL, NormalData);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    m_GlobalCB.SetNormalData(NormalData);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::SetOutput(const GFSDK_SSAO_Output_GL& Output)
{
    m_Output = GFSDK::SSAO::GL::OutputInfo();

    GFSDK_SSAO_Status Status = m_Output.Init(Output);
    if (Status != GFSDK_SSAO_OK)
    {
        return Status;
    }

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::SetAOResolution(UINT Width, UINT Height)
{
    if (Width  != m_RTs.GetFullWidth() ||
        Height != m_RTs.GetFullHeight())
    {
        m_RTs.ReleaseResources(m_GL);
        m_RTs.SetFullResolution(Width, Height);
        m_Viewports.SetFullResolution(Width, Height);
        m_GlobalCB.SetResolutionConstants(m_Viewports);
    }
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::ValidateDataFlow()
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

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
GFSDK_SSAO_Status GFSDK::SSAO::GL::Renderer::SetAOParameters(const GFSDK_SSAO_Parameters& Params)
{
    if (Params.Blur.Enable != m_Options.Blur.Enable ||
        Params.DepthStorage != m_Options.DepthStorage)
    {
        m_RTs.ReleaseResources(m_GL);
    }

    m_GlobalCB.SetAOParameters(Params, m_InputDepth);
    m_Options.SetRenderOptions(Params);

    return GFSDK_SSAO_OK;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::Render(GFSDK_SSAO_RenderMask RenderMask)
{
    m_GlobalCB.UpdateBuffer(m_GL, RenderMask);

    SetFullscreenState();

    RenderHBAOPlus(RenderMask);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::SetFullscreenState()
{
    m_States.SetRasterizerStateFullscreenNoScissor(m_GL);
    m_States.SetDepthStencilStateDisabled(m_GL);
    m_States.SetBlendStateDisabled(m_GL);
    m_States.SetSharedBlendState(m_GL);

    // Our draw calls do not source any vertex or index buffers
    // but we still need to bind a dummy VAO to avoid GL errors on OSX
    m_VAO.Bind(m_GL);

    SetFullViewport();
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::RenderHBAOPlus(GFSDK_SSAO_RenderMask RenderMask)
{
    ASSERT_GL_ERROR(m_GL);

    if (RenderMask & GFSDK_SSAO_DRAW_Z)
    {
        DrawLinearDepth(GetCopyDepthPS());
    }

    if (RenderMask & GFSDK_SSAO_DRAW_DEBUG_N)
    {
        DrawDebugNormals(m_Shaders.DebugNormals_PS.Get(GetFetchNormalPermutation()));
    }

    if (RenderMask & GFSDK_SSAO_DRAW_AO)
    {
        DrawDeinterleavedDepth(m_Shaders.DeinterleaveDepth_PS.Get());

        if (!m_InputNormal.Texture.IsSet())
        {
            DrawReconstructedNormal(m_Shaders.ReconstructNormal_PS.Get());
        }

        DrawCoarseAO(m_Shaders.CoarseAO_PS.Get(GetEnableForegroundAOPermutation(), GetEnableBackgroundAOPermutation(), GetEnableDepthThresholdPermutation(), GetFetchNormalPermutation()));

        if (m_Options.Blur.Enable)
        {
            DrawReinterleavedAO_PreBlur(m_Shaders.ReinterleaveAO_PS.Get(GetEnableBlurPermutation()));
            DrawBlurX(m_Shaders.BlurX_PS.Get(GetEnableSharpnessProfilePermutation(), GetBlurKernelRadiusPermutation()));
            DrawBlurY(m_Shaders.BlurY_PS.Get(GetEnableSharpnessProfilePermutation(), GetBlurKernelRadiusPermutation()));
        }
        else
        {
            DrawReinterleavedAO(m_Shaders.ReinterleaveAO_PS.Get(GetEnableBlurPermutation()));
        }
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawLinearDepth(GFSDK::SSAO::GL::GLSLPrograms::CopyDepth_PS& Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_LINEAR_Z);
#endif

    if (m_InputDepth.DepthTextureType == GFSDK_SSAO_VIEW_DEPTHS &&
        m_InputDepth.Texture.SampleCount == 1 &&
        m_InputDepth.Viewport.RectCoversFullInputTexture)
    {
        m_FullResViewDepthTextureId = m_InputDepth.Texture.GLTexture.TextureId;
        return;
    }

    {
        m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RTs.GetFullResViewDepthTexture(m_GL)->GetFramebuffer());

        Program.Enable(m_GL);
        Program.SetDepthTexture(m_GL, m_InputDepth.Texture.GLTexture);

        m_GL.glDrawArrays(GL_TRIANGLES, 0, 3);
        ASSERT_GL_ERROR(m_GL);

        m_FullResViewDepthTextureId = m_RTs.GetFullResViewDepthTexture(m_GL)->GetTexture().TextureId;
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawDebugNormals(GLSLPrograms::DebugNormals_PS& Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_NORMAL);
#endif

    m_States.SetBlendStateDisabled(m_GL);

    m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Output.FboId);

    Program.Enable(m_GL);
    Program.SetDepthTexture(m_GL, m_FullResViewDepthTextureId, GetAODepthWrapMode());
    Program.SetNormalTexture(m_GL, m_InputNormal.Texture.GLTexture);

    m_GL.glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_ERROR(m_GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawDeinterleavedDepth(GLSLPrograms::DeinterleaveDepth_PS& Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_DEINTERLEAVE_Z);
#endif

    SetQuarterViewport();

    Program.Enable(m_GL);
    Program.SetDepthTexture(m_GL, m_FullResViewDepthTextureId);

    UINT PassIndex = 0;
    for (UINT SliceIndex = 0; SliceIndex < 16; SliceIndex += MAX_NUM_MRTS)
    {
        m_GL.glBindBufferBase(GL_UNIFORM_BUFFER, m_PerPassCBs.GetBindingPoint(), m_PerPassCBs.GetBufferId(SliceIndex));
        m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RTs.GetQuarterResViewDepthTextureArray(m_GL, m_Options)->GetOctaSliceFramebuffer(PassIndex++));
        m_GL.glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    SetFullViewport();
    ASSERT_GL_ERROR(m_GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawReconstructedNormal(GLSLPrograms::ReconstructNormal_PS& Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_NORMAL);
#endif

    m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RTs.GetFullResNormalTexture(m_GL)->GetFramebuffer());

    Program.Enable(m_GL);
    Program.SetDepthTexture(m_GL, m_FullResViewDepthTextureId, GetAODepthWrapMode());

    m_GL.glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_ERROR(m_GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawCoarseAO(GLSLPrograms::CoarseAO_PS& Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_COARSE_AO);
#endif

    SetQuarterViewport();

    Program.Enable(m_GL);
    Program.SetDepthTexture(m_GL, m_RTs.GetQuarterResViewDepthTextureArray(m_GL, m_Options)->GetTextureArray(), GetAODepthWrapMode());
    Program.SetNormalTexture(m_GL, GetFullResNormalTexture());

    m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RTs.GetQuarterResAOTextureArray(m_GL)->GetLayeredFramebuffer());

    for (UINT SliceIndex = 0; SliceIndex < 16; ++SliceIndex)
    {
        m_GL.glBindBufferBase(GL_UNIFORM_BUFFER, m_PerPassCBs.GetBindingPoint(), m_PerPassCBs.GetBufferId(SliceIndex));
        m_GL.glDrawArrays(GL_POINTS, 0, 1);
    }

    SetFullViewport();
    ASSERT_GL_ERROR(m_GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawReinterleavedAO(GLSLPrograms::ReinterleaveAO_PS &Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_INTERLEAVE_AO);
#endif

    ASSERT(!m_Options.Blur.Enable);

    m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Output.FboId);
    SetOutputBlendState(m_GL);
    SetFullViewport();

    Program.Enable(m_GL);
    Program.SetAOTexture(m_GL, m_RTs.GetQuarterResAOTextureArray(m_GL)->GetTextureArray());

    m_GL.glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_ERROR(m_GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawReinterleavedAO_PreBlur(GLSLPrograms::ReinterleaveAO_PS& Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_INTERLEAVE_AO);
#endif

    ASSERT(m_Options.Blur.Enable);

    m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RTs.GetFullResAOZTexture2(m_GL)->GetFramebuffer());

    Program.Enable(m_GL);
    Program.SetAOTexture(m_GL, m_RTs.GetQuarterResAOTextureArray(m_GL)->GetTextureArray());
    Program.SetDepthTexture(m_GL, m_FullResViewDepthTextureId);

    m_GL.glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_ERROR(m_GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawBlurX(GLSLPrograms::BlurX_PS &Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_BLURX);
#endif

    m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RTs.GetFullResAOZTexture(m_GL)->GetFramebuffer());

    Program.Enable(m_GL);
    Program.SetAODepthTexture(m_GL, m_RTs.GetFullResAOZTexture2(m_GL)->GetTexture().TextureId);

    m_GL.glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_ERROR(m_GL);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GL::Renderer::DrawBlurY(GLSLPrograms::BlurY_PS &Program)
{
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::GPUTimer timer(&m_TimestampQueries, GFSDK::SSAO::REGIME_TIME_BLURY);
#endif

    m_GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_Output.FboId);
    SetOutputBlendState(m_GL);
    SetFullViewport();

    Program.Enable(m_GL);
    Program.SetAODepthTexture(m_GL, m_RTs.GetFullResAOZTexture(m_GL)->GetTexture().TextureId);

    m_GL.glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_ERROR(m_GL);
}

//--------------------------------------------------------------------------------
UINT GFSDK::SSAO::GL::Renderer::GetAllocatedVideoMemoryBytes()
{
    return m_RTs.GetCurrentAllocatedVideoMemoryBytes();
}

#endif // SUPPORT_GL
