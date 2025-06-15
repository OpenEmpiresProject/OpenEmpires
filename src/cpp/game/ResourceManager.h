#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "EventHandler.h"

namespace game
{
class ResourceManager : public ion::EventHandler
{
  public:
    ResourceManager(/* args */);
    ~ResourceManager();

  private:
    void onEvent(const ion::Event& e) override;
    void onTick(const ion::Event& e);
};

} // namespace game

#endif