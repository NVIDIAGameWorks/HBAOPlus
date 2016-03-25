/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#pragma once
#include "Common.h"
#include "UserTexture.h"

namespace GFSDK
{
namespace SSAO
{

#if SUPPORT_D3D11
namespace D3D11
{

//--------------------------------------------------------------------------------
enum BlendPassEnumType
{
    BLEND_PASS_0,
    BLEND_PASS_1,
    BLEND_PASS_COUNT,
};

//--------------------------------------------------------------------------------
struct OutputInfo
{
    GFSDK::SSAO::D3D11::UserTextureRTV RenderTarget;
    GFSDK::SSAO::D3D11::UserTextureDSV DepthStencilBuffer;

    UINT BlendPassCount;
    GFSDK_SSAO_BlendState_D3D11 Blend[BLEND_PASS_COUNT];
    GFSDK_SSAO_DepthStencilState_D3D11 DepthStencil[BLEND_PASS_COUNT];

    OutputInfo()
    {
        ZERO_STRUCT(*this);
    }

    GFSDK_SSAO_Status Init(const GFSDK_SSAO_Output_D3D11& Output)
    {
        GFSDK_SSAO_Status Status;

        Status = RenderTarget.Init(Output.pRenderTargetView);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        Status = InitBlendState(Output);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        return GFSDK_SSAO_OK;
    }

    GFSDK_SSAO_Status InitBlendState(const GFSDK_SSAO_Output_D3D11& Output)
    {
        if (Output.TwoPassBlend.Enable)
        {
            return InitTwoPassBlend(Output.TwoPassBlend);
        }

        return InitSinglePassBlend(Output.Blend);
    }

private:
    GFSDK_SSAO_Status InitSinglePassBlend(const GFSDK_SSAO_BlendState_D3D11& BlendState)
    {
        BlendPassCount = 1;
        Blend[BLEND_PASS_0] = BlendState;

        return GFSDK_SSAO_OK;
    }

    GFSDK_SSAO_Status InitTwoPassBlend(const GFSDK_SSAO_TwoPassBlend_D3D11& TwoPassBlend)
    {
        GFSDK_SSAO_Status Status = DepthStencilBuffer.Init(TwoPassBlend.pDepthStencilView);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        BlendPassCount = 2;
        Blend[BLEND_PASS_0] = TwoPassBlend.FirstPass.Blend;
        Blend[BLEND_PASS_1] = TwoPassBlend.SecondPass.Blend;
        DepthStencil[BLEND_PASS_0] = TwoPassBlend.FirstPass.DepthStencil;
        DepthStencil[BLEND_PASS_1] = TwoPassBlend.SecondPass.DepthStencil;

        return GFSDK_SSAO_OK;
    }
};

} // namespace D3D11
#endif // SUPPORT_D3D11

#if SUPPORT_D3D12
namespace D3D12
{

//--------------------------------------------------------------------------------
struct OutputInfo
{
    OutputInfo()
    {
    }

    GFSDK_SSAO_Status Init(const GFSDK_SSAO_Output_D3D12& Output)
    {
        GFSDK_SSAO_Status Status;

        Status = RenderTarget.Init(Output.pRenderTargetView);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        Blend = Output.Blend;

        return GFSDK_SSAO_OK;
    }

    GFSDK::SSAO::D3D12::UserTextureRTV RenderTarget;
    GFSDK_SSAO_BlendState_D3D12 Blend;
};

} // namespace D3D12
#endif // SUPPORT_D3D12

//--------------------------------------------------------------------------------
#if SUPPORT_GL
namespace GL
{

struct OutputInfo
{
    OutputInfo()
        : FboId(0)
        , SampleCount(0)
    {
    }

    GFSDK_SSAO_Status Init(const GFSDK_SSAO_Output_GL& Output)
    {
        FboId = Output.OutputFBO;
        SampleCount = 1;
        Blend = Output.Blend;

        return GFSDK_SSAO_OK;
    }

    GLuint FboId;
    UINT SampleCount;
    GFSDK_SSAO_BlendState_GL Blend;
};

} // namespace GL
#endif // SUPPORT_GL

} // namespace SSAO
} // namespace GFSDK
