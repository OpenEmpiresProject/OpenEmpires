#ifndef TEST_EVENT_H
#define TEST_EVENT_H

#include "Event.h"
#include <gtest/gtest.h>

using namespace ion;

// Test the parameterized constructor
TEST(EventTest, ParameterizedConstructor) {
    Event event(Event::Type::TICK);
    EXPECT_EQ(event.type, Event::Type::TICK);
}

#endif
