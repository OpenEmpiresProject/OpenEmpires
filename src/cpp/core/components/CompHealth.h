#ifndef COMP_HEALTH_H
#define COMP_HEALTH_H

#include "Property.h"

namespace core
{
class CompHealth
{
  public:
    Property<uint32_t> maxHealth;

  public:
    float health = 0.0;
    bool isDead = false;

    void onCreate(uint32_t)
    {
        health = maxHealth.value();
    }
};

} // namespace core

#endif