/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "ConstantBuffers.h"
#include "MatrixView.h"

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetBlurConstants(const GFSDK_SSAO_BlurParameters& Params, const GFSDK::SSAO::InputDepthInfo& InputDepth)
{
    float BaseSharpness = Max(Params.Sharpness, 0.f);
    BaseSharpness /= InputDepth.MetersToViewSpaceUnits;

    if (Params.SharpnessProfile.Enable)
    {
        m_Data.fBlurViewDepth0 = Max(Params.SharpnessProfile.ForegroundViewDepth, 0.f);
        m_Data.fBlurViewDepth1 = Max(Params.SharpnessProfile.BackgroundViewDepth, m_Data.fBlurViewDepth0 + EPSILON);
        m_Data.fBlurSharpness0 = BaseSharpness * Max(Params.SharpnessProfile.ForegroundSharpnessScale, 0.f);
        m_Data.fBlurSharpness1 = BaseSharpness;
    }
    else
    {
        m_Data.fBlurSharpness0 = BaseSharpness;
        m_Data.fBlurSharpness1 = BaseSharpness;
        m_Data.fBlurViewDepth0 = 0.f;
        m_Data.fBlurViewDepth1 = 1.f;
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetDepthThresholdConstants(const GFSDK_SSAO_DepthThreshold& Params)
{
    if (Params.Enable)
    {
        m_Data.fViewDepthThresholdNegInv = -1.f / Max(Params.MaxViewDepth, EPSILON);
        m_Data.fViewDepthThresholdSharpness = Max(Params.Sharpness, 0.f);
    }
    else
    {
        m_Data.fViewDepthThresholdNegInv = 0.f;
        m_Data.fViewDepthThresholdSharpness = 1.f;
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetAORadiusConstants(const GFSDK_SSAO_Parameters& Params, const GFSDK::SSAO::InputDepthInfo& InputDepth)
{
    const float RadiusInMeters = Max(Params.Radius, EPSILON);
    const float R = RadiusInMeters * InputDepth.MetersToViewSpaceUnits;
    m_Data.fR2 = R * R;
    m_Data.fNegInvR2 = -1.f / m_Data.fR2;

    const float TanHalfFovy = InputDepth.ProjectionMatrixInfo.GetTanHalfFovY();
    m_Data.fRadiusToScreen = R * 0.5f / TanHalfFovy * InputDepth.Viewport.Height;

    const float BackgroundViewDepth = Max(Params.BackgroundAO.BackgroundViewDepth, EPSILON);
    m_Data.fBackgroundAORadiusPixels = m_Data.fRadiusToScreen / BackgroundViewDepth;

    const float ForegroundViewDepth = Max(Params.ForegroundAO.ForegroundViewDepth, EPSILON);
    m_Data.fForegroundAORadiusPixels = m_Data.fRadiusToScreen / ForegroundViewDepth;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetAOParameters(const GFSDK_SSAO_Parameters& Params, const GFSDK::SSAO::InputDepthInfo& InputDepth)
{
    SetAORadiusConstants(Params, InputDepth);
    SetBlurConstants(Params.Blur, InputDepth);
    SetDepthThresholdConstants(Params.DepthThreshold);

    m_Data.fPowExponent = Clamp(Params.PowerExponent, 1.f, 8.f);
    m_Data.fNDotVBias = Clamp(Params.Bias, 0.0f, 0.5f);

    const float AOAmountScaleFactor = 1.f / (1.f - m_Data.fNDotVBias);
    m_Data.fSmallScaleAOAmount = Clamp(Params.SmallScaleAO, 0.f, 4.f) * AOAmountScaleFactor * 2.f;
    m_Data.fLargeScaleAOAmount = Clamp(Params.LargeScaleAO, 0.f, 4.f) * AOAmountScaleFactor;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetRenderMask(GFSDK_SSAO_RenderMask RenderMask)
{
    switch (RenderMask)
    {
    case GFSDK_SSAO_RENDER_DEBUG_NORMAL_X:
        m_Data.iDebugNormalComponent = 0;
        break;
    case GFSDK_SSAO_RENDER_DEBUG_NORMAL_Y:
        m_Data.iDebugNormalComponent = 1;
        break;
    case GFSDK_SSAO_RENDER_DEBUG_NORMAL_Z:
        m_Data.iDebugNormalComponent = 2;
        break;
    default:
        m_Data.iDebugNormalComponent = 3;
        break;
    }
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetDepthData(const GFSDK::SSAO::InputDepthInfo& InputDepth)
{
    SetDepthLinearizationConstants(InputDepth);
    SetViewportConstants(InputDepth);
    SetProjectionConstants(InputDepth);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetResolutionConstants(const GFSDK::SSAO::Viewports &Viewports)
{
    m_Data.f2InvFullResolution.X = 1.f / Viewports.FullRes.Width;
    m_Data.f2InvFullResolution.Y = 1.f / Viewports.FullRes.Height;
    m_Data.f2InvQuarterResolution.X = 1.f / Viewports.QuarterRes.Width;
    m_Data.f2InvQuarterResolution.Y = 1.f / Viewports.QuarterRes.Height;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetProjectionConstants(const GFSDK::SSAO::InputDepthInfo& InputDepth)
{
    const float InvFocalLenX  = InputDepth.ProjectionMatrixInfo.GetTanHalfFovX();
    const float InvFocalLenY  = InputDepth.ProjectionMatrixInfo.GetTanHalfFovY();
    m_Data.f2UVToViewA.X =  2.f * InvFocalLenX;
    m_Data.f2UVToViewA.Y = -2.f * InvFocalLenY;
    m_Data.f2UVToViewB.X = -1.f * InvFocalLenX;
    m_Data.f2UVToViewB.Y =  1.f * InvFocalLenY;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetViewportConstants(const GFSDK::SSAO::InputDepthInfo& InputDepth)
{
    // In Shaders/Src/LinearizeDepth_Common.hlsl:
    // float NormalizedDepth = saturate(g_fInverseDepthRangeA * HardwareDepth + g_fInverseDepthRangeB);

    if (InputDepth.DepthTextureType == GFSDK_SSAO_HARDWARE_DEPTHS_SUB_RANGE)
    {
        // Inverse viewport depth range from [MinZ,MaxZ] to [0,1]
        // Z = (HardwareZ - MinZ) / (MaxZ - MinZ)
        const float MinZ  = InputDepth.Viewport.MinDepth;
        const float MaxZ  = InputDepth.Viewport.MaxDepth;
        m_Data.fInverseDepthRangeA = 1.f / (MaxZ - MinZ);
        m_Data.fInverseDepthRangeB = -MinZ * m_Data.fInverseDepthRangeA;
    }
    else
    {
        m_Data.fInverseDepthRangeA = 1.f;
        m_Data.fInverseDepthRangeB = 0.f;
    }

    m_Data.f2InputViewportTopLeft.X = InputDepth.Viewport.TopLeftX;
    m_Data.f2InputViewportTopLeft.Y = InputDepth.Viewport.TopLeftY;
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetDepthLinearizationConstants(const GFSDK::SSAO::InputDepthInfo& InputDepth)
{
    // In Shaders/Src/LinearizeDepth_Common.hlsl:
    // float ViewDepth = 1.0 / (NormalizedDepth * g_fLinearizeDepthA + g_fLinearizeDepthB);

    // Inverse projection from [0,1] to [ZNear,ZFar]
    // W = 1 / [(1/ZFar - 1/ZNear) * Z + 1/ZNear]
    const float InverseZNear = InputDepth.ProjectionMatrixInfo.GetInverseZNear();
    const float InverseZFar  = InputDepth.ProjectionMatrixInfo.GetInverseZFar();
    m_Data.fLinearizeDepthA = InverseZFar - InverseZNear;
    m_Data.fLinearizeDepthB = InverseZNear;

    ASSERT((0.f * m_Data.fLinearizeDepthA + m_Data.fLinearizeDepthB) != 0.f);
    ASSERT((1.f * m_Data.fLinearizeDepthA + m_Data.fLinearizeDepthB) != 0.f);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::GlobalConstants::SetNormalData(const GFSDK_SSAO_InputNormalData& NormalData)
{
    MatrixView WorldToView(NormalData.WorldToViewMatrix);

    for (UINT Row = 0; Row < 3; ++Row)
    {
        for (UINT Col = 0; Col < 3; ++Col)
        {
            m_Data.f44NormalMatrix.Data[Row * 4 + Col] = WorldToView(Row, Col);
        }
    }

    m_Data.fNormalDecodeScale = NormalData.DecodeScale;
    m_Data.fNormalDecodeBias = NormalData.DecodeBias;
}
