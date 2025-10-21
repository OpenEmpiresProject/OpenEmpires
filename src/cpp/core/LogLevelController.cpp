#include "LogLevelController.h"

#include "utils/Logger.h"

#include <SDL3/SDL.h>

using namespace core;

LogLevelController::LogLevelController()
{
    registerCallback(Event::Type::KEY_UP, this, &LogLevelController::onKeyUp);
}

void LogLevelController::onKeyUp(const Event& e)
{
    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);

    if (scancode == SDL_SCANCODE_T)
    {
        m_showSpamLogs = !m_showSpamLogs;
        if (m_showSpamLogs)
        {
            spdlog::default_logger()->set_level(spdlog::level::trace);
        }
        else
        {
            spdlog::default_logger()->set_level(spdlog::level::debug);
        }
    }
}
