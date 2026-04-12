#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "EventHandler.h"

namespace game
{
class ResourceManager : public core::EventHandler
{
  public:
    ResourceManager(/* args */);
    ~ResourceManager();

  private:
    bool onEvent(const core::Event& e) override;
    bool onTick(const core::Event& e);
};

} // namespace game

#endif