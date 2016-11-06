#pragma once

class InputSystem;
class GraphicsEngine;

class Game
{
public:
    Game();
    ~Game();

    void Startup();
    void Run();

private:

    void Update(double p_stepLength);
    void Render();

    InputSystem* m_inputSystem = nullptr;
    GraphicsEngine* m_graphicsEngine = nullptr;
};

