#pragma once

#define THREAD_GROUP_SIZE_X 32
#define THREAD_GROUP_SIZE_Y 32

struct PerFrameBuffer
{
	//XMMATRIX invView;
	XMMATRIX invProj;
	XMFLOAT2 ScreenDimensions;
	unsigned int NumOfVertices;
	unsigned int filler;
};


struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	//XMFLOAT2 TexCord;
	XMFLOAT3 Color;
};

struct Ray
{
	XMFLOAT3 Position;
	XMFLOAT3 Direction;
	XMFLOAT3 Color;
};