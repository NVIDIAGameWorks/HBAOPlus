/* 
* Copyright (c) 2008-2017, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include <Windows.h>
#include <tchar.h>
#include <wrl/client.h>
#include <stdexcept>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <d3dx12p.h>

#include <DirectXMath.h>

// For UI rendering
#include <imgui.h>
#include "imgui_impl_dx12.h"

#include <string>
#include <vector>

// Header for HBAO+
#include "GFSDK_SSAO.h"

// Library link for HBAO+
#ifdef _WIN64
#pragma comment(lib, "GFSDK_SSAO_D3D12.win64.lib")
#else
#pragma comment(lib, "GFSDK_SSAO_D3D12.win32.lib")
#endif

// Use binary mesh reader or obj reader
#define USE_BIN_MESH_READER 1

// MSAA sample count for Viewer application
#define MSAA_SAMPLE_COUNT 1

#define SRC_PATH                L"..\\src\\"
#define HLSL_FILE_PATH          SRC_PATH L"Viewer.hlsl"
#define VERTEX_BIN_FILE_PATH    SRC_PATH L"SibenikVertices.bin"
#define INDEX_BIN_FILE_PATH     SRC_PATH L"SibenikIndices.bin"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 textureCoordinate;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

// For mesh loading
#if USE_BIN_MESH_READER
#include "BinMeshReader.h"
#else
#include "WaveFrontReader.h"
#endif

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    int gWindowWidth = 1280;
    int gWindowHeight = 720;
    bool gIsWindowed = true;
    int gAdapterIndex = -1;
    HWND gMainWindowHandle = 0;
    float gCameraDistance = 1.0f;
    float gModelRotation = 90.0f;
    bool gUseSSAO = true;
    bool gDrawUI = true;
    std::string gSelectedGraphicsAdapter;
    DXGI_FORMAT gColorTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT gNormalTextureFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    DXGI_FORMAT gDepthTextureFormat = DXGI_FORMAT_D32_FLOAT;
};

void CHK(HRESULT hr)
{
    if (FAILED(hr))
        throw runtime_error("HRESULT is failed value.");
}

class D3D
{
public:
    // Number of swap chains
    static const UINT FrameCount = 4;

    ComPtr<IDXGIFactory4> mDxgiFactory;
    ComPtr<IDXGISwapChain3> mSwapChain;

    int mBufferWidth, mBufferHeight;
    UINT mFrameIndex = 0;

    ID3D12Device* mDev;
    ComPtr<ID3D12CommandAllocator> mCmdAllocs[FrameCount];
    ComPtr<ID3D12CommandQueue> mCmdQueue;

    ComPtr<ID3D12GraphicsCommandList> mCmdList;

    ComPtr<ID3D12Fence> mFence;
    UINT64 mFenceValues[FrameCount];
    HANDLE mFenceEvent = 0;

    ComPtr<ID3D12DescriptorHeap> mDescHeapRtv;
    ComPtr<ID3D12DescriptorHeap> mDescHeapDsv;
    ComPtr<ID3D12DescriptorHeap> mDescHeapCbvSrvUav;
    ComPtr<ID3D12DescriptorHeap> mDescHeapSampler;
    ComPtr<ID3D12DescriptorHeap> mSsaoDescHeapCbvSrvUav;

    ComPtr<ID3D12RootSignature> mRootSignature;
    ComPtr<ID3D12PipelineState> mPso;

    ComPtr<ID3D12Resource> mVB;
    D3D12_VERTEX_BUFFER_VIEW mVBView = {};
    D3D12_INDEX_BUFFER_VIEW mIBView = {};
    UINT mIndexCount = 0;
    UINT mVBIndexOffset = 0;

    ComPtr<ID3D12Resource> mDepthBuffer;
    ComPtr<ID3D12Resource> mConstantBuffer;
    void* mCBUploadPtr = nullptr;

    // Normal buffer and render target
    ComPtr<ID3D12Resource> mNormalBuffer[FrameCount];
    GFSDK_SSAO_RenderTargetView_D3D12 mNormalRTV[FrameCount];
    GFSDK_SSAO_ShaderResourceView_D3D12 mNormalSRV[FrameCount];

    // Color buffer and render target
    ComPtr<ID3D12Resource> mColorBuffer[FrameCount];
    GFSDK_SSAO_RenderTargetView_D3D12 mColorRTV[FrameCount];

    // HBAO+ context and parameter
    GFSDK_SSAO_Context_D3D12* mSSAO;
    GFSDK_SSAO_Parameters mAOParams;

public:
    //--------------------------------------------------------------------------------
    D3D(int Width, int Height, HWND hWnd, BOOL IsWindowed, INT AdapterIndex)
        : mBufferWidth(Width), mBufferHeight(Height), mDev(nullptr), mFrameIndex(0), mSSAO(nullptr)
    {
        CHK(CreateDXGIFactory1(IID_PPV_ARGS(mDxgiFactory.ReleaseAndGetAddressOf())));

        ZeroMemory(mFenceValues, sizeof(mFenceValues));

        ID3D12Device* dev = nullptr;
        bool UseWarpDevice = false;

        if (UseWarpDevice)
        {
            ComPtr<IDXGIAdapter> warpAdapter;
            CHK(mDxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

            CHK(D3D12CreateDevice(
                warpAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&dev)
                ));
        }
        else
        {
            ComPtr<IDXGIAdapter> Adapter;
            UINT NumAdapters = 0;
            std::vector< ComPtr<IDXGIAdapter> > Adapters;
            for (UINT idx = 0; mDxgiFactory->EnumAdapters(idx, Adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++idx)
            {
                DXGI_ADAPTER_DESC AdapterDesc;
                Adapter->GetDesc(&AdapterDesc);
                // GPU name : adapter_desc.Description
                wchar_t buff[1024];
                swprintf(buff, L"Graphics Adapter(%d) : %s\n", idx, &AdapterDesc.Description);
                OutputDebugStringW(buff);
                Adapters.push_back(Adapter);
                NumAdapters++;
            }

            if ((AdapterIndex >= 0) && (AdapterIndex < (INT)NumAdapters))
            {
                Adapter = Adapters[AdapterIndex];
            }
            else
            {
                Adapter = Adapters[0];
            }

            DXGI_ADAPTER_DESC AdapterDesc;
            Adapter->GetDesc(&AdapterDesc);
            std::wstring DescW = AdapterDesc.Description;
            std::string DescA = std::string(DescW.begin(), DescW.end());
            gSelectedGraphicsAdapter = DescA;
            Adapters.push_back(Adapter);

            CHK(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev)));
        }

        mDev = dev;

        const UINT NodeMask = 1;

        for (UINT n = 0; n < FrameCount; n++)
        {
            CHK(mDev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdAllocs[n])));
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.NodeMask = NodeMask;
        CHK(mDev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(mCmdQueue.ReleaseAndGetAddressOf())));

        CHK(mDev->CreateCommandList(NodeMask, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocs[mFrameIndex].Get(), nullptr, IID_PPV_ARGS(&mCmdList)));

#define SSAO_NUM_DEPTH_SRV 1
#define SSAO_NUM_NORMAL_SRV FrameCount
#define SSAO_NUM_SRV (SSAO_NUM_DEPTH_SRV + SSAO_NUM_NORMAL_SRV + GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12)

#define IMGUI_NUM_CBV 1
#define IMGUI_NUM_SRV 1

#define VIEWER_NUM_CBV 1
#define VIEWER_NUM_COLOR_RTV FrameCount
#define VIEWER_NUM_NORMAL_RTV FrameCount

#define VIEWER_NUM_RTV (VIEWER_NUM_COLOR_RTV + VIEWER_NUM_NORMAL_RTV)
#define VIEWER_NUM_DSV 1
#define VIEWER_NUM_SAMPLER 0

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};

            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = VIEWER_NUM_RTV + GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12;
            desc.NodeMask = NodeMask;
            CHK(mDev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDescHeapRtv.ReleaseAndGetAddressOf())));
            mDescHeapRtv->SetName(L"ViewerDescHeapRtv");

            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            desc.NumDescriptors = VIEWER_NUM_DSV;
            desc.NodeMask = NodeMask;
            CHK(mDev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDescHeapDsv.ReleaseAndGetAddressOf())));
            mDescHeapDsv->SetName(L"ViewerDescHeapDsv");

            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = VIEWER_NUM_CBV + IMGUI_NUM_CBV + IMGUI_NUM_SRV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = NodeMask;
            CHK(mDev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDescHeapCbvSrvUav.ReleaseAndGetAddressOf())));
            mDescHeapCbvSrvUav->SetName(L"ViewerDescHeapCbvSrvUav");

            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            desc.NumDescriptors = VIEWER_NUM_SAMPLER;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = NodeMask;
            if (desc.NumDescriptors > 0)
            {
                CHK(mDev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDescHeapSampler.ReleaseAndGetAddressOf())));
                mDescHeapSampler->SetName(L"ViewerDescHeapSampler");
            }
        }

        // Create a desc heap for SSAO
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};

            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = SSAO_NUM_SRV;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = NodeMask;
            CHK(mDev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mSsaoDescHeapCbvSrvUav.ReleaseAndGetAddressOf())));
            mSsaoDescHeapCbvSrvUav->SetName(L"ViewerSsaoDescHeapCbvSrvUav");
        }

        CHK(mDev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.ReleaseAndGetAddressOf())));
        mFence->SetName(L"ViewerFence");

        mFenceValues[mFrameIndex]++;

        mFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

        ResizeD3D(hWnd, Width, Height, IsWindowed);

        {
            CD3DX12_DESCRIPTOR_RANGE descRange1[1];
            descRange1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

            CD3DX12_ROOT_PARAMETER rootParam[1];
            rootParam[0].InitAsDescriptorTable(ARRAYSIZE(descRange1), descRange1);

            ID3D10Blob *sig, *info;
            auto rootSigDesc = D3D12_ROOT_SIGNATURE_DESC();
            rootSigDesc.NumParameters = 1;
            rootSigDesc.NumStaticSamplers = 0;
            rootSigDesc.pParameters = rootParam;
            rootSigDesc.pStaticSamplers = nullptr;
            rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            CHK(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &info));
            mDev->CreateRootSignature(
                0,
                sig->GetBufferPointer(),
                sig->GetBufferSize(),
                IID_PPV_ARGS(mRootSignature.ReleaseAndGetAddressOf()));
            mRootSignature->SetName(L"ViewerRS");
            sig->Release();
        }

        ID3D10Blob *vs, *ps;
        {
            ID3D10Blob *info;
            UINT flag = 0;
#if _DEBUG
            flag |= D3DCOMPILE_DEBUG;
#endif /* _DEBUG */
            CHK(D3DCompileFromFile(HLSL_FILE_PATH, nullptr, nullptr, "VSMain", "vs_5_0", flag, 0, &vs, &info));
            CHK(D3DCompileFromFile(HLSL_FILE_PATH, nullptr, nullptr, "PSMain", "ps_5_0", flag, 0, &ps, &info));
        }
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.InputLayout.NumElements = 3;
        psoDesc.InputLayout.pInputElementDescs = inputLayout;
        psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        psoDesc.pRootSignature = mRootSignature.Get();
        psoDesc.VS.pShaderBytecode = vs->GetBufferPointer();
        psoDesc.VS.BytecodeLength = vs->GetBufferSize();
        psoDesc.PS.pShaderBytecode = ps->GetBufferPointer();
        psoDesc.PS.BytecodeLength = ps->GetBufferSize();
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = true;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.StencilEnable = false;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = gNormalTextureFormat;
        psoDesc.DSVFormat = gDepthTextureFormat;
        psoDesc.SampleDesc.Count = MSAA_SAMPLE_COUNT;
        CHK(mDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPso.ReleaseAndGetAddressOf())));
        mPso->SetName(L"ViewerPSO");
        vs->Release();
        ps->Release();

