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
    void onEvent(const core::Event& e) override;
    void onTick(const core::Event& e);
};

} // namespace game

#endif