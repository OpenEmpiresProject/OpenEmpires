#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "Event.h"
#include "EventLoop.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>

namespace ion
{
class EventHandler
{
  public:
    virtual void onInit(EventLoop* eventLoop) {};
    virtual void onExit() {};
    virtual void onEvent(const Event& e) {};

    // Will be called from EventLoop
    void dispatchEvent(const Event& e)
    {
        onEvent(e);
        size_t index = static_cast<size_t>(e.type);
        assert(index < m_callbacksTable.size());
        if (auto& handler = m_callbacksTable[index])
        {
            handler(e); // Call the bound handler
        }
    }

    template <typename T>
    void registerCallback(Event::Type type, T* instance, void (T::*method)(const Event&))
    {
        static_assert(std::is_base_of_v<EventHandler, T>, "Handler must inherit from EventHandler");
        size_t index = static_cast<size_t>(type);
        assert(index < m_callbacksTable.size());
        m_callbacksTable[index] = std::bind(method, instance, std::placeholders::_1);
    }

  private:
    using CallbackFn = std::function<void(const Event&)>;
    static constexpr size_t MAX_EVENT_TYPES = static_cast<size_t>(Event::Type::MAX_TYPE_MARKER);
    std::array<CallbackFn, MAX_EVENT_TYPES> m_callbacksTable;
};
} // namespace ion

#endif