#if (USE_BIN_MESH_READER)
        Mesh mesh;
        LoadVertices(VERTEX_BIN_FILE_PATH, mesh.vertices);
        LoadIndices(INDEX_BIN_FILE_PATH, mesh.indices);
#else
        WaveFrontReader<uint32_t> mesh;
        CHK(mesh.Load(L"sibenik.obj"));
#endif

        mIndexCount = static_cast<UINT>(mesh.indices.size());
        mVBIndexOffset = static_cast<UINT>(sizeof(mesh.vertices[0]) * mesh.vertices.size());
        UINT IBSize = static_cast<UINT>(sizeof(mesh.indices[0]) * mIndexCount);

        void* vbData = mesh.vertices.data();
        void* ibData = mesh.indices.data();
        CHK(mDev->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(mVBIndexOffset + IBSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(mVB.ReleaseAndGetAddressOf())));
        mVB->SetName(L"VertexBuffer");

        char* vbUploadPtr = nullptr;
        CHK(mVB->Map(0, nullptr, reinterpret_cast<void**>(&vbUploadPtr)));
        memcpy_s(vbUploadPtr, mVBIndexOffset, vbData, mVBIndexOffset);
        memcpy_s(vbUploadPtr + mVBIndexOffset, IBSize, ibData, IBSize);
        mVB->Unmap(0, nullptr);

        mVBView.BufferLocation = mVB->GetGPUVirtualAddress();
        mVBView.StrideInBytes = sizeof(mesh.vertices[0]);
        mVBView.SizeInBytes = mVBIndexOffset;
        mIBView.BufferLocation = mVB->GetGPUVirtualAddress() + mVBIndexOffset;
        mIBView.Format = (sizeof(mesh.indices[0]) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        mIBView.SizeInBytes = IBSize;

        // Constant buffer
        CHK(mDev->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(mConstantBuffer.ReleaseAndGetAddressOf())));
        mConstantBuffer->SetName(L"ViewerConstantBuffer");

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = mConstantBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT; // must be a multiple of 256
        mDev->CreateConstantBufferView(&cbvDesc, mDescHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart());
        CHK(mConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mCBUploadPtr)));

        InitSSAO();

        mAOParams = {};
        mAOParams.Radius = 2.f;
        mAOParams.Bias = 0.2f;
        mAOParams.PowerExponent = 2.f;
        mAOParams.Blur.Enable = true;
        mAOParams.Blur.Sharpness = 32.f;
        mAOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;

        ImGui_ImplDX12_Init(hWnd, dev, mCmdQueue.Get(), mCmdList.Get(), mDescHeapCbvSrvUav.Get(), VIEWER_NUM_CBV);
    }

    //--------------------------------------------------------------------------------
    void Release()
    {
        ReleaseSSAO();
        ImGui_ImplDX12_Shutdown();

        mConstantBuffer->Unmap(0, nullptr);
        CloseHandle(mFenceEvent);
        for (UINT Idx = 0; Idx < FrameCount; ++Idx)
        {
            mCmdAllocs[Idx].Reset();
            mColorBuffer[Idx].Reset();
            mNormalBuffer[Idx].Reset();
        }
        mCmdList.Reset();
        mFence.Reset();
        mCmdQueue.Reset();
        mSsaoDescHeapCbvSrvUav.Reset();
        mDescHeapCbvSrvUav.Reset();
        mDescHeapDsv.Reset();
        mDescHeapRtv.Reset();
        mDescHeapSampler.Reset();
        if (mSwapChain)
        {
            mSwapChain->SetFullscreenState(FALSE, NULL);
            mSwapChain.Reset();
        }
        mConstantBuffer.Reset();
        mDepthBuffer.Reset();
        mDxgiFactory.Reset();
        mPso.Reset();
        mRootSignature.Reset();
        mVB.Reset();
    }

    //--------------------------------------------------------------------------------
    ~D3D()
    {
        Release();
    }

    //--------------------------------------------------------------------------------
    void ResizeD3D(HWND hWnd, int Width, int Height, BOOL IsWindowed)
    {
        if (!mDev)
        {
            return;
        }

        DXGI_SWAP_CHAIN_DESC scDesc = {};
        scDesc.BufferCount = FrameCount;
        scDesc.BufferDesc.Width = Width;
        scDesc.BufferDesc.Height = Height;
        scDesc.BufferDesc.Format = gColorTextureFormat;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        //scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT; // For better framerate in windowed mode, but it won't allow fullscreen switching
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        scDesc.OutputWindow = hWnd;
        scDesc.SampleDesc.Count = 1;
        scDesc.SampleDesc.Quality = 0;
        scDesc.Windowed = IsWindowed;

        WaitForGpuIdle();

        // Release color/normal render targets before calling ResizeBuffers().
        // Also reset the frame fence values.
        for (UINT Idx = 0; Idx < FrameCount; ++Idx)
        {
            mColorBuffer[Idx].Reset();
            mNormalBuffer[Idx].Reset();
            mFenceValues[Idx] = mFenceValues[mFrameIndex];
        }

        ComPtr<IDXGISwapChain> swapChain;
        if (!mSwapChain)
        {
            CHK(mDxgiFactory->CreateSwapChain(mCmdQueue.Get(), &scDesc, &swapChain));
            swapChain.As(&mSwapChain);
        }

        DXGI_SWAP_CHAIN_DESC desc = {};
        mSwapChain->GetDesc(&desc);
        CHK(mSwapChain->ResizeBuffers(FrameCount, Width, Height, gColorTextureFormat, desc.Flags));

        mBufferWidth = Width;
        mBufferHeight = Height;

        mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

        for (UINT i = 0; i < FrameCount; i++)
        {
            CHK(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mColorBuffer[i].ReleaseAndGetAddressOf())));
            mColorBuffer[i]->SetName(L"SwapChain_Buffer");
        }

        for (UINT FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
        {
            // Create color buffer RTV
            {
                D3D12_RENDER_TARGET_VIEW_DESC colorRTVDesc = {};
                colorRTVDesc.Format = gColorTextureFormat;
                colorRTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

                CD3DX12_CPU_DESCRIPTOR_HANDLE ColorRTV(
                    mDescHeapRtv->GetCPUDescriptorHandleForHeapStart(),
                    FrameIndex * mDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
                mDev->CreateRenderTargetView(mColorBuffer[FrameIndex].Get(), &colorRTVDesc, ColorRTV);

                mColorRTV[FrameIndex] = {};
                mColorRTV[FrameIndex].CpuHandle = ColorRTV.ptr;
                mColorRTV[FrameIndex].pResource = mColorBuffer[FrameIndex].Get();
            }
        }

        // Create depth buffer and SRV
        {
            // Create depth resource
            {
                D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                    DXGI_FORMAT_R32_TYPELESS, mBufferWidth, mBufferHeight, 1, 1, MSAA_SAMPLE_COUNT, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
                    D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);

                D3D12_CLEAR_VALUE dsvClearValue;
                dsvClearValue.Format = gDepthTextureFormat;
                dsvClearValue.DepthStencil.Depth = 1.0f;
                dsvClearValue.DepthStencil.Stencil = 0;

                CHK(mDev->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // No need to read/write by CPU
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_COMMON,
                    &dsvClearValue,
                    IID_PPV_ARGS(mDepthBuffer.ReleaseAndGetAddressOf())));
                mDepthBuffer->SetName(L"DepthTexture");
            }

            // Depth DSV
            {
                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
#if MSAA_SAMPLE_COUNT > 1
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
#else
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
#endif
                dsvDesc.Format = gDepthTextureFormat;
                dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

                CD3DX12_CPU_DESCRIPTOR_HANDLE DepthDSV(mDescHeapDsv->GetCPUDescriptorHandleForHeapStart());
                mDev->CreateDepthStencilView(mDepthBuffer.Get(), &dsvDesc, DepthDSV);
            }

            // Depth SRV
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};

                depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
                depthSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#if MSAA_SAMPLE_COUNT > 1
                depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
