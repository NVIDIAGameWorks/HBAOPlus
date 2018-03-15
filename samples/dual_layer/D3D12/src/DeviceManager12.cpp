/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "DeviceManager12.h"
#include <WinUser.h>
#include <Windows.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

#define WINDOW_CLASS_NAME   L"NvDX12"

#define WINDOW_STYLE_NORMAL         (WS_OVERLAPPEDWINDOW | WS_VISIBLE)
#define WINDOW_STYLE_FULLSCREEN     (WS_POPUP | WS_SYSMENU | WS_VISIBLE)

#define V_RETURN(code) { hr = (code); if(FAILED(hr)) return hr; }
#define HR_RETURN(hr) if(FAILED(hr)) return hr;

LRESULT CALLBACK WindowProc12(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DeviceManager* pDM = (DeviceManager*)GetWindowLongPtr(hWnd, 0);
    if(pDM)
        return pDM->MsgProc(hWnd, uMsg, wParam, lParam);
    else
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void SetResourceBarrier(ID3D12GraphicsCommandList* commandList,
	ID3D12Resource* res,
	D3D12_RESOURCE_STATES before,
	D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER desc = {};
	desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	desc.Transition.pResource = res;
	desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	desc.Transition.StateBefore = before;
	desc.Transition.StateAfter = after;
	desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	commandList->ResourceBarrier(1, &desc);
}

void SetUavBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* res)
{
	D3D12_RESOURCE_BARRIER desc = {};
	desc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	desc.Transition.pResource = res;
	desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	desc.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	desc.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	commandList->ResourceBarrier(1, &desc);
}

HRESULT
DeviceManager::CreateWindowDeviceAndSwapChain(const DeviceCreationParameters& params, LPWSTR title)
{
    m_WindowTitle = title;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEX windowClass = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc12,
                        0L, sizeof(void*), hInstance, NULL, NULL, NULL, NULL, WINDOW_CLASS_NAME, NULL };

    RegisterClassEx(&windowClass);

    UINT windowStyle = params.startFullscreen 
        ? WINDOW_STYLE_FULLSCREEN
        : params.startMaximized 
            ? (WINDOW_STYLE_NORMAL | WS_MAXIMIZE) 
            : WINDOW_STYLE_NORMAL;
        
    RECT rect = { 0, 0, params.backBufferWidth, params.backBufferHeight };
    AdjustWindowRect(&rect, windowStyle, FALSE);

    m_hWnd = CreateWindowEx(
        0,
        WINDOW_CLASS_NAME, 
        title, 
        windowStyle, 
        params.windowPosX, 
        params.windowPosY, 
        rect.right - rect.left, 
        rect.bottom - rect.top, 
        GetDesktopWindow(),
        NULL,
        hInstance,
        NULL
    );

    if(!m_hWnd)
    {
#ifdef DEBUG
        DWORD errorCode = GetLastError();    
        printf("CreateWindowEx error code = 0x%x\n", errorCode);
#endif

        MessageBox(NULL, L"Cannot create window", m_WindowTitle.c_str(), MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

	SetWindowLongPtr(m_hWnd, 0, (LONG_PTR)this);
    UpdateWindow(m_hWnd);

    HRESULT hr = E_FAIL;

    RECT clientRect;
    GetClientRect(m_hWnd, &clientRect);
    UINT width = clientRect.right - clientRect.left;
    UINT height = clientRect.bottom - clientRect.top;

    ZeroMemory(&m_SwapChainDesc, sizeof(m_SwapChainDesc));
    m_SwapChainDesc.Width = width;
    m_SwapChainDesc.Height = height;
    m_SwapChainDesc.Format = params.swapChainFormat;
    m_SwapChainDesc.SampleDesc.Count = params.swapChainSampleCount;
    m_SwapChainDesc.SampleDesc.Quality = 0;
    m_SwapChainDesc.BufferUsage = params.swapChainUsage;
    m_SwapChainDesc.BufferCount = params.swapChainBufferCount;
    m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    m_SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    if(params.enableDebugRuntime)
    {
        ID3D12Debug* pDebug = NULL;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug));
        HR_RETURN(hr);

        pDebug->EnableDebugLayer();

        pDebug->Release();
    }

    IDXGIFactory2* pDxgiFactory = NULL;
    hr = CreateDXGIFactory2(params.enableDebugRuntime ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&pDxgiFactory));
    HR_RETURN(hr);

    IDXGIAdapter* pAdapter = NULL;
