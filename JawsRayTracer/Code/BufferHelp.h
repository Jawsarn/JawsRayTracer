#pragma once

#define THREAD_GROUP_SIZE_X 32
#define THREAD_GROUP_SIZE_Y 32

struct PerFrameBuffer
{
	XMMATRIX invView;
	XMMATRIX invProj;
	XMFLOAT2 ScreenDimensions;
	XMFLOAT2 filler;
};


struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCord;
};

struct Ray
{
	XMFLOAT2 Direction;
	XMFLOAT3 Color;
};