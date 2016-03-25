// ImGui Win32 + DirectX11 binding
// https://github.com/ocornut/imgui

#include <imgui.h>
#include "imgui_impl_dx12.h"

// DirectX
#include <d3d12.h>
#include <d3dx12.h>

#include <d3dcompiler.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#define ALIGNED_SIZE(size, align) ((size + (align - 1)) & ~(align - 1))

// Data
static INT64                    g_Time = 0;
static INT64                    g_TicksPerSecond = 0;

static HWND                         g_hWnd = 0;
static ID3D12Device*                g_pd3dDevice = NULL;
static ID3D12GraphicsCommandList*   g_pd3dCmdList = NULL;
static ID3D12CommandQueue*          g_pd3dCmdQueue = NULL;
static ID3D12Resource*              g_pFontTextureUploadHeap = NULL;

enum DescHeapHandleLayout
{
    eVertexCB = 0,
    eFontTexture = 1,
};

struct GpuFence
{
	ID3D12Fence* pFence = NULL;
	HANDLE hFenceEvent = 0;
	UINT64 FenceValue = 0;

	void Init()
	{
		if (FAILED(g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)))) return;
		pFence->SetName(L"ImguiFence");
		FenceValue = 0;
		hFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	}

	void Release()
	{
		if (pFence) pFence->Release();
		pFence = NULL;
		FenceValue = 0;
		CloseHandle(hFenceEvent);
	}

	void IncrFence()
	{
		FenceValue++;
	}

	void WaitForFence()
	{
		// Schedule a Signal command in the queue.
		if (FAILED(g_pd3dCmdQueue->Signal(pFence, FenceValue))) return;

		if (FAILED(pFence->SetEventOnCompletion(FenceValue, hFenceEvent))) return;

		WaitForSingleObjectEx(hFenceEvent, INFINITE, FALSE);
	}
};

static GpuFence                 g_GpuFence;

struct DescHeapInfo
{
    ID3D12DescriptorHeap*   DescHeap;
    UINT                    BaseOffset;
    UINT                    IncrSize;

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT Index)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = DescHeap->GetCPUDescriptorHandleForHeapStart();
        CpuHandle.ptr += IncrSize * (BaseOffset + Index);
        return CpuHandle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT Index)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle = DescHeap->GetGPUDescriptorHandleForHeapStart();
        GpuHandle.ptr += IncrSize * (BaseOffset + Index);
        return GpuHandle;
    }
};

DescHeapInfo g_DescHeapCbvSrvUav;

static ID3D12Resource*          g_pVB = NULL;
static ID3D12Resource*          g_pIB = NULL;
static D3D12_VERTEX_BUFFER_VIEW g_VBView = {};
static D3D12_INDEX_BUFFER_VIEW  g_IBView = {};
static ID3D10Blob*              g_pVertexShader = NULL;
static ID3D12Resource*          g_pVertexConstantBuffer = NULL;
static ID3D10Blob*              g_pPixelShader = NULL;
static ID3D12RootSignature*     g_pRootSignature = NULL;
static ID3D12PipelineState*     g_pPSO = NULL;
static ID3D12Resource*              g_pFontTexture = NULL;
static D3D12_STATIC_SAMPLER_DESC    g_FontSampler;
static D3D12_CPU_DESCRIPTOR_HANDLE  g_cbCpuHandle;

static int                      VERTEX_BUFFER_SIZE = 20000;     // TODO: Make buffers smaller and grow dynamically as needed.
static int                      INDEX_BUFFER_SIZE = 40000;      // TODO: Make buffers smaller and grow dynamically as needed.

