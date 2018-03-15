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
#include "Mesh.h"
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;

class Scene
{
public:
    struct Node
    {
        std::shared_ptr<Mesh> mesh;
        XMMATRIX matWorld;
        uint32_t layer;
    };

	enum class ESceneType
	{
		kShaderBall = 0,
		kCustom
	};
    using Nodes = std::vector<Node>;

    bool InitializeDefault();

    const Nodes& GetNodes() const { return mNodes; }

private:
    Nodes mNodes;
};

