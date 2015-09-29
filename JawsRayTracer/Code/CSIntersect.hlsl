


struct Ray
{
	float2 Position;
	float3 Direction;
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
	uint NumOfVertices;
	uint filler;
};

RWTexture2D<float4> output : register(u0);
StructuredBuffer<Ray> Rays : register(t0);
StructuredBuffer<Vertex> Vertices : register(t1);

//http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
//kinda same https://courses.cs.washington.edu/courses/cse457/07sp/lectures/triangle_intersection.pdf


bool CheckCollision(Ray pRay, uint startIndex, inout float t, inout float u, inout float v)
{
	Vertex A = Vertices[startIndex];
	Vertex B = Vertices[startIndex + 1];
	Vertex C = Vertices[startIndex + 2];

	float3 AtoB = B.Position - A.Position;
	float3 AtoC = C.Position - A.Position;
	float3 pVec = cross(pRay.Direction, AtoC);
	float det = dot(AtoB, pVec);
	//if culling comment in
	//if(det < kEpsilon) return false

	if (abs(det) < 0.001f)
	{
		return false;
	}

	float invDet = 1 / det;

	float3 tVec = float3(pRay.Position, 0) - A.Position;
	u = dot(tVec, pVec) * invDet;
	if (u < 0 || u > 1)
	{
		return false;
	}

	float3 qVec = cross(tVec, AtoB);
	v = dot(pRay.Direction, qVec) * invDet;
	if (v < 0 || u + v > 1)
	{
		return false;
	}

	t = dot(AtoC, qVec)*invDet;

	return true;
}


[numthreads(32, 32, 1)]
void CS( uint3 threadID : SV_DispatchThreadID )
{
	//warning might not work if we're out of scope with a thread, since it will write to same slot as next row
	unsigned int index = threadID.y * ScreenDimensions.x + threadID.x;

	Ray myRay = Rays[index];

	float ClosestIndex = 0.0f;

	//check closest triangle 
	for (uint i = 0; i < NumOfVertices; i += 3)
	{
		float t, u, v;
		if (CheckCollision(myRay, i, t, u, v))
		{
			ClosestIndex = 1.0f;
		}
	}


	output[threadID.xy] = float4(ClosestIndex, ClosestIndex, ClosestIndex,0);
}