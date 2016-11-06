#pragma once
#include <Windows.h>

class InputSystem
{
public:
    static InputSystem* GetInstance();
    static void Startup();

    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    InputSystem();
    ~InputSystem();

    static InputSystem* m_singleton;

};

