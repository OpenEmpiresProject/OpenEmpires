#ifndef TEST_EVENT_H
#define TEST_EVENT_H

#include "Event.h"
#include <gtest/gtest.h>

// Test the default constructor
TEST(EventTest, DefaultConstructor) {
    aion::Event event;
    EXPECT_EQ(event.getType(), aion::Event::Type::NONE);
}

// Test the parameterized constructor
TEST(EventTest, ParameterizedConstructor) {
    aion::Event event(aion::Event::Type::TICK);
    EXPECT_EQ(event.getType(), aion::Event::Type::TICK);
}

#endif
