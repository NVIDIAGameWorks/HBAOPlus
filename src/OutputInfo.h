/* 
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved. 
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
struct OutputInfo
{
    GFSDK::SSAO::D3D11::UserTextureRTV RenderTarget;
    GFSDK::SSAO::D3D11::UserTextureDSV DepthStencilBuffer;

    GFSDK_SSAO_BlendState_D3D11 Blend;
    GFSDK_SSAO_DepthStencilState_D3D11 DepthStencil;

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
        return InitSinglePassBlend(Output.Blend);
    }

private:
    GFSDK_SSAO_Status InitSinglePassBlend(const GFSDK_SSAO_BlendState_D3D11& BlendState)
    {
        Blend = BlendState;

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

} // namespace SSAO
} // namespace GFSDK
