#ifndef CORE_INPUTPROCESSOR_H
#define CORE_INPUTPROCESSOR_H

#include "Event.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <stdint.h>

namespace core
{
class InputProcessor
{
  public:
    InputProcessor()
    {
        m_previousKeyboardState = new bool[SDL_SCANCODE_COUNT];
        for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
        {
            m_previousKeyboardState[i] = false;
        }
    }
    ~InputProcessor() = default;

    template <typename Callback> void processInputs(const Callback& callback)
    {
        int numKeys = 0;
        const bool* currentKeyboardState = SDL_GetKeyboardState(&numKeys);
        for (int i = 0; i < numKeys; ++i)
        {
            if (currentKeyboardState[i] && !m_previousKeyboardState[i])
            {
                KeyboardData data{i};
                Event keyDownEvent(Event::Type::KEY_DOWN, data);
                callback(keyDownEvent);
            }
            if (!currentKeyboardState[i] && m_previousKeyboardState[i])
            {
                KeyboardData data{i};
                Event keyDownEvent(Event::Type::KEY_UP, data);
                callback(keyDownEvent);
            }
            m_previousKeyboardState[i] = currentKeyboardState[i];
        }

        float mouseX = 0;
        float mouseY = 0;
        SDL_MouseButtonFlags currentMouseState = SDL_GetMouseState(&mouseX, &mouseY);

        if (m_previouseMouseX != mouseX || m_previouseMouseY != mouseY)
        {
            MouseMoveData data{Vec2(mouseX, mouseY)};
            Event mouseMoveEvent(Event::Type::MOUSE_MOVE, data);
            callback(mouseMoveEvent);

            m_previouseMouseX = mouseX;
            m_previouseMouseY = mouseY;
        }

        if (currentMouseState != m_previousMouseState)
        {
            if ((currentMouseState & SDL_BUTTON_LMASK) &&
                !(m_previousMouseState & SDL_BUTTON_LMASK))
            {
                MouseClickData data{MouseClickData::Button::LEFT, Vec2(mouseX, mouseY)};
                Event mouseClickEvent(Event::Type::MOUSE_BTN_DOWN, data);
                callback(mouseClickEvent);
            }

            if (!(currentMouseState & SDL_BUTTON_LMASK) &&
                (m_previousMouseState & SDL_BUTTON_LMASK))
            {
                MouseClickData data{MouseClickData::Button::LEFT, Vec2(mouseX, mouseY)};
                Event mouseClickEvent(Event::Type::MOUSE_BTN_UP, data);
                callback(mouseClickEvent);
            }

            if ((currentMouseState & SDL_BUTTON_RMASK) &&
                !(m_previousMouseState & SDL_BUTTON_RMASK))
            {
                MouseClickData data{MouseClickData::Button::RIGHT, Vec2(mouseX, mouseY)};
                Event mouseClickEvent(Event::Type::MOUSE_BTN_DOWN, data);
                callback(mouseClickEvent);
            }
            if (!(currentMouseState & SDL_BUTTON_RMASK) &&
                (m_previousMouseState & SDL_BUTTON_RMASK))
            {
                MouseClickData data{MouseClickData::Button::RIGHT, Vec2(mouseX, mouseY)};
                Event mouseClickEvent(Event::Type::MOUSE_BTN_UP, data);
                callback(mouseClickEvent);
            }
            m_previousMouseState = currentMouseState;
        }
    }

  private:
    bool* m_previousKeyboardState = nullptr;
    uint32_t m_previousMouseState = 0;
    int m_previouseMouseX = 0;
    int m_previouseMouseY = 0;
};
} // namespace core

#endif // CORE_INPUTPROCESSOR_H