#else
                depthSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                depthSRVDesc.Texture2D.MipLevels = 1;
                depthSRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
                depthSRVDesc.Texture2D.PlaneSlice = 0;
                depthSRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
#endif

                CD3DX12_CPU_DESCRIPTOR_HANDLE DepthSRV(
                    mSsaoDescHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart());
                mDev->CreateShaderResourceView(mDepthBuffer.Get(), &depthSRVDesc, DepthSRV);
            }
        }

        // Normal render targets
        for (UINT FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
        {
            D3D12_RESOURCE_DESC NormalRTVDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                gNormalTextureFormat, mBufferWidth, mBufferHeight, 1, 1, MSAA_SAMPLE_COUNT, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
                D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);

            FLOAT ClearColor[] = { 0, 0, 0, 1.0f };
            CD3DX12_CLEAR_VALUE NormalClearValue(NormalRTVDesc.Format, ClearColor);
            mDev->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                &NormalRTVDesc, D3D12_RESOURCE_STATE_COMMON, &NormalClearValue, IID_PPV_ARGS(mNormalBuffer[FrameIndex].ReleaseAndGetAddressOf()));
            mNormalBuffer[FrameIndex]->SetName(L"NormalBuffer");

            // SRV
            D3D12_SHADER_RESOURCE_VIEW_DESC NormalSRVDesc = {};
            NormalSRVDesc.Format = NormalRTVDesc.Format;
            NormalSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
