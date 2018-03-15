/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "Scene.h"


bool Scene::InitializeDefault()
{
    std::shared_ptr<Mesh> actorMesh = Mesh::LoadFromFile("..\\Media\\Meshes\\shaderBall.obj");
    std::shared_ptr<Mesh> cubeMesh = Mesh::LoadFromFile("..\\Media\\Meshes\\cube.obj");

    Node node;
    node.mesh = actorMesh;
    node.layer = 1;

    for (uint32_t i = 0; i < 4; ++i)
    {
        node.matWorld = XMMatrixTranslation((i % 2) * 16.0f - 8.0f, (i / 2) * 16.0f - 8.0f, 0.0f);
        mNodes.push_back(node);
    }

    node.mesh = cubeMesh;
    node.matWorld = XMMatrixIdentity();
    node.layer = 0;

    mNodes.push_back(node);


    return true;
}