struct VERTEX_CONSTANT_BUFFER
{
    float        mvp[4][4];
};

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
static void ImGui_ImplDX12_RenderDrawLists(ImDrawData* draw_data)
{
    ImDrawVert* vtx_dst = NULL;
    ImDrawIdx* idx_dst = NULL;

    // Copy and convert all vertices into a single contiguous buffer
    if (g_pVB->Map(0, nullptr, reinterpret_cast<void**>(&vtx_dst)) != S_OK)
        return;
    if (g_pIB->Map(0, nullptr, reinterpret_cast<void**>(&idx_dst)) != S_OK)
        return;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        memcpy(vtx_dst, &cmd_list->VtxBuffer[0], cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
        memcpy(idx_dst, &cmd_list->IdxBuffer[0], cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
        vtx_dst += cmd_list->VtxBuffer.size();
        idx_dst += cmd_list->IdxBuffer.size();
    }
    g_pVB->Unmap(0, nullptr);
    g_pIB->Unmap(0, nullptr);

    // Setup orthographic projection matrix into our constant buffer
    {
        VERTEX_CONSTANT_BUFFER* pConstantBuffer = NULL;
        g_pVertexConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pConstantBuffer));
        
        const float L = 0.0f;
        const float R = ImGui::GetIO().DisplaySize.x;
        const float B = ImGui::GetIO().DisplaySize.y;
        const float T = 0.0f;
        const float mvp[4][4] = 
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f},
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f,},
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };
        memcpy(&pConstantBuffer->mvp, mvp, sizeof(mvp));
        g_pVertexConstantBuffer->Unmap(0, nullptr);
    }

    // Setup viewport
    {
        D3D12_VIEWPORT vp;
        memset(&vp, 0, sizeof(D3D12_VIEWPORT));
        vp.Width = ImGui::GetIO().DisplaySize.x;
        vp.Height = ImGui::GetIO().DisplaySize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_pd3dCmdList->RSSetViewports(1, &vp);
    }

    ID3D12DescriptorHeap* descHeaps[] = { g_DescHeapCbvSrvUav.DescHeap };
    g_pd3dCmdList->SetDescriptorHeaps(ARRAYSIZE(descHeaps), descHeaps);

    // Bind shader and vertex buffers
    g_pd3dCmdList->SetGraphicsRootSignature(g_pRootSignature);
    g_pd3dCmdList->SetPipelineState(g_pPSO);
    g_pd3dCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_pd3dCmdList->IASetVertexBuffers(0, 1, &g_VBView);
    g_pd3dCmdList->IASetIndexBuffer(&g_IBView);

    g_pd3dCmdList->SetGraphicsRootConstantBufferView(0, g_pVertexConstantBuffer->GetGPUVirtualAddress());
    //g_pd3dCmdList->SetGraphicsRootDescriptorTable(0, g_DescHeapCbvSrvUav.GetGpuHandle(eCB));

    // Setup render state
    const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    g_pd3dCmdList->OMSetBlendFactor(blendFactor);

    // Render command lists
    int vtx_offset = 0;
    int idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                const D3D12_RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
                D3D12_GPU_DESCRIPTOR_HANDLE* FontTextureGpuHandle = (D3D12_GPU_DESCRIPTOR_HANDLE*)pcmd->TextureId;
                g_pd3dCmdList->SetGraphicsRootDescriptorTable(1, *FontTextureGpuHandle);
                g_pd3dCmdList->RSSetScissorRects(1, &r);
                g_pd3dCmdList->DrawIndexedInstanced(pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.size();
    }

	g_GpuFence.IncrFence();
}

LRESULT ImGui_ImplDX12_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        io.MouseDown[0] = true;
        return true;
    case WM_LBUTTONUP:
        io.MouseDown[0] = false; 
        return true;
    case WM_RBUTTONDOWN:
        io.MouseDown[1] = true; 
        return true;
    case WM_RBUTTONUP:
        io.MouseDown[1] = false; 
        return true;
    case WM_MOUSEWHEEL:
        io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
        return true;
    case WM_MOUSEMOVE:
        io.MousePos.x = (signed short)(lParam);
        io.MousePos.y = (signed short)(lParam >> 16); 
        return true;
    case WM_KEYDOWN:
        if (wParam < 256)
            io.KeysDown[wParam] = 1;
        return true;
    case WM_KEYUP:
        if (wParam < 256)
            io.KeysDown[wParam] = 0;
        return true;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacter((unsigned short)wParam);
        return true;
    }
    return 0;
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

