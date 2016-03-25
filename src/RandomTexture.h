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
