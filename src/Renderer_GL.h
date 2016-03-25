/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#if SUPPORT_GL

#pragma once
#include "Common.h"
#include "ConstantBuffers.h"
#include "InputDepthInfo.h"
#include "InputNormalInfo.h"
#include "OutputInfo.h"
#include "ProjectionMatrixInfo.h"
#include "RenderOptions.h"
#include "RenderTargets_GL.h"
#include "Shaders_GL.h"
#include "States_GL.h"
#include "AppState_GL.h"
#include "TimestampQueries.h"
#include "BuildVersion.h"
#include "VAO_GL.h"

namespace GFSDK
{
namespace SSAO
{
namespace GL
{

//--------------------------------------------------------------------------------
class Renderer : public GFSDK_SSAO_Context_GL
{
public:
    Renderer(const GFSDK_SSAO_GLFunctions& GL, GFSDK_SSAO_CustomHeap NewDelete)
        : m_GL(GL)
        , m_NewDelete(NewDelete)
        , m_FullResViewDepthTextureId(0)
    {
    }

    //
    // API
    //

    GFSDK_SSAO_Status Create(GFSDK_SSAO_Version HeaderVersion);

    UINT GetAllocatedVideoMemoryBytes();

    void Release();

    GFSDK_SSAO_Status PreCreateFBOs(
        const GFSDK_SSAO_Parameters& Parameters,
        UINT ViewportWidth,
        UINT ViewportHeight);

    GFSDK_SSAO_Status GetProjectionMatrixDepthRange(
        const GFSDK_SSAO_InputData_GL& InputData,
        GFSDK_SSAO_ProjectionMatrixDepthRange& OutputDepthRange);

    GFSDK_SSAO_Status RenderAO(
        const GFSDK_SSAO_InputData_GL& InputData,
        const GFSDK_SSAO_Parameters& Parameters,
        const GFSDK_SSAO_Output_GL& Output,
        GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO);

    //
    // Internals
    //

#if ENABLE_RENDER_TIMES
    static GFSDK::SSAO::RenderTimes s_RenderTimes;
#endif

private:
    void CreateResources();
    void ReleaseResources();
    void SetAOResolution(UINT Width, UINT Height);

    void SetFullscreenState();
    void Render(GFSDK_SSAO_RenderMask RenderMask);
    void RenderHBAOPlus(GFSDK_SSAO_RenderMask RenderMask);

