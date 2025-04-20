#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "EventLoop.h"

using namespace aion;
using namespace testing;

class MockEventLoopListener : public EventListener {
public:
    void onEvent(const Event& e) override {
        callCount++;
    }
    void onInit() {};
    void onExit() {};
    int callCount = 0;
};

TEST(EventLoopTest, RegisterAndDeregisterListener) {
    EventLoop eventLoop;
    auto mockListener = std::make_unique<MockEventLoopListener>();
    auto rawListenerPtr = mockListener.get();

    eventLoop.registerListener(std::move(mockListener));
    ASSERT_EQ(eventLoop.getListenersCount(), 1);

    eventLoop.deregisterListener(rawListenerPtr);
    ASSERT_EQ(eventLoop.getListenersCount(), 0);
}

TEST(EventLoopTest, TickEventIsTriggered) {
    SubSystem* eventLoop = new EventLoop();
    auto mockListener = std::make_unique<MockEventLoopListener>();
    auto mockListernerRawPtr = mockListener.get(); // Good enough for a test

    ((EventLoop*)eventLoop)->registerListener(std::move(mockListener));
    eventLoop->init();

    // Let the event loop run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
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