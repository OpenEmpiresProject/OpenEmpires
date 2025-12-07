#pragma once

#include "EventPublisher.h"
#include "Event.h"

#include <vector>
#include <memory>

namespace core::test
{

// Simple test publisher to capture published events for assertions.
// Call install() after creating the shared_ptr to set the thread-local publisher.
class TestEventPublisher : public core::EventPublisher
{
  public:
    TestEventPublisher() = default;

    // Install this instance as the thread-local publisher. Must be called on a shared_ptr
    // instance (i.e. after std::make_shared<TestEventPublisher>()), otherwise shared_from_this()
    // would throw.
    void install()
    {
        registerPublisher();
    }

    void publish(const core::Event& event) override
    {
        m_events.push_back(event);
    }

    const std::vector<core::Event>& events() const { return m_events; }
    void clear() { m_events.clear(); }

  private:
    std::vector<core::Event> m_events;
};

} // namespace core::test