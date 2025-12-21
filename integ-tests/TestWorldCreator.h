#ifndef TESTWORLDCREATOR_H
#define TESTWORLDCREATOR_H

#include "DemoWorldCreator.h"

class TestWorldCreator : public game::DemoWorldCreator
{
  public:
    TestWorldCreator() : game::DemoWorldCreator(game::DemoWorldCreator::Params())
    {
    }

  private:
    // WorldCreator methods
    void create() override;
};

#endif