#pragma once

#define THREAD_GROUP_SIZE_X 32
#define THREAD_GROUP_SIZE_Y 32

struct PerFrameBuffer
{
	XMMATRIX InvView;
	XMMATRIX Proj;
	XMFLOAT2 ScreenDimensions;
	unsigned int NumOfVertices;
	unsigned int NumOfSpheres;
};


struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	//XMFLOAT2 TexCord;
	XMFLOAT3 Color;
};

struct Sphere
{
	XMFLOAT3 Position;
	float Radius;
	XMFLOAT3 Color;
};

struct Ray
{
	XMFLOAT3 Position;
	XMFLOAT3 Direction;
};