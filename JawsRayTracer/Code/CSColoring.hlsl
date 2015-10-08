
struct Ray
{
	float3 Position;
	float3 Direction;
};

struct Vertex
{
	float3 Position;
	float3 Normal;
	float2 TexCord;
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
	float3 Position;
	float Range;
	float3 Color;
};

cbuffer PerFrameBuffer : register(b0)
{
	matrix View;
	matrix Proj;
	float2 ScreenDimensions; //width height
	uint NumOfVertices;
	uint NumOfSpheres;
	uint NumOfPointLights;
	uint3 filler;
};

RWTexture2D<float4> output : register(u0);
StructuredBuffer<Ray> Rays : register(t0);
StructuredBuffer<Vertex> Vertices : register(t1);
StructuredBuffer<ColorData> ColorDatas : register(t2);
StructuredBuffer<PointLight> PointLights : register(t3);
Texture2D TextureOne : register(t4);

SamplerState Sampler : register(s0);

static const float kEpsilon = 1e-8;

bool CheckTriangleCollision(Ray pRay, uint startIndex, out float t, out float u, out float v)
{
	Vertex A = Vertices[startIndex];
	Vertex B = Vertices[startIndex + 1];
	Vertex C = Vertices[startIndex + 2];

	float3 AtoB = B.Position - A.Position;
	float3 AtoC = C.Position - A.Position;

	float3 pVec = cross(pRay.Direction, AtoC);
	float det = dot(AtoB, pVec);

	//if culling comment in
	/*if (det < kEpsilon)
	{
	return false;
	}*/

	if (abs(det) < kEpsilon)
	{
		return false;
	}

	float invDet = 1 / det;

	float3 tVec = pRay.Position - A.Position;

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
	if (t < 0)
	{
		return false;
	}

	return true;
}



[numthreads(32, 32, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
	unsigned int index = threadID.y * ScreenDimensions.x + threadID.x;

	ColorData tColData = ColorDatas[index];
	Ray tRay = Rays[index];
	
	float3 finalColor = float3(0, 0, 0);

	if (tColData.index > -1)
	{
		Vertex v0 = Vertices[tColData.index];
		Vertex v1 = Vertices[tColData.index + 1];
		Vertex v2 = Vertices[tColData.index + 2];

		//create normal from triangle
		//float3 normal = normalize(cross(v1.Position - v0.Position, v2.Position - v0.Position));
		//float3 hitPos = tRay.Position + tRay.Direction*tColData.distance;


		float2 texCords = v0.TexCord * tColData.w + v1.TexCord * tColData.u + v2.TexCord * tColData.v;
		finalColor = TextureOne.SampleLevel(Sampler, texCords, 0.0f);



		////for each light, we look if any vertices block it
		//for (uint i = 0; i < NumOfPointLights; i++)
		//{
			////we create a ray from our position and the target
			//Ray tToLightRay;
			//PointLight tLight = PointLights[0];
			//tToLightRay.Direction = normalize(tLight.Position - hitPos);
			//tToLightRay.Position = hitPos;

			//bool tInteresected = false;
			////check if we're going from the face outward
			//if (dot(tToLightRay.Direction, normal) > 0)
			//{
			//	for (uint k = 0; k < NumOfVertices; k+=3)
			//	{
			//		float t, u, v;
			//		if (!tInteresected && CheckTriangleCollision(tToLightRay, k, t, u,v))
			//		{
			//			finalColor = float3(0, 0, 0);
			//		}
			//	}
			//}
		//}

		////new ray here
		

		////reflect direction from ray by normal
		//float3 newDir = reflect(tRay.Direction, normal);


	}

	output[threadID.xy] = float4(finalColor , 0);
}