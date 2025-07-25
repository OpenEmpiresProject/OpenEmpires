#include "ResourceManager.h"

#include "GameState.h"
#include "GameTypes.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"

using namespace game;
using namespace ion;

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
    for (auto entity : CompDirty::g_dirtyEntities)
    {
        if (ServiceRegistry::getInstance().getService<GameState>()->hasComponent<CompResource>(
                entity))
        {
            auto [resource, dirty, info, select, transform] =
                ServiceRegistry::getInstance()
                    .getService<GameState>()
                    ->getComponents<CompResource, CompDirty, CompEntityInfo, CompSelectible,
                                    CompTransform>(entity);

            if (resource.remainingAmount == 0)
            {
                info.isDestroyed = true;
                ServiceRegistry::getInstance().getService<GameState>()->gameMap.removeStaticEntity(
                    transform.position.toTile(), entity);
            }
            else
            {
                if (info.entityType == EntityTypes::ET_TREE)
                {
                    info.entitySubType = 1;
                    info.variation = 0; //  regardless of the tree type, this is the chopped version
                    // TODO: Might not be the most optimal way to bring down the bounding box a
                    // chopped tree
                    auto tw = Constants::TILE_PIXEL_WIDTH;
                    auto th = Constants::TILE_PIXEL_HEIGHT;
                    select.boundingBoxes[static_cast<int>(Direction::NONE)] =
                        Rect<int>(tw / 2, th / 2, tw, th);
                }
            }
        }
    }
}