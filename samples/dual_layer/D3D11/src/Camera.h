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

#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:
    Camera();

    union
    {
        struct
        {
            XMMATRIX mMatView;
            XMMATRIX mMatViewProj;
        };
        float PerFrameData[32];
    };
    XMMATRIX mMatProj;

    void InitProjectionMatrix(uint32_t width, uint32_t height, float zNear = 0.01f, float zFar = 10000.0f);
    void MoveForward(float dt);
    void MoveRight(float dt);
    void AdvanceAngles(float yaw, float pitch);

    void UpdateViewMatrix();

    XMVECTOR mPosition;
    XMVECTOR mViewVector;
};

