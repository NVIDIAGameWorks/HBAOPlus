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

#if !SUPPORT_D3D11
struct D3D11_VIEWPORT
{
    FLOAT TopLeftX;
    FLOAT TopLeftY;
    FLOAT Width;
    FLOAT Height;
    FLOAT MinDepth;
    FLOAT MaxDepth;
};
#endif

#if SUPPORT_D3D12
struct InputViewport : public D3D12_VIEWPORT
#else
struct InputViewport : public D3D11_VIEWPORT
#endif
{
    InputViewport()
    {
        ZERO_STRUCT(*this);
    }

    static bool HasValidDimensions(const GFSDK_SSAO_InputViewport& V)
    {
        return (V.Width != 0 && V.Height != 0);
    }

    static bool HasValidDepthRange(const GFSDK_SSAO_InputViewport& V)
    {
        // According to the DX11 spec:
        // Viewport MinDepth and MaxDepth must both be in the range [0.0f...1.0f], and MinDepth must be less-than or equal-to MaxDepth.
        // Viewport parameters are validated in the runtime such that values outside these ranges will never be passed to the DDI.
        return (V.MinDepth >= 0.f && V.MinDepth <= 1.f &&
                V.MaxDepth >= 0.f && V.MaxDepth <= 1.f &&
                V.MinDepth <= V.MaxDepth);
    }

    static bool CoversFullTexture(const GFSDK_SSAO_InputViewport& V, const SSAO::UserTextureDesc& Texture)
    {
        return (V.TopLeftX == 0.f &&
                V.TopLeftY == 0.f &&
                V.Width  == (FLOAT)Texture.Width &&
                V.Height == (FLOAT)Texture.Height);
    }

    GFSDK_SSAO_Status Init(const GFSDK_SSAO_InputViewport& Viewport, const SSAO::UserTextureDesc& Texture)
    {
        if (!Viewport.Enable)
        {
            return InitFromTexture(Texture);
        }

        return InitFromViewport(Viewport, Texture);
    }

    bool RectCoversFullInputTexture;

private:
    GFSDK_SSAO_Status InitFromTexture(const SSAO::UserTextureDesc& Texture)
    {
        TopLeftX   = 0.f;
        TopLeftY   = 0.f;
        Width      = FLOAT(Texture.Width);
        Height     = FLOAT(Texture.Height);
        MinDepth   = 0.f;
        MaxDepth   = 1.f;
        RectCoversFullInputTexture = true;

        return GFSDK_SSAO_OK;
    }

    GFSDK_SSAO_Status InitFromViewport(const GFSDK_SSAO_InputViewport& Viewport, const SSAO::UserTextureDesc& Texture)
    {
        if (!HasValidDimensions(Viewport))
        {
            return GFSDK_SSAO_INVALID_VIEWPORT_DIMENSIONS;
        }

        if (!HasValidDepthRange(Viewport))
        {
            return GFSDK_SSAO_INVALID_VIEWPORT_DEPTH_RANGE;
        }

        TopLeftX = (FLOAT)Viewport.TopLeftX;
        TopLeftY = (FLOAT)Viewport.TopLeftY;
        Width    = (FLOAT)Viewport.Width;
        Height   = (FLOAT)Viewport.Height;
        MinDepth = Viewport.MinDepth;
        MaxDepth = Viewport.MaxDepth;

        Width  = Min(Width,  FLOAT(Texture.Width)  - TopLeftX);
        Height = Min(Height, FLOAT(Texture.Height) - TopLeftY);

        RectCoversFullInputTexture = CoversFullTexture(Viewport, Texture);

        return GFSDK_SSAO_OK;
    }
};

} // namespace SSAO
} // namespace GFSDK
