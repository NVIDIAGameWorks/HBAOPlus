/* 
* Copyright (c) 2008-2017, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#pragma once

#include <D3D11.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)         { if (p) { (p)->Release(); (p)=NULL; } }
#endif

struct SceneRTs
{
    struct Desc
    {
        UINT OutputWidth;
        UINT OutputHeight;
        UINT BorderPixels;
        UINT SampleCount;
    };

    SceneRTs(ID3D11Device* pDevice, Desc &desc)
        : Width(desc.OutputWidth + 2 * desc.BorderPixels)
        , Height(desc.OutputHeight + 2 * desc.BorderPixels)
        , OutputWidth(desc.OutputWidth)
        , OutputHeight(desc.OutputHeight)
        , BorderPixels(desc.BorderPixels)
        , SampleCount(desc.SampleCount)
    {
        CreateGBufferTextures(pDevice);
        CreateOutputTexture(pDevice);
        CreateDepthTexture(pDevice);
    }

    void CreateGBufferTextures(ID3D11Device* pDevice)
    {
        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.Width              = Width;
        texDesc.Height             = Height;
        texDesc.ArraySize          = 1;
        texDesc.MiscFlags          = 0;
        texDesc.MipLevels          = 1;
        texDesc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.Usage              = D3D11_USAGE_DEFAULT;
        texDesc.CPUAccessFlags     = NULL;

        // Allocate MSAA color buffer
        texDesc.SampleDesc.Count   = SampleCount;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

        pDevice->CreateTexture2D(&texDesc, NULL, &ColorTexture);
        pDevice->CreateRenderTargetView(ColorTexture, NULL, &ColorRTV);
        pDevice->CreateShaderResourceView(ColorTexture, NULL, &ColorSRV);

        // Allocate MSAA normal buffer
        texDesc.Format             = DXGI_FORMAT_R11G11B10_FLOAT;

        pDevice->CreateTexture2D(&texDesc, NULL, &NormalTexture);
        pDevice->CreateRenderTargetView(NormalTexture, NULL, &NormalRTV);
        pDevice->CreateShaderResourceView(NormalTexture, NULL, &NormalSRV);

        // Allocate non-MSAA color buffer
        texDesc.SampleDesc.Count   = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

        pDevice->CreateTexture2D(&texDesc, NULL, &ResolvedColorTexture);
        pDevice->CreateRenderTargetView(ColorTexture, NULL, &ResolvedColorRTV);
        pDevice->CreateShaderResourceView(ColorTexture, NULL, &ResolvedColorSRV);
    }

    void CreateOutputTexture(ID3D11Device* pDevice)
    {
        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.Width              = OutputWidth;
        texDesc.Height             = OutputHeight;
        texDesc.ArraySize          = 1;
        texDesc.MiscFlags          = 0;
        texDesc.MipLevels          = 1;
        texDesc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.Usage              = D3D11_USAGE_DEFAULT;
        texDesc.CPUAccessFlags     = NULL;
        texDesc.SampleDesc.Count   = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

        pDevice->CreateTexture2D(&texDesc, NULL, &OutputTexture);
        pDevice->CreateRenderTargetView(OutputTexture, NULL, &OutputRTV);
        pDevice->CreateShaderResourceView(OutputTexture, NULL, &OutputSRV);
    }

    void CreateDepthTexture(ID3D11Device* pDevice)
    {
        // Create a hardware-depth texture that can be fetched from a shader.
        // To do so, use a TYPELESS format and set the D3D11_BIND_SHADER_RESOURCE flag.
        // D3D10.0 did not allow creating such a depth texture with SampleCount > 1.
        // This is now possible since D3D10.1.
        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.ArraySize          = 1;
        texDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags     = NULL;
        texDesc.Width              = Width;
        texDesc.Height             = Height;
        texDesc.MipLevels          = 1;
        texDesc.MiscFlags          = NULL;
        texDesc.SampleDesc.Count   = SampleCount;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage              = D3D11_USAGE_DEFAULT;
        texDesc.Format             = DXGI_FORMAT_R24G8_TYPELESS;
        pDevice->CreateTexture2D(&texDesc, NULL, &DepthStencilTexture);

        // Create a depth-stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        if (SampleCount > 1)
        {
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        }
        dsvDesc.Texture2D.MipSlice = 0;
        dsvDesc.Flags = 0; // new in D3D11
        pDevice->CreateDepthStencilView(DepthStencilTexture, &dsvDesc, &DepthStencilDSV);

        // Create a shader resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        if (SampleCount > 1)
        {
            srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
        }
        pDevice->CreateShaderResourceView(DepthStencilTexture, &srvDesc, &DepthStencilSRV);
    }

    ~SceneRTs()
    {
        SAFE_RELEASE(ColorTexture);
        SAFE_RELEASE(ColorRTV);
        SAFE_RELEASE(ColorSRV);

        SAFE_RELEASE(ResolvedColorTexture);
        SAFE_RELEASE(ResolvedColorRTV);
        SAFE_RELEASE(ResolvedColorSRV);

        SAFE_RELEASE(NormalTexture);
        SAFE_RELEASE(NormalRTV);
        SAFE_RELEASE(NormalSRV);

        SAFE_RELEASE(OutputTexture);
        SAFE_RELEASE(OutputRTV);
        SAFE_RELEASE(OutputSRV);

        SAFE_RELEASE(DepthStencilTexture);
        SAFE_RELEASE(DepthStencilSRV);
        SAFE_RELEASE(DepthStencilDSV);
    }

    UINT Width;
    UINT Height;
    UINT OutputWidth;
    UINT OutputHeight;
    UINT BorderPixels;
    UINT SampleCount;
    ID3D11Texture2D*              ColorTexture;
    ID3D11RenderTargetView*       ColorRTV;
    ID3D11ShaderResourceView*     ColorSRV;
    ID3D11Texture2D*              ResolvedColorTexture;
    ID3D11RenderTargetView*       ResolvedColorRTV;
    ID3D11ShaderResourceView*     ResolvedColorSRV;
    ID3D11Texture2D*              NormalTexture;
    ID3D11RenderTargetView*       NormalRTV;
    ID3D11ShaderResourceView*     NormalSRV;
    ID3D11Texture2D*              OutputTexture;
    ID3D11RenderTargetView*       OutputRTV;
    ID3D11ShaderResourceView*     OutputSRV;
    ID3D11Texture2D*              DepthStencilTexture;
    ID3D11ShaderResourceView*     DepthStencilSRV;
    ID3D11DepthStencilView*       DepthStencilDSV;
};
