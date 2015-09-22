#include "Camera.h"



Camera::Camera()
{
}


Camera::~Camera()
{
}

void Camera::LookAt(XMFLOAT3 p_EyePos, XMFLOAT3 p_FocusPos, XMFLOAT3 p_UpPos)
{
	XMVECTOR p_VecEye = XMLoadFloat3(&p_EyePos);
	XMVECTOR p_VecFocus = XMLoadFloat3(&p_FocusPos);
	XMVECTOR p_VecUp = XMLoadFloat3(&p_UpPos);

	XMMATRIX p_View = XMMatrixLookAtLH(p_VecEye, p_VecFocus, p_VecUp);

	XMStoreFloat4x4(&m_View, p_View);
}

void Camera::LookTo(XMFLOAT3 p_EyePos, XMFLOAT3 p_EyeDir, XMFLOAT3 p_UpPos)
{
	XMVECTOR p_VecEye = XMLoadFloat3(&p_EyePos);
	XMVECTOR p_VecEyeDir = XMLoadFloat3(&p_EyeDir);
	XMVECTOR p_VecUp = XMLoadFloat3(&p_UpPos);

	XMMATRIX p_View = XMMatrixLookToLH(p_VecEye, p_VecEyeDir, p_VecUp);

	XMStoreFloat4x4(&m_View, p_View);
}

void Camera::SetPerspective(float p_Angle, float p_Width, float p_Height, float p_Near, float p_Far)
{
	XMMATRIX p_Proj = XMMatrixPerspectiveFovLH(p_Angle, p_Width/p_Height, p_Near, p_Far);
	XMStoreFloat4x4(&m_Proj, p_Proj);
}

void Camera::SetOrtogonal(float p_Width, float p_Height, float p_Near, float p_Far)
{
	XMMATRIX p_Proj = XMMatrixOrthographicLH(p_Width, p_Height, p_Near, p_Far);
	XMStoreFloat4x4(&m_Proj, p_Proj);
}

XMMATRIX Camera::GetViewProj()
{
	return XMMatrixMultiply(GetView(), GetProj());
}

XMMATRIX Camera::GetInvViewProj()
{
	XMMATRIX t_Mat = GetViewProj();
	return XMMatrixInverse(nullptr, t_Mat);
}

XMMATRIX Camera::GetView()
{
	return XMLoadFloat4x4(&m_View);
}

XMMATRIX Camera::GetProj()
{
	return XMLoadFloat4x4(&m_Proj);
}


XMMATRIX Camera::GetInvView()
{
	return XMMatrixInverse(nullptr, GetView());
}

XMMATRIX Camera::GetInvProj()
{
	return XMMatrixInverse(nullptr, GetProj());
}

