#include "ResourceManager.h"

#include "GameTypes.h"
#include "StateManager.h"
#include "components/CompEntityInfo.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"

using namespace game;
using namespace core;

ResourceManager::ResourceManager(/* args */)
{
    registerCallback(Event::Type::TICK, this, &ResourceManager::onTick);
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::onEvent(const Event& e)
{
}

void ResourceManager::onTick(const Event& e)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    for (auto entity : StateManager::getDirtyEntities())
    {
        if (stateMan->hasComponent<CompResource>(entity))
        {
            auto [resource, info, select, transform] =
                ServiceRegistry::getInstance()
                    .getService<StateManager>()
                    ->getComponents<CompResource, CompEntityInfo, CompSelectible, CompTransform>(
                        entity);

            if (resource.remainingAmount == 0)
            {
                info.isDestroyed = true;
                auto tile = transform.position.toTile();
                stateMan->gameMap().removeStaticEntity(tile, entity);
            }
            else if (resource.remainingAmount < resource.original.value().amount)
            {
                if (info.entityType == EntityTypes::ET_TREE)
                {
                    // TODO: Move this to python configurations
                    info.state = (int) TreeState::STUMP;
                    info.variation = 0; //  regardless of the tree type, this is the chopped version
                    // TODO: Might not be the most optimal way to bring down the bounding box a
                    // chopped tree
                    // auto tw = Constants::TILE_PIXEL_WIDTH;
                    // auto th = Constants::TILE_PIXEL_HEIGHT;
                    // select.boundingBoxes[static_cast<int>(Direction::NONE)] =
                    //    Rect<int>(tw / 2, th / 2, tw, th);

                    auto tile = transform.position.toTile();
                    auto shadow = stateMan->gameMap().getEntity(MapLayerType::ON_GROUND, tile);

                    if (shadow != entt::null)
                    {
                        auto& shadowInfo = stateMan->getComponent<CompEntityInfo>(shadow);
                        shadowInfo.isDestroyed = true;
                        StateManager::markDirty(shadow);
                        stateMan->gameMap().removeStaticEntity(tile, shadow);
                    }
                }
            }
        }
    }
}