#if 0
    UINT nAdapter = 0;
    while(SUCCEEDED(pDxgiFactory->EnumAdapters(nAdapter, &pAdapter)))
    {
        DXGI_ADAPTER_DESC adapterDesc;
        pAdapter->GetDesc(&adapterDesc);
        OutputDebugStringW(adapterDesc.Description);
        nAdapter++;
        //break;
    }
#endif
        
    hr = D3D12CreateDevice(
        pAdapter,
        params.featureLevel,
        IID_PPV_ARGS(&m_Device12));
    HR_RETURN(hr);

    LUID deviceLuid = m_Device12->GetAdapterLuid();
    UINT nAdapter = 0;
    while(SUCCEEDED(pDxgiFactory->EnumAdapters(nAdapter, &pAdapter)))
    {
        DXGI_ADAPTER_DESC adapterDesc;
        pAdapter->GetDesc(&adapterDesc);

        if (deviceLuid.HighPart == adapterDesc.AdapterLuid.HighPart &&
            deviceLuid.LowPart == adapterDesc.AdapterLuid.LowPart)
        {
            m_DxgiAdapter = pAdapter;
            break;
        }
        else
            pAdapter->Release();
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc;
    ZeroMemory(&queueDesc, sizeof(queueDesc));
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.NodeMask = 1;
    hr = m_Device12->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_DefaultQueue));
    HR_RETURN(hr);

    m_FullScreenDesc = {};
    m_FullScreenDesc.RefreshRate.Numerator = params.refreshRate;
    m_FullScreenDesc.RefreshRate.Denominator = 1;
    m_FullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    m_FullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    m_FullScreenDesc.Windowed = !params.startFullscreen;

	IDXGISwapChain1* pSwapChain1 = NULL;
    hr = pDxgiFactory->CreateSwapChainForHwnd(m_DefaultQueue, m_hWnd, &m_SwapChainDesc, &m_FullScreenDesc, NULL, &pSwapChain1);
    HR_RETURN(hr);

	hr = pSwapChain1->QueryInterface(IID_PPV_ARGS(&m_SwapChain));
	pSwapChain1->Release();
	HR_RETURN(hr);


    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    ZeroMemory(&heapDesc, sizeof(heapDesc));
    heapDesc.NumDescriptors = m_SwapChainDesc.BufferCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    hr = m_Device12->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescHeapRTV));
    HR_RETURN(hr);
    
    V_RETURN( CreateRenderTargets() );

    hr = m_Device12->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_FrameFence));
    HR_RETURN(hr);

    for(UINT bufferIndex = 0; bufferIndex < m_SwapChainDesc.BufferCount; bufferIndex++)
    {
        m_FrameFenceEvents.push_back( CreateEvent(NULL, false, true, NULL) );
    }

    DeviceCreated();
    BackBufferResized();

    return S_OK;
}

void
DeviceManager::Shutdown() 
{   
    for(auto fenceEvent : m_FrameFenceEvents)
    {
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }

    m_FrameFenceEvents.clear();

    if(m_SwapChain && GetWindowState() == kWindowFullscreen)
        m_SwapChain->SetFullscreenState(false, NULL);

    DeviceDestroyed();
    
    for(auto& buffer : m_SwapChainBuffers)
        SAFE_RELEASE(buffer);

    SAFE_RELEASE(m_FrameFence);

    SAFE_RELEASE(m_DefaultCommandAllocator);
	SAFE_RELEASE(m_PreRenderCommandList);
	SAFE_RELEASE(m_PostRenderCommandList);

    SAFE_RELEASE(m_DescHeapRTV);

    SAFE_RELEASE(m_SwapChain);
    SAFE_RELEASE(m_DefaultQueue);
    SAFE_RELEASE(m_Device12);

    if(m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }
}

HRESULT
DeviceManager::CreateRenderTargets()
{
    HRESULT hr;
    
    m_SwapChainBuffers.resize(m_SwapChainDesc.BufferCount);
    m_SwapChainRTVs.resize(m_SwapChainDesc.BufferCount);
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = m_DescHeapRTV->GetCPUDescriptorHandleForHeapStart();
    
    for(UINT n = 0; n < m_SwapChainDesc.BufferCount; n++)
    {
        V_RETURN( m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_SwapChainBuffers[n])) );

        m_Device12->CreateRenderTargetView(m_SwapChainBuffers[n], NULL, cpuDescriptor);
        m_SwapChainRTVs[n] = cpuDescriptor;
        
        cpuDescriptor.ptr += m_Device12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
    
    return S_OK;
}

