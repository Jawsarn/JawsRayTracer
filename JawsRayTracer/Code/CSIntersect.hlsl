


struct Ray
{
	float2 Direction;
	float3 Color;
};

struct Vertex
{
	float3 Position;
	float3 Normal;
	float2 TexCord;
};

cbuffer PerFrameBuffer : register(b0)
{
	matrix View;
	matrix Proj;
	float2 ScreenDimensions; //width height
	float2 filler;
};

RWTexture2D<float4> output : register(u0);
StructuredBuffer<Ray> Rays : register(t0);
StructuredBuffer<Vertex> Vertices : register(t1);


[numthreads(32, 32, 1)]
void CS( uint3 threadID : SV_DispatchThreadID )
{
	//warning might not work if we're out of scope with a thread, since it will write to same slot as next row
	unsigned int index = threadID.y * ScreenDimensions.x + threadID.x;

	Ray myRay = Rays[index];


	output[threadID.xy] = float4(1,0,0,0);
}