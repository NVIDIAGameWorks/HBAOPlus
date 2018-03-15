/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "HBAOSample.h"
#include "DeviceManager11.h"
#include <D3D11.h>
#include <wrl.h>
#include "SceneRenderer.h"

#include <GFSDK_SSAO.h>

using namespace Microsoft::WRL;
class HBAOSampleD3D11 : public HBAOSample, public IVisualController
{
public:
    void Initialize() override;
    void Run() override;

    void Render(RenderTargetView RTV) override;
    void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) override;

private:
    bool InitializeHBAOPlus();
    bool InitializeRenderTargets();
    LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    DeviceManager                       mDeviceManager;
    ComPtr<ID3D11Texture2D>             mDepthStencil[2];
    ComPtr<ID3D11Resource>              mNormalsGBuffer;
    ComPtr<ID3D11DepthStencilView>      mDepthStencilView[2];
    ComPtr<ID3D11ShaderResourceView>    mDepthSRV[2];
    
    ID3D11Device*                       mDevice = nullptr;
    ID3D11DeviceContext*                mDeviceContext = nullptr;

    ID3D12CommandQueue*                 mCommandQueue;

    SceneRenderer                       mSceneRenderer;
    
    GFSDK_SSAO_InputData_D3D11          mSSAOInputData;
    GFSDK_SSAO_Context_D3D11*           mSSAOContext;
};