HRESULT DeviceManager::CreatePrePostRenderCommandLists()
{
    HRESULT hr; 

    if (!m_DefaultCommandAllocator)
    {
        hr = m_Device12->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_DefaultCommandAllocator));
        HR_RETURN(hr);
    }

    if (!m_PreRenderCommandList)
    {
        hr = m_Device12->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_DefaultCommandAllocator, NULL, IID_PPV_ARGS(&m_PreRenderCommandList));
        HR_RETURN(hr);
    }
    else
		m_PreRenderCommandList->Reset(m_DefaultCommandAllocator, nullptr);

    SetResourceBarrier(m_PreRenderCommandList, m_SwapChainBuffers[m_BufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    if(m_EnableRenderTargetClear)
    {
		m_PreRenderCommandList->ClearRenderTargetView(m_SwapChainRTVs[m_BufferIndex], m_RenderTargetClearColor, 0, NULL);
    }
	m_PreRenderCommandList->Close();


    if (!m_PostRenderCommandList)
    {
        hr = m_Device12->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_DefaultCommandAllocator, NULL, IID_PPV_ARGS(&m_PostRenderCommandList));
        HR_RETURN(hr);
    }
    else
		m_PostRenderCommandList->Reset(m_DefaultCommandAllocator, nullptr);

    SetResourceBarrier(m_PostRenderCommandList, m_SwapChainBuffers[m_BufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_PostRenderCommandList->Close();
    
    return S_OK;
}

bool    
DeviceManager::OneFrame(double fElapsedTimeSeconds)
{
    if(!m_SwapChain || GetWindowState() == kWindowMinimized)
        return false;

    Animate(fElapsedTimeSeconds);

    TestDeviceRemoval();

    DXGI_SWAP_CHAIN_DESC1 newSwapChainDesc;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC newFullScreenDesc;
    if(SUCCEEDED(m_SwapChain->GetDesc1(&newSwapChainDesc)) && SUCCEEDED(m_SwapChain->GetFullscreenDesc(&newFullScreenDesc)))
    {
        if(m_FullScreenDesc.Windowed != newFullScreenDesc.Windowed)
        {
            ReleaseRenderTargets();
            m_SwapChain->ResizeBuffers(newSwapChainDesc.BufferCount, newSwapChainDesc.Width, newSwapChainDesc.Height, newSwapChainDesc.Format, newSwapChainDesc.Flags);
            CreateRenderTargets();
        }

        m_FullScreenDesc = newFullScreenDesc;
        m_SwapChainDesc = newSwapChainDesc;
    }

	m_BufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
	CreatePrePostRenderCommandLists();

    WaitForSingleObject(m_FrameFenceEvents[m_BufferIndex], INFINITE);

    m_DefaultQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_PreRenderCommandList);

    TestDeviceRemoval();
        
    // rendering back-to-front
    for(auto it = m_vControllers.rbegin(); it != m_vControllers.rend(); it++)
    {
        if((*it)->IsEnabled())
        {
            (*it)->Render(m_SwapChainRTVs[m_BufferIndex]);
        }
    }

    m_DefaultQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_PostRenderCommandList);

    m_SwapChain->Present(m_SyncInterval, 0);
    
    TestDeviceRemoval();

    m_FrameFence->SetEventOnCompletion(m_FrameCount, m_FrameFenceEvents[m_BufferIndex]);
    m_DefaultQueue->Signal(m_FrameFence, m_FrameCount);

    m_FrameCount++;
    return true;
}

void
DeviceManager::MessageLoop() 
{
    MSG msg = {0};

    LARGE_INTEGER perfFreq, previousTime;
    QueryPerformanceFrequency(&perfFreq);
    QueryPerformanceCounter(&previousTime);
    
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            LARGE_INTEGER newTime;
            QueryPerformanceCounter(&newTime);
            
            double elapsedSeconds = (m_FixedFrameInterval >= 0) 
                ? m_FixedFrameInterval  
                : (double)(newTime.QuadPart - previousTime.QuadPart) / (double)perfFreq.QuadPart;

            if(OneFrame(elapsedSeconds))
            {
                Sleep(0);
            }
            else
            {
                // Release CPU resources when idle
                Sleep(1);
            }

            {
                m_vFrameTimes.push_back(elapsedSeconds);
                double timeSum = 0;
                for(auto it = m_vFrameTimes.begin(); it != m_vFrameTimes.end(); it++)
                    timeSum += *it;

                if(timeSum > m_AverageTimeUpdateInterval)
                {
                    m_AverageFrameTime = timeSum / (double)m_vFrameTimes.size();
                    m_vFrameTimes.clear();

                    //char buf[256];
                    //sprintf_s(buf, "Average FPS: %.2f\n", 1.f / m_AverageFrameTime);
                    //OutputDebugStringA(buf);
                }
            }

            previousTime = newTime;
        }
    }
}

