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
#include "ProjectionMatrixInfo.h"
#include "UserTexture.h"
#include "InputViewport.h"
#include "TextureUtil.h"

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
struct InputDepthInfo
{
    InputDepthInfo()
      : DepthTextureType(GFSDK_SSAO_HARDWARE_DEPTHS)
      , MetersToViewSpaceUnits(0.f)
    {
    }

    GFSDK_SSAO_DepthTextureType DepthTextureType;
    FLOAT MetersToViewSpaceUnits;
    SSAO::ProjectionMatrixInfo ProjectionMatrixInfo;
    SSAO::InputViewport Viewport;
};

//--------------------------------------------------------------------------------
#if SUPPORT_D3D11
namespace D3D11
{

struct InputDepthInfo : SSAO::InputDepthInfo
{
    InputDepthInfo()
      : SSAO::InputDepthInfo()
    {
    }

    GFSDK_SSAO_Status SetData(const GFSDK_SSAO_InputDepthData_D3D11& DepthData)
    {
        GFSDK_SSAO_Status Status;

        Status = ProjectionMatrixInfo.Init(DepthData.ProjectionMatrix, API_D3D11);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        Status = Texture0.Init(DepthData.pFullResDepthTextureSRV);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        if (DepthData.pFullResDepthTexture2ndLayerSRV) // the second texture can be nullptr if DualLayer is not enabled
        {
            Status = Texture1.Init(DepthData.pFullResDepthTexture2ndLayerSRV);
            if (Status != GFSDK_SSAO_OK)
            {
                return Status;
            }
            if (Texture1.Width  != Texture0.Width ||
                Texture1.Height != Texture0.Height)
            {
                return GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_RESOLUTION;
            }
            if (Texture1.SampleCount != Texture0.SampleCount)
            {
                return GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_RESOLUTION;
            }
        }

        Status = Viewport.Init(DepthData.Viewport, Texture0);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        DepthTextureType = DepthData.DepthTextureType;
        MetersToViewSpaceUnits = Max(DepthData.MetersToViewSpaceUnits, 0.f);

        return GFSDK_SSAO_OK;
    }

    SSAO::D3D11::UserTextureSRV Texture0;
    SSAO::D3D11::UserTextureSRV Texture1;
};

} // namespace D3D11
#endif // SUPPORT_D3D11

//--------------------------------------------------------------------------------
#if SUPPORT_D3D12
namespace D3D12
{

struct InputDepthInfo : SSAO::InputDepthInfo
{
    InputDepthInfo()
        : SSAO::InputDepthInfo()
    {
    }

    GFSDK_SSAO_Status SetData(const GFSDK_SSAO_InputDepthData_D3D12& DepthData)
    {
        GFSDK_SSAO_Status Status;

        Status = ProjectionMatrixInfo.Init(DepthData.ProjectionMatrix, API_D3D11);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        Status = Texture0.Init(&DepthData.FullResDepthTextureSRV); 
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        if (DepthData.FullResDepthTexture2ndLayerSRV.pResource)
        {
            Status = Texture1.Init(&DepthData.FullResDepthTexture2ndLayerSRV);
            if (Status != GFSDK_SSAO_OK)
            {
                return Status;
            }
            if (Texture1.Width  != Texture0.Width ||
                Texture1.Height != Texture0.Height)
            {
                return GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_RESOLUTION;
            }
            if (Texture1.SampleCount != Texture0.SampleCount)
            {
                return GFSDK_SSAO_INVALID_SECOND_DEPTH_TEXTURE_RESOLUTION;
            }
        }

        Status = Viewport.Init(DepthData.Viewport, Texture0);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        DepthTextureType = DepthData.DepthTextureType;
        MetersToViewSpaceUnits = Max(DepthData.MetersToViewSpaceUnits, 0.f);

        return GFSDK_SSAO_OK;
    }

    SSAO::D3D12::UserTextureSRV Texture0;
    SSAO::D3D12::UserTextureSRV Texture1;
};

} // namespace D3D12
#endif // SUPPORT_D3D12

} // namespace SSAO
} // namespace GFSDK
