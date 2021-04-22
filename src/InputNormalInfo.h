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

} // namespace SSAO
} // namespace GFSDK
