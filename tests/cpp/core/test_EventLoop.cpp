#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stop_token>
#include "EventLoop.h"
#include "EventHandler.h"

using namespace testing;
using namespace std;

namespace core
{

class MockEventLoopListener : public EventHandler
{
  public:
    void onEvent(const Event& e) override
    {
        callCount++;
    }
    void onInit(EventLoop* eventLoop) {};
    void onExit() {};
    int callCount = 0;
};

// TEST(EventLoopTest, RegisterAndDeregisterListener) {
//     stop_token token;
//     EventLoop eventLoop(&token);
//     auto mockListener = std::make_unique<MockEventLoopListener>();
//     auto rawListenerPtr = mockListener.get();

//     eventLoop.registerListener(std::move(mockListener));
//     ASSERT_EQ(eventLoop.getListenersCount(), 1);

//     eventLoop.deregisterListener(rawListenerPtr);
//     ASSERT_EQ(eventLoop.getListenersCount(), 0);
// }

TEST(EventLoopTest, TickEventIsTriggered)
{
    std::stop_source stopSource;
    std::stop_token stopToken = stopSource.get_token();

    auto loop = CreateRef<EventLoop>(&stopToken);
    std::shared_ptr<SubSystem> eventLoop = loop;

    auto mockListener = std::make_unique<MockEventLoopListener>();
    auto mockListernerRawPtr = mockListener.get(); // Good enough for a test

    loop->registerListener(std::move(mockListener));
    eventLoop->init();

    // Let the event loop run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    stopSource.request_stop(); // Stop the event loop
    eventLoop->shutdown();
    ASSERT_GT(mockListernerRawPtr->callCount, 2);
}

// TODO: Fix this. can't shutdown the eventloop
// TEST(EventLoopTest, ShutdownStopsThread) {
//     SubSystem* eventLoop = new EventLoop();
//     eventLoop->init();

//     ASSERT_TRUE(eventLoop->isRunning());

//     eventLoop->shutdown();
//     ASSERT_FALSE(eventLoop->isRunning());
// }
} // namespace core