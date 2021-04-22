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
