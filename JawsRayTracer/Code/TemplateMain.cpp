//--------------------------------------------------------------------------------------
// File: TemplateMain.cpp
//
// BTH-D3D-Template
//
// Copyright (c) Stefan Petersson 2013. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"

#include "ComputeHelp.h"
#include "D3D11Timer.h"
#include <DirectXMath.h>
#include "Camera.h"
#include "BufferHelp.h"
#include "DDSTextureLoader.h"

/*	DirectXTex library - for usage info, see http://directxtex.codeplex.com/
	
	Usage example (may not be the "correct" way, I just wrote it in a hurry):

	DirectX::ScratchImage img;
	DirectX::TexMetadata meta;
	ID3D11ShaderResourceView* srv = nullptr;
	if(SUCCEEDED(hr = DirectX::LoadFromDDSFile(_T("C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Samples\\Media\\Dwarf\\Armor.dds"), 0, &meta, img)))
	{
		//img loaded OK
		if(SUCCEEDED(hr = DirectX::CreateShaderResourceView(g_Device, img.GetImages(), img.GetImageCount(), meta, &srv)))
		{
			//srv created OK
		}
	}
*/
#include <DirectXTex.h>

#if defined( DEBUG ) || defined( _DEBUG )
#pragma comment(lib, "DirectXTexD.lib")
#else
#pragma comment(lib, "DirectXTex.lib")
#endif

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE				g_hInst					= NULL;  
HWND					g_hWnd					= NULL;

IDXGISwapChain*         g_SwapChain				= NULL;
ID3D11Device*			g_Device				= NULL;
ID3D11DeviceContext*	g_DeviceContext			= NULL;

ID3D11UnorderedAccessView*  g_BackBufferUAV		= NULL;  // compute output

ComputeWrap*			g_ComputeSys			= NULL;
ComputeShader*			g_CSCreateRays			= NULL;
ComputeShader*			g_CSIntersect			= NULL;
ComputeShader*			g_CSColoring			= NULL;

D3D11Timer*				g_Timer					= NULL;
Camera*					g_Camera				= NULL;

ComputeBuffer*			g_RayBuffer				= NULL;
ComputeBuffer*			g_VertexBuffer			= NULL;
ComputeBuffer*			g_SphereBuffer			= NULL;
ComputeBuffer*			g_ColorDataBuffer		= NULL;
ComputeBuffer*			g_PointLightBuffer		= NULL;
ID3D11Buffer*			g_PerFrameBuffer		= NULL;

ID3D11ShaderResourceView*	g_TextureOne			= NULL;


ID3D11SamplerState*			g_SamplerStateWrap = NULL;

UINT					g_NumOfVertices			= 0;
UINT					g_NumOfSpheres			= 0;
UINT					g_NumOfPointLights		= 0;
float					dt						= 0;
XMFLOAT2				g_LastMousePos			= XMFLOAT2(0, 0);


int g_Width, g_Height;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT             InitWindow( HINSTANCE hInstance, int nCmdShow );
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT				Render(float deltaTime);
HRESULT				Update(float deltaTime);
HRESULT				InitializeBuffers();
HRESULT				InitializeSampler();

