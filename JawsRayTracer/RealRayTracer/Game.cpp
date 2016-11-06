#include "Game.h"
#include "InputSystem.h"
#include "GraphicsEngine.h"
#include "TimeSystem.h"

Game::Game()
{
}


Game::~Game()
{
}

void Game::Startup()
{
    InputSystem::Startup();
    m_inputSystem = InputSystem::GetInstance();

    GraphicsEngine::Startup(InputSystem::GetInstance()->WndProc);
    m_graphicsEngine = GraphicsEngine::GetInstance();

    TimeSystem::Startup();
}

void Game::Run()
{
    TimeSystem* timeSystem = TimeSystem::GetInstance();

    timeSystem->StartClock();

    while (true)
    {
        // Tick time
        timeSystem->Tick();

        // Loop as many update-steps we will take this frame
        while (timeSystem->ShouldUpdateFrame())
        {
            // Update game based on state
            Update(timeSystem->GetStepLength());

            // Update accumulator and gametime
            timeSystem->UpdateAccumulatorAndGameTime();
        }

        // Update alpha used for inteprolation
        double alpha = timeSystem->GetFrameAlpha();

        // Update camera after we update positions
        Render();
    }
}

void Game::Update(double p_stepLength)
{
}

void Game::Render()
{
}
