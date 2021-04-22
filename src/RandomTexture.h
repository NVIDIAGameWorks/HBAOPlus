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

namespace GFSDK
{
namespace SSAO
{

//--------------------------------------------------------------------------------
class RandomTexture
{
public:
    RandomTexture();

    float4 GetJitter(UINT SliceId)
    {
        ASSERT(SliceId < SIZEOF_ARRAY(m_Jitters));
        return m_Jitters[SliceId % SIZEOF_ARRAY(m_Jitters)];
    }

    float GetRandomNumber(UINT Index);

protected:
    static const UINT MAX_MSAA_SAMPLE_COUNT = 8;
    float4 m_Jitters[RANDOM_TEXTURE_WIDTH * RANDOM_TEXTURE_WIDTH * MAX_MSAA_SAMPLE_COUNT];
};

#if SUPPORT_D3D11
namespace D3D11
{

//--------------------------------------------------------------------------------
class RandomTexture : public SSAO::RandomTexture
{
#if ENABLE_DEBUG_MODES
public:
    RandomTexture()
    : SSAO::RandomTexture()
    , pTexture(NULL)
    , pSRV(NULL)
    {
    }

    void Create(ID3D11Device* pD3DDevice);
    void Release();
    ID3D11Texture2D* pTexture;
    ID3D11ShaderResourceView* pSRV;
#endif
};

} // namespace D3D11
#endif

#if SUPPORT_D3D12
namespace D3D12
{

//--------------------------------------------------------------------------------
class RandomTexture : public SSAO::RandomTexture
{
#if ENABLE_DEBUG_MODES
public:
    RandomTexture()
        : SSAO::RandomTexture()
        , pTexture(NULL)
    {
    }

    void Create(ID3D12Device* pD3DDevice);
    void Release();
    ID3D12Resource* pTexture; //ID3D12Texture2D* pTexture;
    GFSDK_SSAO_ShaderResourceView_D3D12 SRV; // ID3D12ShaderResourceView* pSRV;
#endif
};

} // namespace D3D12
#endif

} // namespace SSAO
} // namespace GFSDK
