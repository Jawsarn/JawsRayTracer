#pragma once
#include <d3d11_2.h>
#include <stdint.h>
#include <Windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

class ComputeWrap;
class ComputeShader;

class GraphicsEngine
{
public:
    static GraphicsEngine* GetInstance();
    static void Startup(int p_nCmdShow, WNDPROC p_winProc);




private:
    static GraphicsEngine* m_singleton;
    GraphicsEngine(int p_nCmdShow, WNDPROC p_winProc);
    ~GraphicsEngine();
    HRESULT InitializeWindow(int p_nCmdShow, WNDPROC p_winProc);
    HRESULT InitializeDirectX();
    HRESULT InitializeBackBuffer();
    HRESULT InitializeShaders();
    HRESULT InitializeBuffers();

    // DirectX Windows parameters
    HINSTANCE m_handleInstance = NULL;
    HWND m_handleWindow = NULL;

    // Window properties
    uint16_t m_width;
    uint16_t m_height;

    // Directx API and backbuffer
    IDXGISwapChain* m_swapChain = NULL;
    ID3D11Device* m_device = NULL;
    ID3D11DeviceContext* m_deviceContext = NULL;
    ID3D11UnorderedAccessView*  m_backBufferUAV = NULL;  // compute output

    // Help classes
    ComputeWrap* m_computeWrapper = NULL;

    // Shaders
    ComputeShader* m_initRaysShader = NULL;
};

