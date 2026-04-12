#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "Event.h"
#include "EventLoop.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>

namespace core
{
class EventHandler
{
  public:
    virtual void onInit(EventLoop& eventLoop) {};
    virtual void onExit() {};

    // Return true if the event is considered consumed and don't want to
    // let any other event handlers to consume it.
    virtual bool onEvent(const Event& e)
    {
        return false;
    }

    // Will be called from EventLoop
    bool dispatchEvent(const Event& e)
    {
        bool consumed = onEvent(e);
        if (not consumed)
        {
            auto index = static_cast<size_t>(e.type);
            assert(index < m_callbacksTable.size());
            if (auto& handler = m_callbacksTable[index])
            {
                return handler(e); // Call the bound handler
            }
        }
        return consumed;
    }

    template <typename T>
    void registerCallback(Event::Type type, T* instance, bool (T::*method)(const Event&))
    {
        static_assert(std::is_base_of_v<EventHandler, T>, "Handler must inherit from EventHandler");
        auto index = static_cast<size_t>(type);
        assert(index < m_callbacksTable.size());
        m_callbacksTable[index] = std::bind(method, instance, std::placeholders::_1);
    }

  private:
    using CallbackFn = std::function<bool(const Event&)>;
    static constexpr size_t MAX_EVENT_TYPES = static_cast<size_t>(Event::Type::MAX_TYPE_MARKER);
    std::array<CallbackFn, MAX_EVENT_TYPES> m_callbacksTable;
};
} // namespace core

#endif