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
	unsigned int NumOfPointLights;
	unsigned int filler1;
	unsigned int filler2;
	unsigned int filler3;
};


struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCord;
	//XMFLOAT3 Color;
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

struct ColorData
{
	float distance;
	float w;
	float u;
	float v;
	int index;
};

struct PointLight
{
	XMFLOAT3 Position;
	float Range;
	XMFLOAT3 Color;
};