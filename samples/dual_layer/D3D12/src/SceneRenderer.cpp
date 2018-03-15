/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "SceneRenderer.h"
#include <Scene.h>
#include <Camera.h>

bool SceneRenderer::InitializeWithScene(ID3D12Device* device, Scene& scene)
{
    mMeshRenderer.Initialize(device);

    for (auto& sceneNode : scene.GetNodes())
    {
        if (sceneNode.layer < kLayersCount)
        {
            mMeshes[sceneNode.layer].emplace_back();

            D3D12Mesh& mesh = mMeshes[sceneNode.layer].back();

            mesh.InitializeFromMesh(device, *sceneNode.mesh);
            mesh.mMatWorld = sceneNode.matWorld;
        }
    }
    return true;
}

void SceneRenderer::RenderLayer(ID3D12GraphicsCommandList* commandList, uint32_t layer, const Camera& camera)
{
    mMeshRenderer.RenderMeshes(commandList, mMeshes[layer], camera);
}