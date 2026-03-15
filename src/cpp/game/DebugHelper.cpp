#include "DebugHelper.h"

#include "Feet.h"
#include "components/CompGraphics.h"

using namespace game;
using namespace core;

DebugHelper::DebugHelper()
{
    registerCallback(Event::Type::TICK, this, &DebugHelper::onTick);
}

DebugHelper::~DebugHelper()
{
    // destructor
}

void DebugHelper::onTick(const core::Event& e)
{
#ifndef NDEBUG

    auto& densityGrid = m_stateMan->getDensityGrid();
    auto& gameMap = m_stateMan->gameMap();
    const int densityGridTileSize = Constants::FEET_PER_TILE / Constants::DENSITY_GRID_RESOLUTION;
    const int densityGridTileHalfSize = densityGridTileSize / 2;

    for (size_t tileY = 0; tileY < gameMap.height; tileY++)
    {
        for (size_t tileX = 0; tileX < gameMap.width; tileX++)
        {
            auto tile = gameMap.getEntity(MapLayerType::GROUND, Tile(tileX, tileY));
            auto& tileGraphic = m_stateMan->getComponent<CompGraphics>(tile);
            size_t debugOverlayIndex = 0;
            bool dirty = false;

            for (size_t densityTileRelativeY = 0;
                 densityTileRelativeY < Constants::DENSITY_GRID_RESOLUTION; ++densityTileRelativeY)
            {
                for (size_t densityTileRelativeX = 0;
                     densityTileRelativeX < Constants::DENSITY_GRID_RESOLUTION;
                     ++densityTileRelativeX)
                {
                    size_t densityTileX =
                        tileX * Constants::DENSITY_GRID_RESOLUTION + densityTileRelativeX;
                    size_t densityTileY =
                        tileY * Constants::DENSITY_GRID_RESOLUTION + densityTileRelativeY;

                    Feet pos(densityTileX * densityGridTileSize + densityGridTileHalfSize,
                             densityTileY * densityGridTileSize + densityGridTileHalfSize);
                    auto density = densityGrid.getDensitySaturated(pos);
                    bool prevState = tileGraphic.debugOverlays[debugOverlayIndex].enabled;
                    Color prevColor = tileGraphic.debugOverlays[debugOverlayIndex].color;

                    if (density > 0.1)
                    {
                        if (density > 0.4)
                        {
                            tileGraphic.debugOverlays[debugOverlayIndex].color = Color::RED;
                        }
                        else if (density > 0.3)
                        {
                            tileGraphic.debugOverlays[debugOverlayIndex].color = Color::YELLOW;
                        }
                        else
                        {
                            tileGraphic.debugOverlays[debugOverlayIndex].color = Color::GREEN;
                        }
                        tileGraphic.debugOverlays[debugOverlayIndex].color.a = 100;
                        tileGraphic.debugOverlays[debugOverlayIndex].enabled = true;
                    }
                    else
                    {
                        tileGraphic.debugOverlays[debugOverlayIndex].enabled = false;
                    }

                    if (prevState != tileGraphic.debugOverlays[debugOverlayIndex].enabled)
                        dirty = true;

                    if (prevColor != tileGraphic.debugOverlays[debugOverlayIndex].color)
                        dirty = true;

                    debugOverlayIndex++;
                }
            }

            if (dirty)
                StateManager::markDirty(tile);
        }
    }

#endif
}