#if MSAA_SAMPLE_COUNT > 1
            NormalSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
#else
            NormalSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            NormalSRVDesc.Texture2D.MipLevels = 1;
            NormalSRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
            NormalSRVDesc.Texture2D.PlaneSlice = 0;
            NormalSRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
#endif

            mNormalSRV[FrameIndex] = {};
            mNormalSRV[FrameIndex].pResource = mNormalBuffer[FrameIndex].Get();
            CD3DX12_CPU_DESCRIPTOR_HANDLE NormalSRVHandle(
                mSsaoDescHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart(),
                SSAO_NUM_DEPTH_SRV + FrameIndex,
                mDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
            mDev->CreateShaderResourceView(mNormalBuffer[FrameIndex].Get(), &NormalSRVDesc, NormalSRVHandle);

            // RTV
            D3D12_RENDER_TARGET_VIEW_DESC normalRTVDesc = {};
            normalRTVDesc.Format = NormalRTVDesc.Format;
#if MSAA_SAMPLE_COUNT > 1
            normalRTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
#else
            normalRTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            normalRTVDesc.Texture2D.MipSlice = 0;
#endif
            mNormalRTV[FrameIndex] = {};
            mNormalRTV[FrameIndex].pResource = mNormalBuffer[FrameIndex].Get();
            mNormalRTV[FrameIndex].CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
                mDescHeapRtv->GetCPUDescriptorHandleForHeapStart(),
                VIEWER_NUM_COLOR_RTV + FrameIndex,
                mDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)).ptr;
            mDev->CreateRenderTargetView(mNormalBuffer[FrameIndex].Get(), &normalRTVDesc, { mNormalRTV[FrameIndex].CpuHandle });
        }
    }

    void InitSSAO()
    {
        const UINT NodeMask = 1;

        GFSDK_SSAO_CustomHeap CustomHeap;
        CustomHeap.new_ = ::operator new;
        CustomHeap.delete_ = ::operator delete;

        GFSDK_SSAO_DescriptorHeaps_D3D12 DescriptorHeaps;

        DescriptorHeaps.CBV_SRV_UAV.pDescHeap = mSsaoDescHeapCbvSrvUav.Get();
        DescriptorHeaps.CBV_SRV_UAV.BaseIndex = SSAO_NUM_DEPTH_SRV + SSAO_NUM_NORMAL_SRV;
        DescriptorHeaps.CBV_SRV_UAV.NumDescriptors = GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12;

        DescriptorHeaps.RTV.pDescHeap = mDescHeapRtv.Get();
        DescriptorHeaps.RTV.BaseIndex = VIEWER_NUM_RTV;
        DescriptorHeaps.RTV.NumDescriptors = GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12;

        GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D12(mDev, NodeMask, DescriptorHeaps, &mSSAO, &CustomHeap);
        assert(status == GFSDK_SSAO_OK);
    }

    void ReleaseSSAO()
    {
        if (mSSAO)
        {
            mSSAO->Release();
            mSSAO = nullptr;
        }
    }

    //--------------------------------------------------------------------------------
    ID3D12Device* GetDevice() const
    {
        return mDev;
    }

    void WaitForGpuIdle()
    {
        // Schedule a Signal command in the queue.
        const UINT64 currentFenceValue = mFenceValues[mFrameIndex];
        CHK(mCmdQueue->Signal(mFence.Get(), currentFenceValue));

        CHK(mFence->SetEventOnCompletion(mFenceValues[mFrameIndex], mFenceEvent));
        WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
    }

    //--------------------------------------------------------------------------------
    void Draw()
    {
        // Upload constant buffer
        XMMATRIX ViewMat, ProjMat, WorldMat, ViewProjMat;
        {
#if USE_BIN_MESH_READER
            ViewMat = XMMatrixIdentity();
            WorldMat = XMMatrixIdentity();
#else
            WorldMat = XMMatrixRotationY(XMConvertToRadians(gModelRotation));
            ViewMat = XMMatrixLookAtLH({ 0, 1, gCameraDistance }, { 0, 0, 0 }, { 0, 1, 0 });
#endif

            float nearPlane = .01f;
            float farPlane = 500.0f;
            ProjMat = XMMatrixPerspectiveFovLH(40 * 3.141592f / 180.f, (float)mBufferWidth / mBufferHeight, nearPlane, farPlane);
            XMMATRIX MVPTransMat = XMMatrixTranspose(WorldMat * ViewMat * ProjMat);
            XMMATRIX WorldTransMat = XMMatrixTranspose(WorldMat);

            ViewProjMat = XMMatrixMultiply(ViewMat, ProjMat);

            // mCBUploadPtr is Write-Combine memory
            memcpy_s(mCBUploadPtr, 64, &MVPTransMat, 64);
            memcpy_s(reinterpret_cast<char*>(mCBUploadPtr) + 64, 64, &WorldTransMat, 64);
        }

        // Get current RTV descriptor
        CD3DX12_CPU_DESCRIPTOR_HANDLE descHandleRtv(
            mDescHeapRtv->GetCPUDescriptorHandleForHeapStart(),
            mFrameIndex,
            mDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

        // Get current swap chain
        ID3D12Resource* ColorBuffer = mColorBuffer[mFrameIndex].Get();
        ID3D12Resource* DepthBuffer = mDepthBuffer.Get();

        // Get DSV
        CD3DX12_CPU_DESCRIPTOR_HANDLE DepthDSV(mDescHeapDsv->GetCPUDescriptorHandleForHeapStart());

        SetResourceBarrier(mCmdList.Get(), ColorBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        SetResourceBarrier(mCmdList.Get(), DepthBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);

        // Viewport & Scissor
        D3D12_VIEWPORT viewport = {};
        viewport.Width = (float)mBufferWidth;
        viewport.Height = (float)mBufferHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        mCmdList->RSSetViewports(1, &viewport);
        D3D12_RECT scissor = {};
        scissor.right = (LONG)mBufferWidth;
        scissor.bottom = (LONG)mBufferHeight;
        mCmdList->RSSetScissorRects(1, &scissor);

        // Clear DepthTexturesh
        mCmdList->ClearDepthStencilView(DepthDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        // Draw the geometry
        D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { { mNormalRTV[mFrameIndex].CpuHandle } };
        mCmdList->OMSetRenderTargets(ARRAYSIZE(RTVs), RTVs, false, &DepthDSV);

        mCmdList->SetGraphicsRootSignature(mRootSignature.Get());
        ID3D12DescriptorHeap* descHeaps[] = { mDescHeapCbvSrvUav.Get() };
        mCmdList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);

        mCmdList->SetGraphicsRootDescriptorTable(0, mDescHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());
        mCmdList->SetPipelineState(mPso.Get());
        mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mCmdList->IASetVertexBuffers(0, 1, &mVBView);
        mCmdList->IASetIndexBuffer(&mIBView);
        mCmdList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);

        SetResourceBarrier(mCmdList.Get(), DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        if (gUseSSAO)
        {
            // Set input data
            GFSDK_SSAO_InputData_D3D12 InputData = {};
            InputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;

            // FullResDepthTextureSRV
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE DepthSrvGpuHandle(
                    mSsaoDescHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());
                InputData.DepthData.FullResDepthTextureSRV.pResource = DepthBuffer;
                InputData.DepthData.FullResDepthTextureSRV.GpuHandle = DepthSrvGpuHandle.ptr;
            }

            // DepthData
            InputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&ProjMat);
            InputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;

#if USE_BIN_MESH_READER
            InputData.DepthData.MetersToViewSpaceUnits = 0.005f;
#else
            InputData.DepthData.MetersToViewSpaceUnits = 1.f;
#endif

            // NormalData
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE NormalSrvGpuHandle(
                    mSsaoDescHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart(),
                    SSAO_NUM_DEPTH_SRV + mFrameIndex,
                    mDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

                mNormalSRV[mFrameIndex].GpuHandle = NormalSrvGpuHandle.ptr;

#if USE_BIN_MESH_READER
                InputData.NormalData.Enable = false;
#else
                InputData.NormalData.Enable = true;
#endif

                if (InputData.NormalData.Enable)
                {
                    InputData.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&ViewMat);
                    InputData.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
                    InputData.NormalData.FullResNormalTextureSRV = mNormalSRV[mFrameIndex];
                }
            }

            //GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_DEBUG_NORMAL;
            GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO;

            // Set SSAO descriptor heap
            {
                ID3D12DescriptorHeap* descHeaps[] = { mSsaoDescHeapCbvSrvUav.Get() };
                mCmdList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);
            }

            GFSDK_SSAO_Output_D3D12 Output;
            Output.pRenderTargetView = &mColorRTV[mFrameIndex];

            GFSDK_SSAO_Status status = mSSAO->RenderAO(mCmdQueue.Get(), mCmdList.Get(), InputData, mAOParams, Output, RenderMask);
            assert(status == GFSDK_SSAO_OK);

            // Revert to the original descriptor heap
            {
                ID3D12DescriptorHeap* descHeaps[] = { mDescHeapCbvSrvUav.Get() };
                mCmdList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);
            }
        }

        if (gDrawUI)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { mColorRTV[mFrameIndex].CpuHandle };

            mCmdList->OMSetRenderTargets(ARRAYSIZE(RTVs), RTVs, false, nullptr);
            DrawUI();
        }

        // Barrier RenderTarget -> Present
        SetResourceBarrier(mCmdList.Get(), ColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        // Exec
        CHK(mCmdList->Close());
        ID3D12CommandList* const cmdList = mCmdList.Get();
        mCmdQueue->ExecuteCommandLists(1, &cmdList);

        // Present
        CHK(mSwapChain->Present(0, 0));

        // Move to next frame
        {
            // Schedule a Signal command in the queue.
            const UINT64 currentFenceValue = mFenceValues[mFrameIndex];
            CHK(mCmdQueue->Signal(mFence.Get(), currentFenceValue));

            // Update the frame index.
            mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

            // If the next frame is not ready to be rendered yet, wait until it is ready.
            UINT64 CompletedValue = mFence->GetCompletedValue();
            if (CompletedValue < mFenceValues[mFrameIndex])
            {
                CHK(mFence->SetEventOnCompletion(mFenceValues[mFrameIndex], mFenceEvent));
                WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
            }

            // Set the fence value for the next frame.
            mFenceValues[mFrameIndex] = currentFenceValue + 1;
        }

        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        CHK(mCmdAllocs[mFrameIndex]->Reset());

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        CHK(mCmdList->Reset(mCmdAllocs[mFrameIndex].Get(), nullptr));
    }

    //--------------------------------------------------------------------------------
    void DrawUI()
    {
        // Draw UI
        ImGui_ImplDX12_NewFrame();

        bool show_ssao_window = true;
        bool show_test_window = false;

        // Show SSAO property window
        if (show_ssao_window)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("HBAO+", &show_ssao_window);

            GFSDK_SSAO_Version Version;
            GFSDK_SSAO_Status Status;
            Status = GFSDK_SSAO_GetVersion(&Version);
            assert(Status == GFSDK_SSAO_OK);

            ImGui::Text("D3D12 HBAO+ %d.%d.%d.%d", Version.Major, Version.Minor, Version.Branch, Version.Revision);

            ImGui::Text("%s", gSelectedGraphicsAdapter.c_str());
#if MSAA_SAMPLE_COUNT > 1
            ImGui::Text("%dx MSAA", MSAA_SAMPLE_COUNT);
#endif
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            float radius = mAOParams.Radius;
            ImGui::DragFloat("Radius", &radius, 0.05f, 0.0f, 100.0f);
            if (radius != mAOParams.Radius)
            {
                mAOParams.Radius = radius;
                int stop = 0;
                stop = stop;
            }
            ImGui::DragFloat("PowerExponent", &mAOParams.PowerExponent, 0.05f, 1.f, 8.f);
            ImGui::DragFloat("Bias", &mAOParams.Bias, 0.001f, 0, 0.5f);

            ImGui::DragFloat("SmallScaleAO", &mAOParams.SmallScaleAO, 0.01f, 0.f, 2.f);
            ImGui::DragFloat("LargeScaleAO", &mAOParams.LargeScaleAO, 0.01f, 0.f, 2.f);

            bool BlurEnabled = mAOParams.Blur.Enable ? true : false;
            ImGui::Checkbox("Blur.Enable", &BlurEnabled);
            mAOParams.Blur.Enable = BlurEnabled;

            const char* listbox_items[] = { "GFSDK_SSAO_BLUR_RADIUS_2", "GFSDK_SSAO_BLUR_RADIUS_4" };
            int listbox_item_current = (int)mAOParams.Blur.Radius;
            ImGui::ListBox("Blur.Radius", &listbox_item_current, listbox_items, ARRAYSIZE(listbox_items), 2);
            mAOParams.Blur.Radius = (GFSDK_SSAO_BlurRadius)(listbox_item_current);

            ImGui::DragFloat("Blur.Sharpness", &mAOParams.Blur.Sharpness, 0.f, 0.f, 32.0f);

            ImGui::End();
        }

        // Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowTestWindow(&show_test_window);
        }

        ImGui::Render();
    }

