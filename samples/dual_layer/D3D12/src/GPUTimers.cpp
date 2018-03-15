/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "GPUTimers.h"
#include <assert.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_D3D_CALL
#define SAFE_D3D_CALL(x)     { if (x != S_OK) assert(0); }
#endif

//--------------------------------------------------------------------------------
void GPUTimers::Create(ID3D12Device* pD3DDevice, ID3D12CommandQueue* pQueue, UINT NumTimers)
{
    m_pQueue = pQueue;
    m_Timers.resize(NumTimers);

    D3D12_QUERY_HEAP_DESC queryHeapDesc{};
    queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    queryHeapDesc.Count = NumTimers * 2;
    SAFE_D3D_CALL( pD3DDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_pQueryHeap)) );

    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 0;
    heapProperties.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resourceDesc.Width = NumTimers * sizeof(UINT64) * 2;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    SAFE_D3D_CALL( pD3DDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_pQueryResults)) );

    for (UINT i = 0; i < m_Timers.size(); ++i)
    {
        m_Timers[i].TimestampQueryInFlight = false;
    }
}

//--------------------------------------------------------------------------------
void GPUTimers::Release()
{
    SAFE_RELEASE(m_pQueryHeap);
    SAFE_RELEASE(m_pQueryResults);
    m_Timers.clear();
}

//--------------------------------------------------------------------------------
void GPUTimers::BeginFrame(ID3D12GraphicsCommandList* pCommandList)
{

}

//--------------------------------------------------------------------------------
void GPUTimers::EndFrame(ID3D12GraphicsCommandList* pCommandList)
{
    UINT64 timestampFrequency;
    m_pQueue->GetTimestampFrequency(&timestampFrequency);

    double InvFrequencyMS = 1000.0 / (double)timestampFrequency;
    pCommandList->ResolveQueryData(m_pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, (UINT)m_Timers.size() * 2, m_pQueryResults, 0);

    UINT64* queryResults = nullptr;
    m_pQueryResults->Map(0, nullptr, (void**)&queryResults);
    
    for (UINT i = 0; i < m_Timers.size(); ++i)
    {
        if (m_Timers[i].TimestampQueryInFlight)
        {
            UINT64 TimestampValueBegin = queryResults[i * 2];
            UINT64 TimestampValueEnd = queryResults[i * 2 + 1];
            m_Timers[i].TimestampQueryInFlight = false;
            m_Timers[i].GPUTimeInMS = float(double(TimestampValueEnd - TimestampValueBegin) * InvFrequencyMS);
        }
        else
        {
            m_Timers[i].GPUTimeInMS = 0.f;
        }
    }
    m_pQueryResults->Unmap(0, nullptr);
}

//--------------------------------------------------------------------------------
void GPUTimers::StartTimer(ID3D12GraphicsCommandList* pCommandList, UINT i)
{
    if (!m_Timers[i].TimestampQueryInFlight)
    {
        pCommandList->EndQuery(m_pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, i * 2);
    }
}

//--------------------------------------------------------------------------------
void GPUTimers::StopTimer(ID3D12GraphicsCommandList* pCommandList, UINT i)
{
    if (!m_Timers[i].TimestampQueryInFlight)
    {
         pCommandList->EndQuery(m_pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, i * 2 + 1);
    }
    m_Timers[i].TimestampQueryInFlight = true;
}

//--------------------------------------------------------------------------------
float GPUTimers::GetGPUTimeInMS(UINT i)
{
    if (i < m_Timers.size())
    {
        return m_Timers[i].GPUTimeInMS;
    }
    assert(0);
    return 0.f;
}
