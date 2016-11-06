#pragma once

class InputSystem;
class GraphicsEngine;

class Game
{
public:
    Game();
    ~Game();

    void Startup(int p_nCmdShow);
    void Run();

private:

    void Update(double p_stepLength);
    void Render();

    InputSystem* m_inputSystem = nullptr;
    GraphicsEngine* m_graphicsEngine = nullptr;
};