private:
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
};

D3D* gD3D = nullptr;

extern LRESULT   ImGui_ImplDX12_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static WORD sLastX = 0, sLastY = 0;

    ImGui_ImplDX12_WndProcHandler(hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_SIZE:
    {
        if (gD3D)
        {
            BOOL IsFullScreen = FALSE;
            if (gD3D->mSwapChain) {
                gD3D->mSwapChain->GetFullscreenState(&IsFullScreen, NULL);
            }
            gD3D->ResizeD3D(hWnd, LOWORD(lParam), HIWORD(lParam), !IsFullScreen);
        }
    }
    break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            gDrawUI = !gDrawUI;
            //PostMessage(hWnd, WM_DESTROY, 0, 0);
            //return 0;
        }
        if (wParam == VK_F5)
        {
            gD3D->ReleaseSSAO();
            gD3D->InitSSAO();
            OutputDebugStringA("SSAO Reloaded\n");
        }
        if (wParam == 'W')
        {
            gCameraDistance -= 0.1f;
        }
        else if (wParam == 'S')
        {
            gCameraDistance += 0.1f;
        }

        break;

    case WM_MOUSEMOVE:
    {
        WORD x = LOWORD(lParam);
        WORD y = HIWORD(lParam);

        if (wParam & MK_RBUTTON)
        {
            gModelRotation -= float(x - sLastX);
        }
        sLastX = x;
        sLastY = y;
    }
    break;
    case WM_PAINT:
        if (gD3D) gD3D->Draw();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//--------------------------------------------------------------------------------
