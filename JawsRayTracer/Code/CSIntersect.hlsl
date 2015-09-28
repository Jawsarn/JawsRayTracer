


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


int CheckCollision(Ray pRay, uint startIndex)
{
	Vertex A = Vertices[startIndex];
	Vertex B = Vertices[startIndex + 1];
	Vertex C = Vertices[startIndex + 2];

	//compute plane normal
	float3 AtoB = B.Position - A.Position;
	float3 AtoC = C.Position - A.Position;

	//get the normal, not normalized
	float3 norm = cross(AtoB, AtoC);

	//area for triangle, its usually AtoB*AtoC*sin(a) but cross product fixes this!
	float area = length(norm) / 2.0f;


	//find P 
	float NormalDotRayDir = dot(norm, pRay.Direction);

	float d = dot(norm, A.Position);

	float t = dot(norm, pRay.Position) + d / NormalDotRayDir;



	float3 P = float3(pRay.Position, 0) + t* pRay.Direction; //middle


	float3 PerVec; //vector perpendicular to triangle's plane



	float result = 0.0f;

	float3 edge0 = A.Position - B.Position;
	float3 vp0 = P - A.Position;
	PerVec = cross(edge0, vp0);
	result = min(result, dot(norm, C.Position));


	float3 edge1 = C.Position - B.Position;
	float3 vp1 = P - B.Position;
	PerVec = cross(edge1, vp1);
	float u = (length(C.Position) / 2.0f) / area;
	result = min(result, dot(norm, C.Position));

	float3 edge2 = A.Position - C.Position;
	float3 vp2 = P - C.Position;
	PerVec = cross(edge2, vp2);
	float w = (length(C.Position) / 2.0f) / area;
	result = min(result, dot(norm, C.Position));

	if (result == 0)
	{
		return startIndex;
	}


	return -1;
}


[numthreads(32, 32, 1)]
void CS( uint3 threadID : SV_DispatchThreadID )
{
	//warning might not work if we're out of scope with a thread, since it will write to same slot as next row
	unsigned int index = threadID.y * ScreenDimensions.x + threadID.x;

	Ray myRay = Rays[index];

	int ClosestIndex = -1;

	//check closest triangle 
	for (uint i = 0; i < NumOfVertices; i += 3)
	{
		ClosestIndex = max(ClosestIndex, CheckCollision(myRay, i));
	}

	Vertex ver = Vertices[0];


	output[threadID.xy] = float4(myRay.Position.x, myRay.Position.y,0,0);
}