char* FeatureLevelToString(D3D_FEATURE_LEVEL featureLevel)
{
	if(featureLevel == D3D_FEATURE_LEVEL_11_0)
		return "11.0";
	if(featureLevel == D3D_FEATURE_LEVEL_10_1)
		return "10.1";
	if(featureLevel == D3D_FEATURE_LEVEL_10_0)
		return "10.0";

	return "Unknown";
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT Init()
{
	HRESULT hr = S_OK;;

	RECT rc;
	GetClientRect( g_hWnd, &rc );
	g_Width = rc.right - rc.left;;
	g_Height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverType;

	D3D_DRIVER_TYPE driverTypes[] = 
	{
		//D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		//D3D_DRIVER_TYPE_REFERENCE,
		
	};
	UINT numDriverTypes = sizeof(driverTypes) / sizeof(driverTypes[0]);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof(sd) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_Width;
	sd.BufferDesc.Height = g_Height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevelsToTry[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL initiatedFeatureLevel;

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			driverType,
			NULL,
			createDeviceFlags,
			featureLevelsToTry,
			ARRAYSIZE(featureLevelsToTry),
			D3D11_SDK_VERSION,
			&sd,
			&g_SwapChain,
			&g_Device,
			&initiatedFeatureLevel,
			&g_DeviceContext);

		if( SUCCEEDED( hr ) )
		{
			char title[256];
			sprintf_s(
				title,
				sizeof(title),
				"BTH - Direct3D 11.0 Template | Direct3D 11.0 device initiated with Direct3D %s feature level",
				FeatureLevelToString(initiatedFeatureLevel)
			);
			SetWindowTextA(g_hWnd, title);

			break;
		}
	}
	if( FAILED(hr) )
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer;
	hr = g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer );
	if( FAILED(hr) )
		return hr;

	// create shader unordered access view on back buffer for compute shader to write into texture
	hr = g_Device->CreateUnorderedAccessView( pBackBuffer, NULL, &g_BackBufferUAV );

	//create helper sys and compute shader instance
	g_ComputeSys = new ComputeWrap(g_Device, g_DeviceContext);
	g_CSCreateRays = g_ComputeSys->CreateComputeShader(_T("CSCreateRays.hlsl"), NULL, "CS", NULL);
	g_CSIntersect = g_ComputeSys->CreateComputeShader(_T("CSIntersect.hlsl"), NULL, "CS", NULL);
	g_CSColoring = g_ComputeSys->CreateComputeShader(_T("CSColoring.hlsl"), NULL, "CS", NULL);
	g_Timer = new D3D11Timer(g_Device, g_DeviceContext);

	//create camera
	g_Camera = new Camera();
	g_Camera->LookTo(XMFLOAT3(0, 0, -10), XMFLOAT3(0, 0, 1), XMFLOAT3(0, 1, 0));
	g_Camera->SetPerspective(PI / 4.0f, (float)g_Width, (float)g_Height, 0.1f, 10000.0f);

	//create textuers
	hr = CreateDDSTextureFromFile(g_Device, L"Jaws.dds", nullptr, &g_TextureOne);
	if (FAILED(hr))
		return hr;

	hr = InitializeBuffers();
	if (FAILED(hr))
		return hr;

	hr = InitializeSampler();
	if (FAILED(hr))
		return hr;


	return S_OK;
}

HRESULT InitializeSampler()
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC; // D3D11_FILTER_ANISOTROPIC  D3D11_FILTER_MIN_MAG_MIP_LINEAR
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX; //D3D11_FLOAT32_MAX
	sampDesc.MaxAnisotropy = 16; //why not max it out when we can?
	hr = g_Device->CreateSamplerState(&sampDesc, &g_SamplerStateWrap);
	if (FAILED(hr))
		return hr;

	g_DeviceContext->CSSetSamplers(0, 1, &g_SamplerStateWrap);

	return hr;
}

