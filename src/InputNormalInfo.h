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
#include "MatrixView.h"
#include "TextureUtil.h"
#include "UserTexture.h"

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
class WorldToViewMatrix
{
public:
    static bool IsValid(const GFSDK_SSAO_Matrix& WorldToViewMatrix)
    {
        MatrixView m(WorldToViewMatrix);

        // Necessary condition for the matrix to be a valid LookAt matrix
        // Note: the matrix may contain a uniform scaling, so we do not check m(3,3)
        return (m(0,3) == 0.f &&
                m(1,3) == 0.f &&
                m(2,3) == 0.f);
    }
};

//--------------------------------------------------------------------------------
#if SUPPORT_D3D11
namespace D3D11
{

struct InputNormalInfo
{
    GFSDK_SSAO_Status SetData(const GFSDK_SSAO_InputNormalData_D3D11& NormalData)
    {
        if (!NormalData.pFullResNormalTextureSRV)
        {
            return GFSDK_SSAO_NULL_ARGUMENT;
        }

        if (!GFSDK::SSAO::WorldToViewMatrix::IsValid(NormalData.WorldToViewMatrix))
        {
            return GFSDK_SSAO_INVALID_WORLD_TO_VIEW_MATRIX;
        }

        Texture.Init(NormalData.pFullResNormalTextureSRV);

        return GFSDK_SSAO_OK;
    }

    SSAO::D3D11::UserTextureSRV Texture;
};

} // namespace D3D11
#endif

//--------------------------------------------------------------------------------
#if SUPPORT_D3D12
namespace D3D12
{

struct InputNormalInfo
{
    GFSDK_SSAO_Status SetData(const GFSDK_SSAO_InputNormalData_D3D12& NormalData)
    {
        if (!NormalData.FullResNormalTextureSRV.pResource)
        {
            return GFSDK_SSAO_NULL_ARGUMENT;
        }

        if (!GFSDK::SSAO::WorldToViewMatrix::IsValid(NormalData.WorldToViewMatrix))
        {
            return GFSDK_SSAO_INVALID_WORLD_TO_VIEW_MATRIX;
        }

        Texture.Init(&NormalData.FullResNormalTextureSRV);

        return GFSDK_SSAO_OK;
    }

    SSAO::D3D12::UserTextureSRV Texture;
};

} // namespace D3D12
#endif

//--------------------------------------------------------------------------------
#if SUPPORT_GL
namespace GL
{

struct InputNormalInfo
{
    GFSDK_SSAO_Status SetData(const GFSDK_SSAO_GLFunctions& GL, const GFSDK_SSAO_InputNormalData_GL& NormalData)
    {
        if (!GFSDK::SSAO::GL::TextureUtil::HasValidTextureTarget(NormalData.FullResNormalTexture))
        {
            return GFSDK_SSAO_GL_INVALID_TEXTURE_TARGET;
        }

        if (!GFSDK::SSAO::WorldToViewMatrix::IsValid(NormalData.WorldToViewMatrix))
        {
            return GFSDK_SSAO_INVALID_WORLD_TO_VIEW_MATRIX;
        }

        Texture.Init(GL, NormalData.FullResNormalTexture);

        return GFSDK_SSAO_OK;
    }

    SSAO::GL::UserTexture Texture;
};

} // namespace GL
#endif // SUPPORT_GL

} // namespace SSAO
} // namespace GFSDK
