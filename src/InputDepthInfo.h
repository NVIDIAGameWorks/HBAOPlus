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

        Status = Texture.Init(DepthData.pFullResDepthTextureSRV);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        Status = Viewport.Init(DepthData.Viewport, Texture);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        DepthTextureType = DepthData.DepthTextureType;
        MetersToViewSpaceUnits = Max(DepthData.MetersToViewSpaceUnits, 0.f);

        return GFSDK_SSAO_OK;
    }

    SSAO::D3D11::UserTextureSRV Texture;
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

        Status = Texture.Init(&DepthData.FullResDepthTextureSRV);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        Status = Viewport.Init(DepthData.Viewport, Texture);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        DepthTextureType = DepthData.DepthTextureType;
        MetersToViewSpaceUnits = Max(DepthData.MetersToViewSpaceUnits, 0.f);

        return GFSDK_SSAO_OK;
    }

    SSAO::D3D12::UserTextureSRV Texture;
};

} // namespace D3D12
#endif // SUPPORT_D3D12

//--------------------------------------------------------------------------------
#if SUPPORT_GL
namespace GL
{

//--------------------------------------------------------------------------------
struct InputDepthInfo : SSAO::InputDepthInfo
{
    InputDepthInfo()
        : SSAO::InputDepthInfo()
    {
    }

    GFSDK_SSAO_Status SetData(const GFSDK_SSAO_GLFunctions& GL, const GFSDK_SSAO_InputDepthData_GL& DepthData)
    {
        GFSDK_SSAO_Status Status;

        Status = ProjectionMatrixInfo.Init(DepthData.ProjectionMatrix, API_GL);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        Status = Texture.Init(GL, DepthData.FullResDepthTexture);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        Status = Viewport.InitFromTexture(DepthData.Viewport, Texture);
        if (Status != GFSDK_SSAO_OK)
        {
            return Status;
        }

        DepthTextureType = DepthData.DepthTextureType;
        MetersToViewSpaceUnits = Max(DepthData.MetersToViewSpaceUnits, 0.f);

        return GFSDK_SSAO_OK;
    }

    SSAO::GL::UserTexture Texture;
};

} // namespace GL
#endif // SUPPORT_GL

} // namespace SSAO
} // namespace GFSDK
