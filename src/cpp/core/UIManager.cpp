#include "UIManager.h"

using namespace core;

UIManager::UIManager()
{
    registerCallback(Event::Type::TICK, this, &UIManager::onTick);
}

void UIManager::registerWindow(Ref<ui::Window> window)
{

    m_windows.push_back(window);
}

void UIManager::onEvent(const Event& e)
{
    for (auto window : m_windows)
    {
        window->feedInput(e);
    }
}

void UIManager::onTick(const Event& e)
{
    for (auto window : m_windows)
    {
        window->updateGraphicCommand();
    }
}