HRESULT InitializeBuffers()
{
	HRESULT hr = S_OK;


	//Create vertex buffer
	Vertex t_Vertices[] =
	{
		{	XMFLOAT3(-0.25f,0,5.0f), XMFLOAT3(0,0,0), XMFLOAT2(0.0f,0) },
		{	XMFLOAT3(0.25f,0,5.0f), XMFLOAT3(0,0,0), XMFLOAT2(1.0f,0.0f)},
		{	XMFLOAT3(-0.25f,-0.5f,5.0f), XMFLOAT3(0,0,0), XMFLOAT2(0.0f,1.0) },

		{ XMFLOAT3(0.25f,0,5.0f), XMFLOAT3(0,0,0), XMFLOAT2(1.0f,0.0f) },
		{ XMFLOAT3(0.25f,-0,5.0f), XMFLOAT3(0,0,0), XMFLOAT2(1.0f,1.0f) },
		{ XMFLOAT3(-0.25f,-0.5f,5.0f), XMFLOAT3(0,0,0), XMFLOAT2(0.0f,1.0f) },
	};

	g_NumOfVertices = ARRAYSIZE(t_Vertices);
	g_VertexBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Vertex), g_NumOfVertices, true, true, t_Vertices);

	//create Spheres
	Sphere t_Spheres[]
	{
		{ XMFLOAT3(-5, 0, 0), 2.0f,XMFLOAT3(1, 0, 0)},
		{ XMFLOAT3(-5, 0, 5), 0.5f,XMFLOAT3(0, 1, 1) },
	};
	g_NumOfSpheres = ARRAYSIZE(t_Spheres);
	g_SphereBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Sphere), g_NumOfSpheres, true, true, t_Spheres);


	//create ray buffer
	g_RayBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Ray), g_Height*g_Width, true, true, nullptr);

	//create colordata buffer
	g_ColorDataBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(ColorData), g_Height*g_Width, true, true, nullptr);

	PointLight t_PointLights[]
	{
		{XMFLOAT3(0,0,0), 2.0f, XMFLOAT3(1,1,1)},

	};


	g_NumOfPointLights = ARRAYSIZE(t_PointLights);
	//create pointlight buffer
	g_PointLightBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(PointLight), g_NumOfPointLights, true, false, t_PointLights);

	//create per frame buffer
	PerFrameBuffer p_FrameBuffer;
	p_FrameBuffer.InvView = XMMatrixTranspose(g_Camera->GetInvView());
	p_FrameBuffer.Proj = XMMatrixTranspose(g_Camera->GetProj());
	p_FrameBuffer.ScreenDimensions = XMFLOAT2((float)g_Height, (float)g_Width);
	p_FrameBuffer.NumOfVertices = g_NumOfVertices;
	p_FrameBuffer.NumOfSpheres = g_NumOfSpheres;
	p_FrameBuffer.NumOfPointLights = 0;

	g_PerFrameBuffer = g_ComputeSys->CreateConstantBuffer(sizeof(PerFrameBuffer), &p_FrameBuffer,D3D11_USAGE_DYNAMIC ,D3D11_CPU_ACCESS_WRITE);

	return hr;
}

HRESULT Update(float deltaTime)
{
	g_Camera->Update();

	return S_OK;
}

