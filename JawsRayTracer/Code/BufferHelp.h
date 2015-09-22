#pragma once

struct PerFrameBuffer
{
	XMMATRIX invViewProj;
	float screenWidth;
	float screenHeight;
	XMFLOAT2 filler;
};


struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCord;
};