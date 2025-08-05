#ifndef TEST_THREADSYNCHRONIZER_H
#define TEST_THREADSYNCHRONIZER_H

#include "ThreadSynchronizer.h"
#include "FrameData.h"
#include <gtest/gtest.h>
#include <thread>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <atomic>

namespace core
{

class ThreadSynchronizerTest : public ::testing::Test
{
  protected:
    ThreadSynchronizer<FrameData> synchronizer;
};

TEST_F(ThreadSynchronizerTest, InitialFrameData)
{
    FrameData& senderFrame = synchronizer.getSenderFrameData();
    FrameData& receiverFrame = synchronizer.getReceiverFrameData();
    EXPECT_EQ(senderFrame.frameNumber, 0);
    EXPECT_EQ(receiverFrame.frameNumber, 0);
}

TEST_F(ThreadSynchronizerTest, SenderReceiverSynchronization)
{

    std::thread receiver(
        [&]()
        {
            for (int i = 1; i <= 5; ++i)
            {
                FrameData& frame = synchronizer.getReceiverFrameData();
                EXPECT_EQ(frame.frameNumber, i - 1); // Always one frame behind than the sender
                synchronizer.waitForSender();
            }
        });

    std::thread sender(
        [&]()
        {
            for (int i = 1; i <= 5; ++i)
            {
                FrameData& frame = synchronizer.getSenderFrameData();
                frame.frameNumber = i;
                synchronizer.waitForReceiver();
            }
        });

    sender.join();
    receiver.join();
}

TEST_F(ThreadSynchronizerTest, ThreadsRunInParallel)
{
    std::atomic<bool> senderStarted = false;
    std::atomic<bool> receiverStarted = false;

    std::chrono::steady_clock::time_point senderStart, senderEnd;
    std::chrono::steady_clock::time_point receiverStart, receiverEnd;

    std::thread sender(
        [&]()
        {
            senderStarted = true;
            while (!receiverStarted)
                std::this_thread::yield(); // Wait for receiver to start
            senderStart = std::chrono::steady_clock::now();

            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            synchronizer.waitForReceiver();

            senderEnd = std::chrono::steady_clock::now();
        });

    std::thread receiver(
        [&]()
        {
            receiverStarted = true;
            while (!senderStarted)
                std::this_thread::yield(); // Wait for sender to start
            receiverStart = std::chrono::steady_clock::now();

            synchronizer.waitForSender();

            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            receiverEnd = std::chrono::steady_clock::now();
        });

    sender.join();
    receiver.join();

    auto senderDuration =
        std::chrono::duration_cast<std::chrono::milliseconds>(senderEnd - senderStart);
    auto receiverDuration =
        std::chrono::duration_cast<std::chrono::milliseconds>(receiverEnd - receiverStart);
    auto totalSpan = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::max(senderEnd, receiverEnd) - std::min(senderStart, receiverStart));

    // If they were purely serial, total span would be equal or greater than the sum of both
    // durations If they were parallel, total span would be less than that
    EXPECT_LT(totalSpan.count(), senderDuration.count() + receiverDuration.count());
}

TEST_F(ThreadSynchronizerTest, GracefulShutdownUnblocksThreads)
{
    std::atomic<bool> senderExited{false};
    std::atomic<bool> receiverExited{false};

    std::thread sender(
        [&]
        {
            synchronizer.waitForReceiver();
            senderExited = true;
        });

    std::thread receiver(
        [&]
        {
            synchronizer.waitForSender();
            receiverExited = true;
        });

    // Allow threads to block
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Initiate shutdown
    synchronizer.shutdown();

    // Wait for threads to exit
    sender.join();
    receiver.join();

    EXPECT_TRUE(senderExited);
    EXPECT_TRUE(receiverExited);
}
} // namespace core

#endif // TEST_THREADSYNCHRONIZER_H