LRESULT 
DeviceManager::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_DESTROY:
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        case WM_SYSKEYDOWN:
            if(wParam == VK_F4)
            {
                PostQuitMessage(0);
                return 0;
            }
            break;

        case WM_ENTERSIZEMOVE:
            m_InSizingModalLoop = true;
            m_NewWindowSize.cx = m_SwapChainDesc.Width;
            m_NewWindowSize.cy = m_SwapChainDesc.Height;
            break;

        case WM_EXITSIZEMOVE:
            m_InSizingModalLoop = false;
            ResizeSwapChain(true);
            break;

        case WM_SIZE:
            // Ignore the WM_SIZE event if there is no device,
            // or if the window has been minimized (size == 0),
            // or if it has been restored to the previous size (this part is tested inside ResizeSwapChain)
            if (m_Device12 && (lParam != 0))
            {
                m_NewWindowSize.cx = LOWORD(lParam);
                m_NewWindowSize.cy = HIWORD(lParam);

                if(!m_InSizingModalLoop)
                    ResizeSwapChain(true);
            }
    }

    if( uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST || 
        uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST )
    {
        // processing messages front-to-back
        for(auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
        {
            if((*it)->IsEnabled())
            {
                // for kb/mouse messages, 0 means the message has been handled
                if(0 == (*it)->MsgProc(hWnd, uMsg, wParam, lParam))
                    return 0;
            }
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void
DeviceManager::ResizeSwapChain(bool releaseRenderTargets)
{
    if(m_NewWindowSize.cx == (LONG)m_SwapChainDesc.Width && 
       m_NewWindowSize.cy == (LONG)m_SwapChainDesc.Height)
        return;

    m_SwapChainDesc.Width = m_NewWindowSize.cx;
    m_SwapChainDesc.Height = m_NewWindowSize.cy;

    if (m_SwapChain)
    {
        if(releaseRenderTargets)
            ReleaseRenderTargets();

        // Resize the swap chain
        m_SwapChain->ResizeBuffers(m_SwapChainDesc.BufferCount, m_SwapChainDesc.Width, 
                                    m_SwapChainDesc.Height, m_SwapChainDesc.Format, 
                                    m_SwapChainDesc.Flags);
                    
        CreateRenderTargets();

        // Notify the controllers
        BackBufferResized();
    }
}

void DeviceManager::ReleaseRenderTargets()
{
    if(m_FrameFenceEvents.size() == 0)
        return;

    // Make sure that all frames have finished rendering
    WaitForMultipleObjects(DWORD(m_FrameFenceEvents.size()), &m_FrameFenceEvents[0], true, INFINITE);

    // Set the events so that WaitForSingleObject in OneFrame will not hang later
    for(auto e : m_FrameFenceEvents)
        SetEvent(e);
        
    // Release the old buffers because ResizeBuffers requires that
    for(auto& buffer : m_SwapChainBuffers)
        SAFE_RELEASE(buffer);

    m_SwapChainBuffers.clear();
    m_SwapChainRTVs.clear();
}

void DeviceManager::TestDeviceRemoval()
{
    if(m_Device12 == NULL)
        return;

    HRESULT hr = m_Device12->GetDeviceRemovedReason();

    if(SUCCEEDED(hr))
        return;

    char buf[256];
    sprintf_s(buf, "D3D12 Device Removed! Error code = 0x%08x. Terminating the process.\n", hr);
    OutputDebugStringA(buf);

    ExitProcess(1);
}

void
DeviceManager::Animate(double fElapsedTimeSeconds)
{
    // front-to-back, but the order shouldn't matter
    for(auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
    {
        if((*it)->IsEnabled())
        {
            (*it)->Animate(fElapsedTimeSeconds);
        }
    }
}

void
DeviceManager::DeviceCreated()
{
    // creating resources front-to-back
    for(auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
    {
        (*it)->DeviceCreated();
    }
}

void
DeviceManager::DeviceDestroyed()
{
    // releasing resources back-to-front
    for(auto it = m_vControllers.rbegin(); it != m_vControllers.rend(); it++)
    {
        (*it)->DeviceDestroyed();
    }
}

void
DeviceManager::BackBufferResized()
{
    if(m_SwapChain == NULL)
        return;

    for(auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
    {
        (*it)->BackBufferResized(m_SwapChainDesc.Width, m_SwapChainDesc.Height, m_SwapChainDesc.SampleDesc.Count);
    }
}

HRESULT
DeviceManager::ChangeBackBufferFormat(DXGI_FORMAT format, UINT sampleCount)
{
    HRESULT hr = E_FAIL;

    if((format == DXGI_FORMAT_UNKNOWN || format == m_SwapChainDesc.Format) &&
       (sampleCount == 0 || sampleCount == m_SwapChainDesc.SampleDesc.Count))
        return S_FALSE;

    if(m_Device12)
    {
        bool fullscreen = (GetWindowState() == kWindowFullscreen);
        if(fullscreen)
            m_SwapChain->SetFullscreenState(false, NULL);

        IDXGISwapChain1* newSwapChain = NULL;
        DXGI_SWAP_CHAIN_DESC1 newSwapChainDesc = m_SwapChainDesc;

        if(format != DXGI_FORMAT_UNKNOWN)
            newSwapChainDesc.Format = format;
        if(sampleCount != 0)
            newSwapChainDesc.SampleDesc.Count = sampleCount;

        IDXGIAdapter* pDXGIAdapter = GetDXGIAdapter();

        IDXGIFactory2* pDXGIFactory = NULL;
        pDXGIAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory));

        hr = pDXGIFactory->CreateSwapChainForHwnd(m_Device12, m_hWnd, &newSwapChainDesc, NULL, NULL, &newSwapChain);

        pDXGIFactory->Release();
        pDXGIAdapter->Release();
        
        if (FAILED(hr))
        {
            if(fullscreen)
                m_SwapChain->SetFullscreenState(true, NULL);

            return hr;
        }

        SAFE_RELEASE(m_SwapChain);
        
		newSwapChain->QueryInterface(IID_PPV_ARGS(&m_SwapChain));
		SAFE_RELEASE(newSwapChain);
        
        m_SwapChainDesc = newSwapChainDesc;

        if(fullscreen)                
            m_SwapChain->SetFullscreenState(true, NULL);

        CreateRenderTargets();
        BackBufferResized();
    }

    return S_OK;
}

void
DeviceManager::AddControllerToFront(IVisualController* pController) 
{ 
    m_vControllers.remove(pController);
    m_vControllers.push_front(pController);
}

void
DeviceManager::AddControllerToBack(IVisualController* pController) 
{
    m_vControllers.remove(pController);
    m_vControllers.push_back(pController);
}

void
DeviceManager::RemoveController(IVisualController* pController) 
{ 
    m_vControllers.remove(pController);
}

HRESULT 
DeviceManager::ResizeWindow(int width, int height)
{
    if(m_SwapChain == NULL)
        return E_FAIL;

    RECT rect;
    GetWindowRect(m_hWnd, &rect);

    ShowWindow(m_hWnd, SW_RESTORE);

    if(!MoveWindow(m_hWnd, rect.left, rect.top, width, height, true))
        return E_FAIL;

    // No need to call m_SwapChain->ResizeBackBuffer because MoveWindow will send WM_SIZE, which calls that function.

    return S_OK;
}

HRESULT DeviceManager::SetFullScreenState(bool fullscreen)
{
    if(m_SwapChain == NULL)
        return E_FAIL;

    return m_SwapChain->SetFullscreenState(fullscreen, NULL);
}

HRESULT            
DeviceManager::ToggleFullScreen()
{
    return SetFullScreenState(GetWindowState() != kWindowFullscreen);
}

DeviceManager::WindowState
DeviceManager::GetWindowState() 
{ 
    if(m_SwapChain && !m_FullScreenDesc.Windowed)
        return kWindowFullscreen;
        
    if(m_hWnd == INVALID_HANDLE_VALUE)
        return kWindowNone;

    if(IsZoomed(m_hWnd))
        return kWindowMaximized;

    if(IsIconic(m_hWnd))
        return kWindowMinimized;

    return kWindowNormal;
}

HRESULT
DeviceManager::GetDisplayResolution(int& width, int& height)
{
    if(m_hWnd != INVALID_HANDLE_VALUE)
    {
        HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO info;
        info.cbSize = sizeof(MONITORINFO);

        if(GetMonitorInfo(monitor, &info))
        {
            width = info.rcMonitor.right - info.rcMonitor.left;
            height = info.rcMonitor.bottom - info.rcMonitor.top;
            return S_OK;
        }
    }

    return E_FAIL;
}

IDXGIAdapter*   
DeviceManager::GetDXGIAdapter()
{   
    if(m_DxgiAdapter)
        m_DxgiAdapter->AddRef();

    return m_DxgiAdapter;
}

void DeviceManager::SetPrimaryRenderTargetClearColor(bool enableClear, const float * pColor)
{
    m_EnableRenderTargetClear = enableClear;

    if(pColor)
        memcpy(m_RenderTargetClearColor, pColor, sizeof(float) * 4);
}
