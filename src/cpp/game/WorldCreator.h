#ifndef WORLDCREATOR_H
#define WORLDCREATOR_H

#include "DRSFile.h"
#include "EventHandler.h"
#include "GameTypes.h"
#include "GraphicsRegistry.h"
#include "Player.h"
#include "Property.h"
#include "Renderer.h"
#include "Settings.h"
#include "StateManager.h"
#include "SubSystem.h"
#include "Tile.h"
#include "TileMap.h"

#include <memory>

namespace game
{

class WorldCreator : public core::EventHandler
{
  public:
    struct Params
    {
    };

    WorldCreator(const Params& params) {};
    virtual ~WorldCreator() = default;

    virtual void create() = 0;
    virtual bool isReady() const = 0;
};
} // namespace game

#endif