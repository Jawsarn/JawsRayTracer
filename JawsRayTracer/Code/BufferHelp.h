#pragma once

struct PerFrameBuffer
{
	XMMATRIX invViewProj;
	float screenWidth;
	float screenHeight;
};


struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCord;
};