static void ImGui_ImplDX12_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();

    ImFont* font0 = io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 18);

    // Build
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Create DX12 texture
    {
        D3D12_RESOURCE_DESC Desc = {};
        Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        Desc.Width = width;
        Desc.Height = height;
        Desc.MipLevels = 1;
        Desc.DepthOrArraySize = 1;
        Desc.SampleDesc.Count = 1;
        Desc.SampleDesc.Quality = 0;
        Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.Alignment = 0;

        D3D12_HEAP_PROPERTIES texHeapProp = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1, 1
        };

        if (g_pFontTexture) g_pFontTexture->Release();

        if (g_pd3dDevice->CreateCommittedResource(
            &texHeapProp,
            D3D12_HEAP_FLAG_NONE,
            &Desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&g_pFontTexture)) != S_OK)
        {
            assert(0);
            return;
        }

        g_pFontTexture->SetName(L"ImGuiFontTexture");

        // Initialize texture
        {
            // Create the texture.
            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(g_pFontTexture, 0, 1);

            // Create the GPU upload buffer.
            if (g_pd3dDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&g_pFontTextureUploadHeap)) != S_OK)
            {
                assert(0);
                return;
            }
            g_pFontTextureUploadHeap->SetName(L"Imgui_TextureUploadHeap");
            // Copy data to the intermediate upload heap and then schedule a copy 
            // from the upload heap to the Texture2D.
            D3D12_SUBRESOURCE_DATA textureData = {};
            textureData.pData = pixels;
            textureData.RowPitch = width * 4;
            textureData.SlicePitch = textureData.RowPitch * height;

            SetResourceBarrier(g_pd3dCmdList, g_pFontTexture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);

            UpdateSubresources(g_pd3dCmdList, g_pFontTexture, g_pFontTextureUploadHeap, 0, 0, 1, &textureData);

            SetResourceBarrier(g_pd3dCmdList, g_pFontTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
        
        // SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SRVDesc.Texture2D.MipLevels = 1;
        SRVDesc.Texture2D.MostDetailedMip = 0; // No MIP
        SRVDesc.Texture2D.PlaneSlice = 0;
        SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = g_DescHeapCbvSrvUav.GetCpuHandle(eFontTexture);
        g_pd3dDevice->CreateShaderResourceView(g_pFontTexture, &SRVDesc, CpuHandle);

        // Store our identifier
        D3D12_GPU_DESCRIPTOR_HANDLE* pGpuHandle = new D3D12_GPU_DESCRIPTOR_HANDLE;
        (*pGpuHandle) = g_DescHeapCbvSrvUav.GetGpuHandle(eFontTexture);
        io.Fonts->TexID = (void *)pGpuHandle;

        //D3D12_SAMPLER_DESC samplerDesc;
        //samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        //samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        //samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        //samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        //samplerDesc.MinLOD = -FLT_MAX;
        //samplerDesc.MaxLOD = FLT_MAX;
        //samplerDesc.MipLODBias = 0;
        //samplerDesc.MaxAnisotropy = 0;
        //samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        //mDev->CreateSampler(&samplerDesc, mDescHeapSampler->GetCPUDescriptorHandleForHeapStart());
    }

    // Cleanup (don't clear the input data if you want to append new fonts later)
    io.Fonts->ClearInputData();
    io.Fonts->ClearTexData();
}

