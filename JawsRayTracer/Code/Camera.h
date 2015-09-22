#pragma once
#include <DirectXMath.h>

using namespace DirectX;


class Camera
{
public:
	Camera();
	~Camera();
	void LookAt(XMFLOAT3 p_EyePos, XMFLOAT3 p_FocusPos, XMFLOAT3 p_UpPos);
	void LookTo(XMFLOAT3 p_EyePos, XMFLOAT3 p_EyeDir, XMFLOAT3 p_UpPos);
	void SetPerspective(float p_Angle, float p_Width, float p_Height, float p_Near, float p_Far);
	void SetOrtogonal(float p_Width, float p_Height, float p_Near, float p_Far);

	XMMATRIX GetViewProj();
	XMMATRIX GetInvViewProj();
	XMMATRIX GetView();
	XMMATRIX GetProj();
	XMMATRIX GetInvView();
	XMMATRIX GetInvProj();

private:
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;
	
};

