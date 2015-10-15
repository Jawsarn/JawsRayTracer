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

	XMFLOAT3 CameraPosition;
	unsigned int NumOfPointLights;
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
	XMFLOAT3 Color;
};

struct ColorData
{
	float w;
	float u;
	float v;
	int index;
	XMFLOAT3 hitPos;
	float filler;
	XMFLOAT3 direction;
	float filler2;
};

struct PointLight
{
	PointLight()
	{

	}
	PointLight(XMFLOAT3 pPos, float pRange, XMFLOAT3 pColor)
	{
		Position = pPos;
		Range = pRange;
		Color = pColor;
	}
	XMFLOAT3 Position;
	float Range;
	XMFLOAT3 Color;
};