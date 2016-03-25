/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "AppState_DX11.h"

#if SUPPORT_D3D11

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::AppState::Save(ID3D11DeviceContext* pDeviceContext)
{
    pDeviceContext->IAGetPrimitiveTopology(&Topology);
    pDeviceContext->IAGetInputLayout(&pInputLayout);

    UINT NumViewports = 1;
    pDeviceContext->RSGetViewports(&NumViewports, &Viewport);
    pDeviceContext->RSGetState(&pRasterizerState);

    pDeviceContext->VSGetShader(&pVS, NULL, NULL);
    pDeviceContext->HSGetShader(&pHS, NULL, NULL);
    pDeviceContext->DSGetShader(&pDS, NULL, NULL);
    pDeviceContext->GSGetShader(&pGS, NULL, NULL);
    pDeviceContext->PSGetShader(&pPS, NULL, NULL);
    pDeviceContext->CSGetShader(&pCS, NULL, NULL);

    pDeviceContext->PSGetShaderResources(0, SIZEOF_ARRAY(pPSShaderResourceViews), pPSShaderResourceViews);
    pDeviceContext->PSGetSamplers(0, SIZEOF_ARRAY(pPSSamplers), pPSSamplers);
    pDeviceContext->GSGetConstantBuffers(0, SIZEOF_ARRAY(pGSConstantBuffers), pGSConstantBuffers);
    pDeviceContext->PSGetConstantBuffers(0, SIZEOF_ARRAY(pPSConstantBuffers), pPSConstantBuffers);

    pDeviceContext->OMGetDepthStencilState(&pDepthStencilState, &StencilRef);
    pDeviceContext->OMGetBlendState(&pBlendState, BlendFactor, &SampleMask);
    pDeviceContext->OMGetRenderTargets(SIZEOF_ARRAY(pRenderTargetViews), pRenderTargetViews, &pDepthStencilView);
}

//--------------------------------------------------------------------------------
void GFSDK::SSAO::D3D11::AppState::Restore(ID3D11DeviceContext* pDeviceContext)
{
    pDeviceContext->IASetPrimitiveTopology(Topology);
    pDeviceContext->IASetInputLayout(pInputLayout);

    pDeviceContext->RSSetViewports(1, &Viewport);
    pDeviceContext->RSSetState(pRasterizerState);

    pDeviceContext->VSSetShader(pVS, NULL, 0);
    pDeviceContext->HSSetShader(pHS, NULL, 0);
    pDeviceContext->DSSetShader(pDS, NULL, 0);
    pDeviceContext->GSSetShader(pGS, NULL, 0);
    pDeviceContext->PSSetShader(pPS, NULL, 0);
    pDeviceContext->CSSetShader(pCS, NULL, 0);

    pDeviceContext->PSSetShaderResources(0, SIZEOF_ARRAY(pPSShaderResourceViews), pPSShaderResourceViews);
    pDeviceContext->PSSetSamplers(0, SIZEOF_ARRAY(pPSSamplers), pPSSamplers);
    pDeviceContext->GSSetConstantBuffers(0, SIZEOF_ARRAY(pGSConstantBuffers), pGSConstantBuffers);
    pDeviceContext->PSSetConstantBuffers(0, SIZEOF_ARRAY(pPSConstantBuffers), pPSConstantBuffers);

    pDeviceContext->OMSetDepthStencilState(pDepthStencilState, StencilRef);
    pDeviceContext->OMSetBlendState(pBlendState, BlendFactor, SampleMask);
    pDeviceContext->OMSetRenderTargets(SIZEOF_ARRAY(pRenderTargetViews), pRenderTargetViews, pDepthStencilView);

    SAFE_RELEASE(pInputLayout);
    SAFE_RELEASE(pRasterizerState);
    SAFE_RELEASE(pVS);
    SAFE_RELEASE(pHS);
    SAFE_RELEASE(pDS);
    SAFE_RELEASE(pGS);
    SAFE_RELEASE(pPS);
    SAFE_RELEASE(pCS);
    SAFE_RELEASE(pDepthStencilState);
    SAFE_RELEASE(pBlendState);
    SAFE_RELEASE(pDepthStencilView);
    SAFE_RELEASE_ARRAY(pPSShaderResourceViews);
    SAFE_RELEASE_ARRAY(pPSSamplers);
    SAFE_RELEASE_ARRAY(pGSConstantBuffers);
    SAFE_RELEASE_ARRAY(pPSConstantBuffers);
    SAFE_RELEASE_ARRAY(pRenderTargetViews);
}

void GFSDK::SSAO::D3D11::AppState::UnbindSRVs(ID3D11DeviceContext* pDeviceContext)
{
    ID3D11ShaderResourceView* pNullSRVs[NumPSShaderResourceViews] = { NULL };
    pDeviceContext->PSSetShaderResources(0, ARRAYSIZE(pNullSRVs), pNullSRVs);
}

#endif // SUPPORT_D3D11