    Generated::ShaderPermutations::RESOLVE_DEPTH GetResolveDepthPermutation()
    {
        return (m_InputDepth.Texture.SampleCount == 1) ?    Generated::ShaderPermutations::RESOLVE_DEPTH_0 :
                                                            Generated::ShaderPermutations::RESOLVE_DEPTH_1;
    }
    Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL GetFetchNormalPermutation()
    {
        return (!m_InputNormal.Texture.IsSet())          ?  Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL_0 :
               (m_InputNormal.Texture.SampleCount == 1)  ?  Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL_1 :
                                                            Generated::ShaderPermutations::FETCH_GBUFFER_NORMAL_2;
    }
    Generated::ShaderPermutations::ENABLE_FOREGROUND_AO GetEnableForegroundAOPermutation()
    {
        return (m_Options.EnableForegroundAO) ?     Generated::ShaderPermutations::ENABLE_FOREGROUND_AO_1 :
                                                    Generated::ShaderPermutations::ENABLE_FOREGROUND_AO_0;
    }
    Generated::ShaderPermutations::ENABLE_BACKGROUND_AO GetEnableBackgroundAOPermutation()
    {
        return (m_Options.EnableBackgroundAO) ?     Generated::ShaderPermutations::ENABLE_BACKGROUND_AO_1 :
                                                    Generated::ShaderPermutations::ENABLE_BACKGROUND_AO_0;
    }
    Generated::ShaderPermutations::ENABLE_DEPTH_THRESHOLD GetEnableDepthThresholdPermutation()
    {
        return (m_Options.EnableDepthThreshold) ?   Generated::ShaderPermutations::ENABLE_DEPTH_THRESHOLD_1 :
                                                    Generated::ShaderPermutations::ENABLE_DEPTH_THRESHOLD_0;
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
    GFSDK::SSAO::GL::GLSLPrograms::CopyDepth_PS& GetCopyDepthPS()
    {
         return (m_InputDepth.DepthTextureType == GFSDK_SSAO_VIEW_DEPTHS) ?
            m_Shaders.CopyDepth_PS.Get(GetResolveDepthPermutation()) :
            m_Shaders.LinearizeDepth_PS.Get(GetResolveDepthPermutation());
    }

    GLint GetAODepthWrapMode()
    {
        return (m_Options.DepthClampMode == GFSDK_SSAO_CLAMP_TO_EDGE) ? GL_CLAMP_TO_EDGE : GL_CLAMP_TO_BORDER;
    }
    GFSDK_SSAO_Texture_GL GetFullResNormalTexture()
    {
        return (m_InputNormal.Texture.IsSet()) ? m_InputNormal.Texture.GLTexture : m_RTs.GetFullResNormalTexture(m_GL)->GetTexture();
    }

    void DrawLinearDepth(GFSDK::SSAO::GL::GLSLPrograms::CopyDepth_PS& Program);
    void DrawDebugNormals(GLSLPrograms::DebugNormals_PS& Program);
    void DrawDeinterleavedDepth(GLSLPrograms::DeinterleaveDepth_PS& Program);
    void DrawReconstructedNormal(GLSLPrograms::ReconstructNormal_PS& Program);
    void DrawCoarseAO(GLSLPrograms::CoarseAO_PS& Program);
    void DrawReinterleavedAO(GLSLPrograms::ReinterleaveAO_PS& Program);
    void DrawReinterleavedAO_PreBlur(GLSLPrograms::ReinterleaveAO_PS& Program);
    void DrawBlurX(GLSLPrograms::BlurX_PS &Program);
    void DrawBlurY(GLSLPrograms::BlurY_PS &Program);

    GFSDK_SSAO_Status SetDataFlow(const GFSDK_SSAO_InputData_GL& InputData, const GFSDK_SSAO_Parameters& Parameters, const GFSDK_SSAO_Output_GL& Output);
    GFSDK_SSAO_Status ValidateDataFlow();
    GFSDK_SSAO_Status SetInputData(const GFSDK_SSAO_InputData_GL& InputData);
    GFSDK_SSAO_Status SetInputDepths(const GFSDK_SSAO_InputDepthData_GL& DepthData);
    GFSDK_SSAO_Status SetInputNormals(const GFSDK_SSAO_InputNormalData_GL& NormalData);
    GFSDK_SSAO_Status SetAOParameters(const GFSDK_SSAO_Parameters& Parameters);
    GFSDK_SSAO_Status SetOutput(const GFSDK_SSAO_Output_GL& Output);

    void SetFullViewport()
    {
        m_GL.glViewport(
            GLint(m_Viewports.FullRes.TopLeftX), 
            GLint(m_Viewports.FullRes.TopLeftY),
            GLint(m_Viewports.FullRes.Width),
            GLint(m_Viewports.FullRes.Height));
    }

    void SetQuarterViewport()
    {
        m_GL.glViewport(
            GLint(m_Viewports.QuarterRes.TopLeftX),
            GLint(m_Viewports.QuarterRes.TopLeftY),
            GLint(m_Viewports.QuarterRes.Width),
            GLint(m_Viewports.QuarterRes.Height));
    }

    void SetOutputBlendState(const GFSDK_SSAO_GLFunctions& GL)
    {
        if (m_Output.Blend.Mode == GFSDK_SSAO_OVERWRITE_RGB)
        {
            m_States.SetBlendStateDisabledPreserveAlpha(GL);
        }
        else if (m_Output.Blend.Mode == GFSDK_SSAO_MULTIPLY_RGB)
        {
            m_States.SetBlendStateMultiplyPreserveAlpha(GL);
        }
        else
        {
            m_States.SetCustomBlendState(GL, m_Output.Blend.CustomState);
        }
    }

    GFSDK::SSAO::GL::InputDepthInfo m_InputDepth;
    GFSDK::SSAO::GL::InputNormalInfo m_InputNormal;
    GFSDK::SSAO::GL::GlobalConstantBuffer m_GlobalCB;
    GFSDK::SSAO::GL::PerPassConstantBuffers m_PerPassCBs;
    GFSDK::SSAO::GL::RenderTargets m_RTs;
    GFSDK::SSAO::GL::States m_States;
    GFSDK::SSAO::GL::OutputInfo m_Output;
    GFSDK::SSAO::GL::Shaders m_Shaders;
    GFSDK::SSAO::GL::VAO m_VAO;
    GFSDK::SSAO::GL::AppState m_AppState;
    GFSDK::SSAO::RenderOptions m_Options;
    GFSDK::SSAO::Viewports m_Viewports;
    GLuint m_FullResViewDepthTextureId;
    GFSDK_SSAO_CustomHeap m_NewDelete;
    GFSDK_SSAO_GLFunctions m_GL;
    GFSDK::SSAO::BuildVersion m_BuildVersion;
#if ENABLE_RENDER_TIMES
    GFSDK::SSAO::GL::TimestampQueries m_TimestampQueries;
#endif
};

} // namespace GL
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_GL