void UpdatePerFrameBuffer()
{
	PerFrameBuffer p_FrameBuffer;
	p_FrameBuffer.InvView = XMMatrixTranspose(g_Camera->GetInvView());
	p_FrameBuffer.Proj = XMMatrixTranspose(g_Camera->GetProj());
	p_FrameBuffer.ScreenDimensions = XMFLOAT2((float)g_Height, (float)g_Width);
	p_FrameBuffer.NumOfVertices = g_NumOfVertices;
	p_FrameBuffer.NumOfSpheres = g_NumOfSpheres;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	PerFrameBuffer* p = nullptr;
	if (SUCCEEDED(g_DeviceContext->Map(g_PerFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
		*(PerFrameBuffer*)MappedResource.pData = p_FrameBuffer;

	g_DeviceContext->Unmap(g_PerFrameBuffer, 0);
	
}


HRESULT Render(float deltaTime)
{
	UpdatePerFrameBuffer();
	//set textures
	//ID3D11UnorderedAccessView* uav[] = { g_BackBufferUAV , g_VertexBuffer->GetUnorderedAccessView()};
	//g_DeviceContext->CSSetUnorderedAccessViews(0, 2, uav, NULL);
	
	ID3D11UnorderedAccessView* uav[] = { g_RayBuffer->GetUnorderedAccessView() };
	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, uav, nullptr);

	//clear SRV
	ID3D11ShaderResourceView* srv[] = { nullptr , nullptr};
	g_DeviceContext->CSSetShaderResources(0, 2, srv);

	//set constant buffers
	g_DeviceContext->CSSetConstantBuffers(0, 1, &g_PerFrameBuffer);

	//set shader
	g_CSCreateRays->Set();

	
	//start time
	g_Timer->Start();

	//calc num of thread groups
	UINT x = ceil((float)g_Width / (float)THREAD_GROUP_SIZE_X);
	UINT y = ceil((float)g_Height / (float)THREAD_GROUP_SIZE_Y);

	//draw call
	g_DeviceContext->Dispatch( x, y, 1 );

	//unset stuff
	g_CSCreateRays->Unset();
	g_CSIntersect->Set();

	//set uav
	ID3D11UnorderedAccessView* uav2[] = { g_ColorDataBuffer->GetUnorderedAccessView() };
	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, uav2, NULL);

	//set srv
	ID3D11ShaderResourceView* srv2[] = { g_RayBuffer->GetResourceView(), g_VertexBuffer->GetResourceView(), g_SphereBuffer->GetResourceView()};
	g_DeviceContext->CSSetShaderResources(0, 3, srv2);


	g_DeviceContext->Dispatch(x, y, 1);




	g_CSIntersect->Unset();
	g_CSColoring->Set();

	//set uav
	ID3D11UnorderedAccessView* uav3[] = { g_BackBufferUAV };
	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, uav3, NULL);

	//set srv
	ID3D11ShaderResourceView* srv3[] = { g_RayBuffer->GetResourceView(), g_VertexBuffer->GetResourceView(), g_ColorDataBuffer->GetResourceView(), g_PointLightBuffer->GetResourceView() ,g_TextureOne };
	g_DeviceContext->CSSetShaderResources(0, 5, srv3);


	g_DeviceContext->Dispatch(x, y, 1);
	

	ID3D11UnorderedAccessView* uav4[] = { nullptr };
	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, uav4, NULL);

	//set srv
	ID3D11ShaderResourceView* srv4[] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	g_DeviceContext->CSSetShaderResources(0, 5, srv4);


	//stop time
	g_Timer->Stop();

	

	//swap backbuffer to front
	if(FAILED(g_SwapChain->Present( 0, 0 )))
		return E_FAIL;

	//debug info
	char title[256];
	sprintf_s(
		title,
		sizeof(title),
		"BTH - DirectCompute DEMO - Dispatch time: %f",
		g_Timer->GetTime()
	);
	SetWindowTextA(g_hWnd, title);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
		return 0;

	if( FAILED( Init() ) )
		return 0;

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

	// Main message loop
	MSG msg = {0};
	while(WM_QUIT != msg.message)
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			__int64 currTimeStamp = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
			dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;

			//render
			Update(dt);
			Render(dt);

			prevTimeStamp = currTimeStamp;
		}
	}

	return (int) msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = 0;
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = _T("BTH_D3D_Template");
	wcex.hIconSm        = 0;
	if( !RegisterClassEx(&wcex) )
		return E_FAIL;

	// Create window
	g_hInst = hInstance; 
	RECT rc = { 0, 0, 800, 800 };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	if(!(g_hWnd = CreateWindow(
							_T("BTH_D3D_Template"),
							_T("BTH - Direct3D 11.0 Template"),
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							rc.right - rc.left,
							rc.bottom - rc.top,
							NULL,
							NULL,
							hInstance,
							NULL)))
	{
		return E_FAIL;
	}

	ShowWindow( g_hWnd, nCmdShow );

	return S_OK;
}

void CheckKeys()
{
	if (GetAsyncKeyState('W') & 0x8000)
	{
		g_Camera->Walk(100.0f*dt);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		g_Camera->Walk(-100.0f*dt);
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		g_Camera->Strafe(-100.0f*dt);
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		g_Camera->Strafe(100.0f*dt);
	}
	if (GetAsyncKeyState('E') & 0x8000)
	{
		g_Camera->HoverY(100.0f*dt);
	}
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		g_Camera->HoverY(-100.0f*dt);
	}
}

void OnMouseMove(WPARAM btnStae, int x, int y)
{
	if (btnStae && MK_LBUTTON != 0)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - g_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - g_LastMousePos.y));

		g_Camera->Pitch(dy);
		g_Camera->RotateY(dx);
	}

	g_LastMousePos.x = static_cast< float >(x);
	g_LastMousePos.y = static_cast< float >(y);
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		CheckKeys();
		switch(wParam)
		{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
		}
		break;

	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}