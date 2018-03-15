/*
* Copyright (c) 2008-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "Camera.h"

Camera::Camera()
{
    InitProjectionMatrix(1280, 720);

    mPosition = { -5.0f, -5.0f, 3.0f };
    mViewVector = XMVector3Normalize({ -4.0f, -4.0f, -2.0f });
    UpdateViewMatrix();
}

void Camera::InitProjectionMatrix(uint32_t width, uint32_t height, float zNear, float zFar)
{
    mMatProj = XMMatrixPerspectiveFovRH(XMConvertToRadians(60.0f), (float)width / (float)height, zNear, zFar);
    UpdateViewMatrix();
}

void Camera::MoveForward(float dt)
{
    mPosition += mViewVector * dt;
    UpdateViewMatrix();
}

void Camera::MoveRight(float dt)
{
    mPosition += XMMatrixTranspose(mMatView).r[0] * dt;
    UpdateViewMatrix();
}

void Camera::AdvanceAngles(float yaw, float pitch)
{
    XMMATRIX matBasisVecs = XMMatrixTranspose(mMatView);

    XMMATRIX matRot = XMMatrixRotationY(yaw);// *XMMatrixRotationX(pitch);

    mViewVector = XMVector3Rotate(mViewVector, XMQuaternionRotationAxis(matBasisVecs.r[1], yaw));
    mViewVector = XMVector3Rotate(mViewVector, XMQuaternionRotationAxis(matBasisVecs.r[0], pitch));
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    mMatView = XMMatrixLookAtRH(mPosition, mPosition + mViewVector, { 0.0f, 0.0f, 1.0f });
    mMatViewProj = XMMatrixMultiply(mMatView, mMatProj);
}