void FitToDesiredClientSize(HWND hwnd, int Width, int Height, DWORD window_style, bool has_menu)
{
    RECT current_win_rect;
    ::GetWindowRect(hwnd, &current_win_rect);
    RECT desired_rect = current_win_rect;
    desired_rect.right = current_win_rect.left + Width;
    desired_rect.bottom = current_win_rect.top + Height;
    ::AdjustWindowRect(&desired_rect, window_style, (has_menu) ? TRUE : FALSE);
    Width = desired_rect.right - desired_rect.left;
    Height = desired_rect.bottom - desired_rect.top;
    ::SetWindowLongPtr(hwnd, GWL_STYLE, window_style);
    ::SetWindowPos(hwnd, HWND_TOP, 0, 0, Width, Height, SWP_SHOWWINDOW);
}

//--------------------------------------------------------------------------------
static HWND SetupWindow(int Width, int Height, bool IsWindowed)
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = (HMODULE)GetModuleHandle(0);
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = _T("WindowClass");
    wcex.hIconSm = nullptr;
    if (!RegisterClassEx(&wcex))
    {
        throw runtime_error("RegisterClassEx()");
    }

    DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    HWND hWnd = CreateWindowEx(
        0, // WS_EX_TOPMOST,
        _T("WindowClass"), _T("HBAO+ DX12"),
        windowStyle,
        0, 0, Width, Height,
        nullptr, nullptr, nullptr, nullptr);
    if (!hWnd)
    {
        throw runtime_error("CreateWindow()");
    }

    if (IsWindowed)
    {
        FitToDesiredClientSize(hWnd, Width, Height, windowStyle, false);
    }

    return hWnd;
}