bool    ImGui_ImplDX12_CreateDeviceObjects()
{
    if (!g_pd3dDevice)
        return false;
    if (g_pVB)
        ImGui_ImplDX12_InvalidateDeviceObjects();

    D3D12_HEAP_PROPERTIES uploadHeapProp = {
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, 1
    };

    ID3D10Blob* pVertexShaderBlob = NULL;
    ID3D10Blob* pPixelShaderBlob = NULL;

    // Create the vertex shader
    {
        static const char* vertexShader = 
            "cbuffer vertexBuffer : register(b0) \
            {\
            float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
            float2 pos : POSITION;\
            float2 uv  : TEXCOORD0;\
            float4 col : COLOR0;\
            };\
            \
            struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
            PS_INPUT output;\
            output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
            output.col = input.col;\
            output.uv  = input.uv;\
            return output;\
            }";

        D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_5_0", 0, 0, &pVertexShaderBlob, NULL);
        if (pVertexShaderBlob == NULL) // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
        {
            assert(0);
            return false;
        }

        // Create the constant buffer
        {
            D3D12_RESOURCE_DESC cbResourceDesc = {
                D3D12_RESOURCE_DIMENSION_BUFFER,
                0,
                D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                1, 1, 1,
                DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE
            };

            if (g_pd3dDevice->CreateCommittedResource(
                &uploadHeapProp,
                D3D12_HEAP_FLAG_NONE,
                &cbResourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&g_pVertexConstantBuffer)) != S_OK)
            {
                assert(0);
                return false;
            }

            g_pVertexConstantBuffer->SetName(L"ImguiVertexCB");

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = g_pVertexConstantBuffer->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

            D3D12_CPU_DESCRIPTOR_HANDLE g_cbCpuHandle = g_DescHeapCbvSrvUav.GetCpuHandle(eVertexCB);
            g_pd3dDevice->CreateConstantBufferView(&cbvDesc, g_cbCpuHandle);
        }
    }

    // Create the pixel shader
    {
        static const char* pixelShader = 
            "struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            sampler sampler0;\
            Texture2D texture0;\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
            float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
            return out_col; \
            }";

        D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_5_0", 0, 0, &pPixelShaderBlob, NULL);
        if (pPixelShaderBlob == NULL)  // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
        {
            assert(0);
            return false;
        }
    }

    // Create the blending setup
    D3D12_BLEND_DESC localBlendState;
    {
        ZeroMemory(&localBlendState, sizeof(localBlendState));
        localBlendState.IndependentBlendEnable = false;
        localBlendState.AlphaToCoverageEnable = false;
        localBlendState.RenderTarget[0].BlendEnable = true;
        localBlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        localBlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        localBlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        localBlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        localBlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        localBlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        localBlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    // Create the rasterizer state
    D3D12_RASTERIZER_DESC localRasterizeState;
    {
        ZeroMemory(&localRasterizeState, sizeof(localRasterizeState));
        localRasterizeState.FillMode = D3D12_FILL_MODE_SOLID;
        localRasterizeState.CullMode = D3D12_CULL_MODE_NONE;
        //localRasterizeState.ScissorEnable = true;
        localRasterizeState.DepthClipEnable = true;
    }

    // Create the vertex buffer
    {
        UINT VBSizeInByte = VERTEX_BUFFER_SIZE * sizeof(ImDrawVert);
        D3D12_RESOURCE_DESC vbResourceDesc = {
            D3D12_RESOURCE_DIMENSION_BUFFER,
            0,
            VBSizeInByte, 1, 1, 1,
            DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE
        };

        if (FAILED(g_pd3dDevice->CreateCommittedResource(
            &uploadHeapProp,
            D3D12_HEAP_FLAG_NONE,
            &vbResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&g_pVB))))
        {
            assert(0);
            return false;
        }

        g_VBView.BufferLocation = g_pVB->GetGPUVirtualAddress();
        g_VBView.StrideInBytes = sizeof(ImDrawVert);
        g_VBView.SizeInBytes = VBSizeInByte;
    }

    // Create the index buffer
    {
        UINT IBSizeInByte = INDEX_BUFFER_SIZE * sizeof(ImDrawIdx);;
        D3D12_RESOURCE_DESC ibResourceDesc = {
            D3D12_RESOURCE_DIMENSION_BUFFER,
            0,
            IBSizeInByte, 1, 1, 1,
            DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE
        };

        if (FAILED(g_pd3dDevice->CreateCommittedResource(
            &uploadHeapProp,
            D3D12_HEAP_FLAG_NONE,
            &ibResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&g_pIB))))
        {
            assert(0);
            return false;
        }

        g_IBView.BufferLocation = g_pIB->GetGPUVirtualAddress();
        g_IBView.Format = DXGI_FORMAT_R16_UINT;
        g_IBView.SizeInBytes = IBSizeInByte;
    }

    D3D12_INPUT_ELEMENT_DESC localLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8 + 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    {
#if (1)
        D3D12_DESCRIPTOR_RANGE range1[1];
        range1[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range1[0].NumDescriptors = 1;
        range1[0].BaseShaderRegister = 0;
        range1[0].RegisterSpace = 0;
        range1[0].OffsetInDescriptorsFromTableStart = 0;
        
        D3D12_DESCRIPTOR_RANGE range2[1];
        range2[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range2[0].NumDescriptors = 1;
        range2[0].BaseShaderRegister = 0;
        range2[0].RegisterSpace = 0;
        range2[0].OffsetInDescriptorsFromTableStart = 0;

        D3D12_ROOT_PARAMETER rootParam[2];
        // Init as constants
        rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParam[0].Descriptor.RegisterSpace = 0;
        rootParam[0].Descriptor.ShaderRegister = 0;
        rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
        rootParam[1].DescriptorTable.pDescriptorRanges = range2;
        rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        //rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        //rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
        //rootParam[0].DescriptorTable.pDescriptorRanges = range1;
        //rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        //rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        //rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
        //rootParam[1].DescriptorTable.pDescriptorRanges = range2;
        //rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#else
        CD3DX12_DESCRIPTOR_RANGE descRange1[1];
        descRange1[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0
        CD3DX12_DESCRIPTOR_RANGE descRange2[1];
        descRange2[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // srv0

        CD3DX12_ROOT_PARAMETER rootParam[2];
        rootParam[0].InitAsDescriptorTable(ARRAYSIZE(descRange1), descRange1, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParam[1].InitAsDescriptorTable(ARRAYSIZE(descRange2), descRange2, D3D12_SHADER_VISIBILITY_PIXEL);
#endif

        D3D12_STATIC_SAMPLER_DESC samplerDesc;
        {
            ZeroMemory(&samplerDesc, sizeof(samplerDesc));
            samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.MipLODBias = 0.f;
            samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            samplerDesc.MinLOD = 0.f;
            samplerDesc.MaxLOD = 0.f;
        }

        ID3D10Blob *sig, *info;
        D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
        rootSigDesc.NumParameters = sizeof(rootParam)/sizeof(rootParam[0]);
        rootSigDesc.pParameters = rootParam;
        rootSigDesc.NumStaticSamplers = 1;
        rootSigDesc.pStaticSamplers = &samplerDesc;
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &info)))
        {
            assert(0);
            return false;
        }
        
        g_pd3dDevice->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&g_pRootSignature));
		g_pRootSignature->SetName(L"ImguiRS");
        sig->Release();
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.InputLayout.NumElements = 3;
    psoDesc.InputLayout.pInputElementDescs = localLayout;
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    psoDesc.pRootSignature = g_pRootSignature;
    psoDesc.VS.pShaderBytecode = pVertexShaderBlob->GetBufferPointer();
    psoDesc.VS.BytecodeLength = pVertexShaderBlob->GetBufferSize();
    psoDesc.PS.pShaderBytecode = pPixelShaderBlob->GetBufferPointer();
    psoDesc.PS.BytecodeLength = pPixelShaderBlob->GetBufferSize();
    psoDesc.RasterizerState = localRasterizeState;
    psoDesc.BlendState = localBlendState;
    psoDesc.DepthStencilState.DepthEnable = false;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.StencilEnable = false;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    psoDesc.SampleDesc.Count = 1;
    if (FAILED(g_pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pPSO))))
    {
        assert(0);
        return false;
    }
	g_pPSO->SetName(L"ImGuiPso");
    pVertexShaderBlob->Release();
    pPixelShaderBlob->Release();

    ImGui_ImplDX12_CreateFontsTexture();

    return true;
}

