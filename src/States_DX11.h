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

#if SUPPORT_D3D11

#pragma once
#include "Common.h"

namespace GFSDK
{
namespace SSAO
{
namespace D3D11
{

//--------------------------------------------------------------------------------
class States
{
public:
    States()
        : m_pBlendState_Disabled(NULL)
        , m_pBlendState_Multiply_PreserveAlpha(NULL)
        , m_pBlendState_Disabled_PreserveAlpha(NULL)
        , m_pDepthStencilState_Disabled(NULL)
        , m_pRasterizerState_Fullscreen_NoScissor(NULL)
        , m_pSamplerState_PointClamp(NULL)
        , m_pSamplerState_PointBorder(NULL)
        , m_pSamplerState_LinearClamp(NULL)
#if ENABLE_DEBUG_MODES
        , m_pSamplerState_PointWrap(NULL)
#endif
      {
      }

    void Create(ID3D11Device* pD3DDevice);
    void Release();

    ID3D11BlendState* GetBlendStateMultiplyPreserveAlpha()
    {
        return m_pBlendState_Multiply_PreserveAlpha;
    }
    ID3D11BlendState* GetBlendStateDisabledPreserveAlpha()
    {
        return m_pBlendState_Disabled_PreserveAlpha;
    }
    ID3D11BlendState* GetBlendStateDisabled()
    {
        return m_pBlendState_Disabled;
    }
    ID3D11DepthStencilState* GetDepthStencilStateDisabled()
    {
        return m_pDepthStencilState_Disabled;
    }
    ID3D11RasterizerState* GetRasterizerStateFullscreenNoScissor()
    {
        return m_pRasterizerState_Fullscreen_NoScissor;
    }
    ID3D11SamplerState*& GetSamplerStatePointClamp()
    {
        return m_pSamplerState_PointClamp;
    }
    ID3D11SamplerState*& GetSamplerStatePointBorder()
    {
        return m_pSamplerState_PointBorder;
    }
    ID3D11SamplerState*& GetSamplerStateLinearClamp()
    {
        return m_pSamplerState_LinearClamp;
    }
#if ENABLE_DEBUG_MODES
    ID3D11SamplerState*& GetSamplerStatePointWrap()
    {
        return m_pSamplerState_PointWrap;
    }
#endif

private:
    void CreateBlendStates(ID3D11Device* pD3DDevice);
    void CreateDepthStencilStates(ID3D11Device* pD3DDevice);
    void CreateRasterizerStates(ID3D11Device* pD3DDevice);
    void CreateSamplerStates(ID3D11Device* pD3DDevice);

    ID3D11BlendState* m_pBlendState_Disabled;
    ID3D11BlendState* m_pBlendState_Multiply_PreserveAlpha;
    ID3D11BlendState* m_pBlendState_Disabled_PreserveAlpha;
    ID3D11DepthStencilState* m_pDepthStencilState_Disabled;
    ID3D11RasterizerState* m_pRasterizerState_Fullscreen_NoScissor;
    ID3D11SamplerState* m_pSamplerState_PointClamp;
    ID3D11SamplerState* m_pSamplerState_PointBorder;
    ID3D11SamplerState* m_pSamplerState_LinearClamp;
#if ENABLE_DEBUG_MODES
    ID3D11SamplerState* m_pSamplerState_PointWrap;
#endif
};

} // namespace D3D11
} // namespace SSAO
} // namespace GFSDK

#endif // SUPPORT_D3D11
