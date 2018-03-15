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
#include <Windows.h>
#include <DXGI1_4.h>
#include <D3D12.h>
#include <list>
#include <vector>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

struct DeviceCreationParameters
{
    bool startMaximized;
    bool startFullscreen;
    int windowPosX;
    int windowPosY;
    int backBufferWidth;
    int backBufferHeight;
    int refreshRate;
    int swapChainBufferCount;
    DXGI_FORMAT swapChainFormat;
    DXGI_USAGE swapChainUsage;
    int swapChainSampleCount;
    D3D_DRIVER_TYPE driverType;
    D3D_FEATURE_LEVEL featureLevel;
    bool enableDebugRuntime;

    DeviceCreationParameters()
        : startMaximized(false)
        , startFullscreen(false)
        , windowPosX(CW_USEDEFAULT)
        , windowPosY(CW_USEDEFAULT)
        , backBufferWidth(1280)
        , backBufferHeight(720)
        , refreshRate(0)
        , swapChainBufferCount(2)
        , swapChainFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
        , swapChainUsage(DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT)
        , swapChainSampleCount(1)
        , driverType(D3D_DRIVER_TYPE_HARDWARE)
        , featureLevel(D3D_FEATURE_LEVEL_11_0)
        , enableDebugRuntime(false)
    { }
};

typedef D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView;

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
class IVisualController
{
private:
    bool            m_Enabled;
public:
    IVisualController() : m_Enabled(true) { }

    virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return 1; }
    virtual void Render(RenderTargetView RTV) { }
    virtual void Animate(double fElapsedTimeSeconds) { }
    virtual HRESULT DeviceCreated() { return S_OK; }
    virtual void DeviceDestroyed() { }
    virtual void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) { }

    virtual void EnableController() { m_Enabled = true; }
    virtual void DisableController() { m_Enabled = false; }
    virtual bool IsEnabled() { return m_Enabled; }
};
#pragma warning(pop)


void SetResourceBarrier(ID3D12GraphicsCommandList* commandList,
    ID3D12Resource* res,
    D3D12_RESOURCE_STATES before,
    D3D12_RESOURCE_STATES after);

void SetUavBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* res);

class DeviceManager
{
public:
    enum WindowState
    {
        kWindowNone,
        kWindowNormal,
        kWindowMinimized,
        kWindowMaximized,
        kWindowFullscreen
    };

protected:
    ID3D12Device*                   m_Device12;
    ID3D12CommandQueue*             m_DefaultQueue;
    IDXGISwapChain3*                m_SwapChain;
    DXGI_SWAP_CHAIN_DESC1           m_SwapChainDesc;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_FullScreenDesc;
    ID3D12CommandAllocator*         m_DefaultCommandAllocator;
    ID3D12GraphicsCommandList*		m_PreRenderCommandList;
    ID3D12GraphicsCommandList*		m_PostRenderCommandList;
    ID3D12DescriptorHeap*           m_DescHeapRTV;
    IDXGIAdapter*                   m_DxgiAdapter;
    HWND                            m_hWnd;
    std::list<IVisualController*>   m_vControllers;
    std::wstring                    m_WindowTitle;
    double                          m_FixedFrameInterval;
    UINT                            m_SyncInterval;
    std::list<double>               m_vFrameTimes;
    double                          m_AverageFrameTime;
    double                          m_AverageTimeUpdateInterval;
    bool                            m_InSizingModalLoop;
    SIZE                            m_NewWindowSize;
    std::vector<ID3D12Resource*>    m_SwapChainBuffers;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_SwapChainRTVs;
    ID3D12Fence*                    m_FrameFence;
    std::vector<HANDLE>             m_FrameFenceEvents;
    int                             m_BufferIndex;
    UINT64                          m_FrameCount;
    bool                            m_EnableRenderTargetClear;
    float                           m_RenderTargetClearColor[4];
private:
    HRESULT                 CreateRenderTargets();
    HRESULT                 CreatePrePostRenderCommandLists();
    void                    ResizeSwapChain(bool releaseRenderTargets);
    void                    ReleaseRenderTargets();
    void                    TestDeviceRemoval();
public:

    DeviceManager()
        : m_Device12(NULL)
        , m_DefaultQueue(NULL)
        , m_SwapChain(NULL)
        , m_hWnd(NULL)
        , m_WindowTitle(L"")
        , m_FixedFrameInterval(-1)
        , m_InSizingModalLoop(false)
        , m_SyncInterval(0)
        , m_AverageFrameTime(0)
        , m_AverageTimeUpdateInterval(0.5)
        , m_DefaultCommandAllocator(NULL)
		, m_PreRenderCommandList(NULL)
		, m_PostRenderCommandList(NULL)
        , m_DescHeapRTV(NULL)
        , m_DxgiAdapter(NULL)
        , m_BufferIndex(0)
        , m_FrameCount(1)
        , m_FrameFence(NULL)
    { }

    virtual ~DeviceManager()
    {
        Shutdown();
    }

    virtual HRESULT CreateWindowDeviceAndSwapChain(const DeviceCreationParameters& params, LPWSTR windowTitle);
    virtual HRESULT ChangeBackBufferFormat(DXGI_FORMAT format, UINT sampleCount);
    virtual HRESULT ResizeWindow(int width, int height);
    virtual HRESULT SetFullScreenState(bool fullscreen);
    virtual HRESULT ToggleFullScreen();

    virtual void    Shutdown();
    virtual void    MessageLoop();
    virtual bool    OneFrame(double fElapsedTimeSeconds);
    virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void    Animate(double fElapsedTimeSeconds);
    virtual void    DeviceCreated();
    virtual void    DeviceDestroyed();
    virtual void    BackBufferResized();

    void            AddControllerToFront(IVisualController* pController);
    void            AddControllerToBack(IVisualController* pController);
    void            RemoveController(IVisualController* pController);

    void            SetFixedFrameInterval(double seconds) { m_FixedFrameInterval = seconds; }
    void            DisableFixedFrameInterval() { m_FixedFrameInterval = -1; }

    HWND            GetHWND() { return m_hWnd; }
    ID3D12Device*   GetDevice() { return m_Device12; }
    ID3D12CommandQueue* GetDefaultQueue() { return m_DefaultQueue; }
    ID3D12Resource* GetCurrentBackBuffer() { return m_SwapChainBuffers[m_BufferIndex]; }
    WindowState     GetWindowState();
    bool            GetVsyncEnabled() { return m_SyncInterval > 0; }
    void            SetVsyncEnabled(bool enabled) { m_SyncInterval = enabled ? 1 : 0; }
    HRESULT         GetDisplayResolution(int& width, int& height);
    IDXGIAdapter*   GetDXGIAdapter();
    double          GetAverageFrameTime() { return m_AverageFrameTime; }
    void            SetAverageTimeUpdateInterval(double value) { m_AverageTimeUpdateInterval = value; }
    void            SetPrimaryRenderTargetClearColor(bool enableClear, const float* pColor);
};