//--------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR CmdLine, int)
{
    MSG msg;
    ZeroMemory(&msg, sizeof msg);

    ID3D12Device* dev = nullptr;

    std::vector<std::string> arguments;
    if (CmdLine)
    {
        char buff[1024];
        strcpy(buff, CmdLine);
        char * pch = strtok(buff, " ");
        while (pch != NULL)
        {
            arguments.push_back(pch);
            pch = strtok(NULL, " ");
        }
    }

    for (size_t idx = 0; idx < arguments.size(); ++idx)
    {
        std::string arg = arguments[idx];
        if (_stricmp(arg.c_str(), "-width") == 0)
        {
            gWindowWidth = atoi(arguments[idx + 1].c_str());
            idx++;
        }
        else if (_stricmp(arg.c_str(), "-height") == 0)
        {
            gWindowHeight = atoi(arguments[idx + 1].c_str());
            idx++;
        }
        else if (_stricmp(arg.c_str(), "-window") == 0)
        {
            gIsWindowed = (atoi(arguments[idx + 1].c_str()) == 1) ? true : false;
            idx++;
        }
        else if (_stricmp(arg.c_str(), "-ui") == 0)
        {
            gDrawUI = (atoi(arguments[idx + 1].c_str()) == 1) ? true : false;
            idx++;
        }
        else if (_stricmp(arg.c_str(), "-adapter") == 0)
        {
            gAdapterIndex = (atoi(arguments[idx + 1].c_str()) == 1) ? true : false;
            idx++;
        }
    }

#ifdef NDEBUG
    try
#endif
    {
        gMainWindowHandle = SetupWindow(gWindowWidth, gWindowHeight, gIsWindowed);
        ShowWindow(gMainWindowHandle, SW_SHOW);
        UpdateWindow(gMainWindowHandle);

        gD3D = new D3D(gWindowWidth, gWindowHeight, gMainWindowHandle, gIsWindowed, gAdapterIndex);
        dev = gD3D->GetDevice();

        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
#ifdef NDEBUG
    catch (std::exception &e)
    {
        MessageBoxA(gMainWindowHandle, e.what(), "Exception occured.", MB_ICONSTOP);
    }
#endif

    if (gD3D)
        delete gD3D;

    if (dev)
        dev->Release();

    return static_cast<int>(msg.wParam);
}
