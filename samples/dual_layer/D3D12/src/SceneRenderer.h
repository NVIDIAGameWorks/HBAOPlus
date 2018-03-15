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
#include "D3D12Mesh.h"
#include "MeshRenderer.h"

class Camera;
class Scene;

class SceneRenderer
{
public:
    bool InitializeWithScene(ID3D12Device* device, Scene& scene);
    void RenderLayer(ID3D12GraphicsCommandList* commandList, uint32_t layer, const Camera& camera);
    static const uint32_t kLayersCount = 2;
    
private:
    std::vector<D3D12Mesh> mMeshes[kLayersCount];
    MeshRenderer mMeshRenderer;
};

