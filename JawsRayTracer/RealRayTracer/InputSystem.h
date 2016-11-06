#pragma once
#include <Windows.h>

class InputSystem
{
public:
    static InputSystem* GetInstance();
    static void Startup();



private:
    InputSystem();
    ~InputSystem();

    static InputSystem* m_singleton;

};


extern LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

