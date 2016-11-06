#include "GraphicsEngine.h"
#include "ComputeHelp.h"
#include <stdexcept>



GraphicsEngine* GraphicsEngine::m_singleton = nullptr;

GraphicsEngine * GraphicsEngine::GetInstance()
{
    if (m_singleton == nullptr)
        throw std::runtime_error("Non-initialized graphic engine received");

    return m_singleton;
}

void GraphicsEngine::Startup(int p_nCmdShow, WNDPROC p_winProc)
{
    if (m_singleton != nullptr)
        throw std::runtime_error("Startup called multiple times");

    m_singleton = new GraphicsEngine(p_nCmdShow, p_winProc);
}

GraphicsEngine::GraphicsEngine(int p_nCmdShow, WNDPROC p_winProc)
{
    HRESULT hr S_OK;
    
    hr = InitializeWindow(p_nCmdShow, p_winProc);
    if (FAILED(hr))
        throw std::runtime_error("Startup error");
    
    hr = InitializeDirectX();
    if(FAILED(hr))
        throw std::runtime_error("Startup error");

    hr = InitializeBackBuffer();
    if (FAILED(hr))
        throw std::runtime_error("Startup error");

    //create helper sys and compute shader instance
    m_computeWrapper = new ComputeWrap(m_device, m_deviceContext);
    
    hr = InitializeShaders();
    if (FAILED(hr))
        throw std::runtime_error("Startup error");


    hr = InitializeBuffers();
    if (FAILED(hr))
        throw std::runtime_error("Startup error");
}


GraphicsEngine::~GraphicsEngine()
{
}

char* FeatureLevelToString(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel == D3D_FEATURE_LEVEL_11_1)
        return "11.1";
    if (featureLevel == D3D_FEATURE_LEVEL_11_0)
        return "11.0";
    if (featureLevel == D3D_FEATURE_LEVEL_10_1)
        return "10.1";
    if (featureLevel == D3D_FEATURE_LEVEL_10_0)
        return "10.0";

    return "Unknown";
}

char* DriverTypeToString(D3D_DRIVER_TYPE driverType)
{
    if (driverType == D3D_DRIVER_TYPE_HARDWARE)
        return "Hardware";
    if (driverType == D3D_DRIVER_TYPE_REFERENCE)
        return "Reference";
    if (driverType == D3D_DRIVER_TYPE_NULL)
        return "Null";
    if (driverType == D3D_DRIVER_TYPE_SOFTWARE)
        return "Software";
    if (driverType == D3D_DRIVER_TYPE_WARP)
        return "Warp";
        
    return "Unknown";
}

HRESULT GraphicsEngine::InitializeWindow(int p_nCmdShow, WNDPROC p_winProc)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = p_winProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_handleInstance;
    wcex.hIcon = 0;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = _T("BTH_D3D_Template");
    wcex.hIconSm = 0;
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    RECT rc = { 0, 0, 800, 800 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    if (!(m_handleWindow = CreateWindow(
        _T("BTH_D3D_Template"),
        _T("BTH - Direct3D 11.0 Template"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        NULL,
        NULL,
        m_handleInstance,
        NULL)))
    {
        return E_FAIL;
    }

    ShowWindow(m_handleWindow, p_nCmdShow);

    return S_OK;
}

HRESULT GraphicsEngine::InitializeDirectX()
{
    HRESULT hr = S_OK;;

    // Get size of window
    RECT rec;
    GetClientRect(m_handleWindow, &rec);
    m_width = rec.right - rec.left;;
    m_height = rec.bottom - rec.top;

    // Add debug flag
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC swapDesc;
    ZeroMemory(&swapDesc, sizeof(swapDesc));
    swapDesc.BufferCount = 1;
    swapDesc.BufferDesc.Width = m_width;
    swapDesc.BufferDesc.Height = m_height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapDesc.OutputWindow = m_handleWindow;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
    swapDesc.Windowed = TRUE;

    // Get best drivers and featurelevel for run
    D3D_FEATURE_LEVEL initiatedFeatureLevel;
    D3D_FEATURE_LEVEL featureLevelsToTry[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    D3D_DRIVER_TYPE driverType;
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_REFERENCE,
        D3D_DRIVER_TYPE_NULL,
        D3D_DRIVER_TYPE_SOFTWARE,
        D3D_DRIVER_TYPE_WARP
    };

    // Try create swap with driver and feature
    for (UINT driverTypeIndex = 0; driverTypeIndex < ARRAYSIZE(driverTypes); driverTypeIndex++)
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
            &swapDesc,
            &m_swapChain,
            &m_device,
            &initiatedFeatureLevel,
            &m_deviceContext);

        // Set name of window
        if (SUCCEEDED(hr))
        {
            char title[256];
            sprintf_s(
                title,
                sizeof(title),
                "DX Feature level: %s, with driver of %s",
                FeatureLevelToString(initiatedFeatureLevel), DriverTypeToString(driverType)
            );
            SetWindowTextA(m_handleWindow, title);
            break;
        }
    }

    return hr;
}

HRESULT GraphicsEngine::InitializeBackBuffer()
{
    HRESULT hr;
    // Create a render target view
    ID3D11Texture2D* backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
        return hr;

    // create shader unordered access view on back buffer for compute shader to write into texture
    hr = m_device->CreateUnorderedAccessView(backBuffer, NULL, &m_backBufferUAV);
    return hr;
}

HRESULT GraphicsEngine::InitializeShaders()
{
    m_initRaysShader = m_computeWrapper->CreateComputeShader(_T("../Shaders/InitRaysCS.hlsl"), NULL, "CS", NULL);

    return S_OK;
}

HRESULT GraphicsEngine::InitializeBuffers()
{
    return S_OK;
}
