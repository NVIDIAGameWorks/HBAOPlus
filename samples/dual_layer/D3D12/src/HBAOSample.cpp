/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "HBAOSample.h"
#include "imgui/imgui.h"
#include <Windows.h>

// Library link for HBAO+
#ifdef _WIN64
#pragma comment(lib, "GFSDK_SSAO_D3D12.win64.lib")
#else
#pragma comment(lib, "GFSDK_SSAO_D3D12.win32.lib")
#endif

#pragma comment(lib, "D3dcompiler.lib")

void HBAOSample::RenderUI()
{
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

        ImGui::Text("HBAO+ %d.%d.%d.%d", Version.Major, Version.Minor, Version.Branch, Version.Revision);

        ImGui::Text("AO: %.2f ms/frame", mFrameTimeMs);

        float radius = mAOParameters.Radius;
        ImGui::DragFloat("Radius", &radius, 0.05f, 0.0f, 100.0f);
        if (radius != mAOParameters.Radius)
        {
            mAOParameters.Radius = radius;
            int stop = 0;
            stop = stop;
        }
        ImGui::DragFloat("PowerExponent", &mAOParameters.PowerExponent, 0.05f, 1.f, 8.f);
        ImGui::DragFloat("Bias", &mAOParameters.Bias, 0.001f, 0, 0.5f);

        ImGui::DragFloat("SmallScaleAO", &mAOParameters.SmallScaleAO, 0.01f, 0.f, 2.f);
        ImGui::DragFloat("LargeScaleAO", &mAOParameters.LargeScaleAO, 0.01f, 0.f, 2.f);

        bool BlurEnabled = mAOParameters.Blur.Enable ? true : false;
        ImGui::Checkbox("Blur.Enable", &BlurEnabled);
        mAOParameters.Blur.Enable = BlurEnabled;

        ImGui::DragFloat("Blur.Sharpness", &mAOParameters.Blur.Sharpness, 0.f, 0.f, 32.0f);

        {
            const char* listbox_items[] = { "GFSDK_SSAO_FP16_VIEW_DEPTHS", "GFSDK_SSAO_FP32_VIEW_DEPTHS" };
            int listbox_item_current = (int)mAOParameters.DepthStorage;
            ImGui::ListBox("DepthStorage", &listbox_item_current, listbox_items, ARRAYSIZE(listbox_items), 2);
            mAOParameters.DepthStorage = (GFSDK_SSAO_DepthStorage)(listbox_item_current);
        }

        {
            bool DoubleLayerEnabled = mAOParameters.EnableDualLayerAO ? true : false;
            ImGui::Checkbox("DualLayerAO", &DoubleLayerEnabled);
            mAOParameters.EnableDualLayerAO = DoubleLayerEnabled;
        }
        ImGui::End();
    }

    // Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (show_test_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
        ImGui::ShowTestWindow(&show_test_window);
    }

    ImGui::Render();

    auto& io = ImGui::GetIO();

    static float oldMouseX = io.MousePos.x;
    static float oldMouseY = io.MousePos.y;

    if (io.MouseClicked[0])
    {
        oldMouseX = io.MousePos.x;
        oldMouseY = io.MousePos.y;
    }

    if (!io.WantCaptureMouse && io.MouseDown[0])
    {
        float mouseDeltaX = io.MousePos.x - oldMouseX;
        oldMouseX = io.MousePos.x;
        float mouseDeltaY = io.MousePos.y - oldMouseY;
        oldMouseY = io.MousePos.y;

        const float kRotationDelta = 0.001f;
        mCamera.AdvanceAngles(-mouseDeltaX * kRotationDelta, -mouseDeltaY * kRotationDelta);
    }

    const float kMoveDelta = 0.01f;
    if (io.KeysDown['W'] || io.KeysDown[VK_UP])
    {
        mCamera.MoveForward(kMoveDelta);
    }
    if (io.KeysDown['S'] || io.KeysDown[VK_DOWN])
    {
        mCamera.MoveForward(-kMoveDelta);
    }

    if (io.KeysDown['D'] || io.KeysDown[VK_RIGHT])
    {
        mCamera.MoveRight(kMoveDelta);
    }
    if (io.KeysDown['A'] || io.KeysDown[VK_LEFT])
    {
        mCamera.MoveRight(-kMoveDelta);
    }
}

void HBAOSample::InitializeHBAOParameters()
{
    mAOParameters = {};

    mAOParameters.Radius = 2.f;
    mAOParameters.Bias = 0.2f;
    mAOParameters.PowerExponent = 2.f;
    mAOParameters.Blur.Enable = true;
    mAOParameters.Blur.Sharpness = 32.f;
    mAOParameters.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
    mAOParameters.DepthStorage = GFSDK_SSAO_FP32_VIEW_DEPTHS;
    mAOParameters.EnableDualLayerAO = true;
}