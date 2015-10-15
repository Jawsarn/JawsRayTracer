//--------------------------------------------------------------------------------------
// BasicCompute.fx
// Direct3D 11 Shader Model 5.0 Demo
// Copyright (c) Stefan Petersson, 2012
//--------------------------------------------------------------------------------------

/*
Create rays from screencords with help of view proj matrices

*/



struct Ray
{
	float3 Position;
	float3 Direction;
	float3 Color;
	int lastVertexIndex;
	float reflectionFactor;
};

cbuffer PerFrameBuffer : register(b0)
{
	matrix InvView;
	matrix Proj;
	float2 ScreenDimensions; //width height
	uint NumOfVertices;
	uint NumOfSpheres;
	float3 CameraPosition;
	uint NumOfPointLights;
};



RWStructuredBuffer<Ray> outputRays : register(u0);

[numthreads(32, 32, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
	//warning might not work if we're out of scope with a thread, since it will write to same slot as next row
	unsigned int outIndex = threadID.y * ScreenDimensions.x + threadID.x;

	//get clobal cords
	uint2 t_GlobalCord = threadID.xy;

	//get the screen cords
	float2 t_ScreenPixelOffset = float2(2.0f, -2.0f) / ScreenDimensions;
	float2 t_ScreenPos = (float2(t_GlobalCord) + 0.5f) * t_ScreenPixelOffset + float2(-1.0f, 1.0f);

	//create ray in screenspace, this should now only have to be multiplied with a view matrix to get to view space?
	float2 screenSpaceRay = float2(t_ScreenPos.x / Proj._11, t_ScreenPos.y / Proj._22);

	//near plane
	float t_Near = Proj._43;

	//either multiplie with inverse view here, or multiply the vertices with view?
	float3 t_ViewSpacePos = mul(float4(t_ScreenPos, 2.0f, 1.0f), InvView).xyz;

	float3 viewSpaceRay = mul(float4(screenSpaceRay, 1.0f, 0.0f), InvView).xyz;

	Ray t_Ray;
	//t_Ray.Direction = screenSpaceRay;
	t_Ray.Position = t_ViewSpacePos;
	t_Ray.Direction = normalize(viewSpaceRay);
	t_Ray.Color = float3(0, 0, 0);
	t_Ray.lastVertexIndex = -1;
	t_Ray.reflectionFactor = 1.0f;

	outputRays[outIndex] = t_Ray;
}

/* Reference code for creating rays





float3 ConstructViewFromDepth(float2 p_ScreenPos, float p_Depth)
{
	float2 screenSpaceRay = float2(p_ScreenPos.x / Projection._11, p_ScreenPos.y / Projection._22);

		return float3(float2(screenSpaceRay* p_Depth), p_Depth);
}

PixelData GetPixelData(uint2 p_GlobalCord)
{
	PixelData o_Output;

	//get depth normal data
	float t_Depth = g_Normal_Depth[p_GlobalCord].w;
	float3 t_Normal = g_Normal_Depth[p_GlobalCord].xyz;

	//
	float2 t_ScreenPixelOffset = float2(2.0f, -2.0f) / ScreenDimensions;
	float2 t_ScreenPos = (float2(p_GlobalCord)+0.5f)* t_ScreenPixelOffset + float2(-1.0f, 1.0f); //not sure if this is ok?

	t_Depth = Projection._43 / (t_Depth - Projection._33); //getting depth from projection matrix,

	o_Output.PositionView = ConstructViewFromDepth(t_ScreenPos, t_Depth);
	o_Output.NormalView = mul(float4(t_Normal, 0), View).xyz;

	return o_Output;
}

void CalculateDepth(uint p_GroupIndex, PixelData p_Data)
{
	//setting Max to min distance, and Min to max distance for checking
	float t_TestMinZ = CamNearFar.y;
	float t_TestMaxZ = CamNearFar.x;

	float t_Depth = p_Data.PositionView.z;

	bool t_ValidPixel = t_Depth >= CamNearFar.x && t_Depth < CamNearFar.y; //to check if the pixel is inside the camera range, if nothing is drawn depth is 0, => a large box is something else is far away
	if (t_ValidPixel)
	{
		t_TestMinZ = min(t_TestMinZ, t_Depth);
		t_TestMaxZ = max(t_TestMaxZ, t_Depth);
	}

	if (p_GroupIndex == 0) //initialize variables
	{
		g_MinDepth = 0x7F7FFFFF;
		g_MaxDepth = 0;
		g_VisibleLightCount = 0;
	}

	GroupMemoryBarrierWithGroupSync();

	if (t_TestMaxZ >= t_TestMinZ) //to check if it is a valid data
	{
		InterlockedMax(g_MaxDepth, asuint(t_TestMaxZ));
		InterlockedMin(g_MinDepth, asuint(t_TestMinZ));
	}

	GroupMemoryBarrierWithGroupSync();
}

*/