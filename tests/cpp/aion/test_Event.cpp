#ifndef TEST_EVENT_H
#define TEST_EVENT_H

#include "Event.h"
#include <gtest/gtest.h>

// Test the parameterized constructor
TEST(EventTest, ParameterizedConstructor) {
    aion::Event event(aion::Event::Type::TICK);
    EXPECT_EQ(event.type, aion::Event::Type::TICK);
}

#endif