void    ImGui_ImplDX12_InvalidateDeviceObjects()
{
    if (!g_pd3dDevice)
        return;

    if (g_pFontTexture)
    {
        g_pFontTexture->Release();
        g_pFontTexture = NULL;
        D3D12_GPU_DESCRIPTOR_HANDLE* pGpuHandle = (D3D12_GPU_DESCRIPTOR_HANDLE*)ImGui::GetIO().Fonts->TexID;
        delete pGpuHandle;
        ImGui::GetIO().Fonts->TexID = 0;
    }

    if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
    if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
    if (g_pVertexConstantBuffer) { g_pVertexConstantBuffer->Release(); g_pVertexConstantBuffer = NULL; }
    if (g_pPSO) { g_pPSO->Release(); g_pPSO = NULL; }
    if (g_pRootSignature) { g_pRootSignature->Release(); g_pRootSignature = NULL; }
}

bool    ImGui_ImplDX12_Init(void* hwnd, ID3D12Device* device, ID3D12CommandQueue* pCmdQueue, ID3D12GraphicsCommandList* pCmdList, ID3D12DescriptorHeap* descHeap, unsigned int descHeapBaseOffset)
{
    g_hWnd = (HWND)hwnd;
    g_pd3dDevice = device;
    g_pd3dCmdList = pCmdList;
	g_pd3dCmdQueue = pCmdQueue;
    
    if (!descHeap)
        return false;
    
	g_GpuFence.Init();

    g_DescHeapCbvSrvUav.DescHeap = descHeap;
    g_DescHeapCbvSrvUav.BaseOffset = descHeapBaseOffset;
    g_DescHeapCbvSrvUav.IncrSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond)) 
        return false;
    if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
        return false;

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;                              // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    io.RenderDrawListsFn = ImGui_ImplDX12_RenderDrawLists;
    io.ImeWindowHandle = g_hWnd;

    return true;
}

void ImGui_ImplDX12_Shutdown()
{
    // Command list should be closed before this method
    g_GpuFence.WaitForFence();
    g_GpuFence.Release();
    
    if (g_pFontTextureUploadHeap) g_pFontTextureUploadHeap->Release();
    g_pFontTextureUploadHeap = NULL;
    if (g_pFontTexture) g_pFontTexture->Release();
    g_pFontTexture = NULL;

    ImGui_ImplDX12_InvalidateDeviceObjects();
    ImGui::Shutdown();
    g_pd3dDevice = NULL;
    g_pd3dCmdList = NULL;
    g_hWnd = (HWND)0;
}

void ImGui_ImplDX12_NewFrame()
{
    if (!g_pVB)
        ImGui_ImplDX12_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    GetClientRect(g_hWnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER *)&current_time); 
    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
    // io.MousePos : filled by WM_MOUSEMOVE events
    // io.MouseDown : filled by WM_*BUTTON* events
    // io.MouseWheel : filled by WM_MOUSEWHEEL events

    // Hide OS mouse cursor if ImGui is drawing it
    SetCursor(io.MouseDrawCursor ? NULL : LoadCursor(NULL, IDC_ARROW));

    // Start the frame
    ImGui::NewFrame();
}
