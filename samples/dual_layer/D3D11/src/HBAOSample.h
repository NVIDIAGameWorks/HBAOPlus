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
#include "Scene.h"
#include "Camera.h"
#include "GFSDK_SSAO.h"

enum GPUTimeId
{
    GPU_TIME_AO,
    NUM_GPU_TIMES
};

class HBAOSample
{
public:
    virtual void Initialize() {};
    virtual void Run() = 0;
    virtual void RenderUI();

protected:
    void                    InitializeHBAOParameters();

    Scene                   mScene;
    Camera                  mCamera;
    uint32_t                mWindowWidth = 1280;
    uint32_t                mWindowHeight = 720;
    float                   mFrameTimeMs;
    GFSDK_SSAO_Parameters   mAOParameters;
};