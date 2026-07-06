#include "DebugHelper.h"

#include "Feet.h"
#include "Gizmos.h"
#include "components/CompGraphics.h"
#include "components/CompTransform.h"
#include "logging/Logger.h"

#include <fmt/core.h>

using namespace core;

DebugHelper::DebugHelper()
{
    registerCallback(Event::Type::TICK, this, &DebugHelper::onTick);
    registerCallback(Event::Type::KEY_UP, this, &DebugHelper::onKeyUp);
}

DebugHelper::~DebugHelper()
{
    // destructor
}

bool DebugHelper::onTick(const core::Event& e)
{
#ifdef DEBUG
    if (not m_showGizmos)
        return false;

    /*
     *   Below fmt::format proved to be extremely costly, therefore we precalculate
     *   and cache the values.
     */
    if (m_densityGridCellGizmoNames.isEmpty())
    {
        m_densityGridCellGizmoNames.resize(Constants::DENSITY_GRID_RESOLUTION,
                                           Constants::DENSITY_GRID_RESOLUTION);
        for (size_t x = 0; x < Constants::DENSITY_GRID_RESOLUTION; ++x)
        {
            for (size_t y = 0; y < Constants::DENSITY_GRID_RESOLUTION; ++y)
            {
                auto gizmoName = fmt::format("density___{}_{}", x, y);
                m_densityGridCellGizmoNames.set(x, y, gizmoName);
            }
        }
    }

    auto& densityGrid = m_stateMan->getDensityGrid();
    auto& gameMap = m_stateMan->gameMap();
    const int densityGridTileSize = Constants::FEET_PER_TILE / Constants::DENSITY_GRID_RESOLUTION;
    const int densityGridTileHalfSize = densityGridTileSize / 2;
    const int subCellFeetSize = Constants::FEET_PER_TILE / Constants::DENSITY_GRID_RESOLUTION;

    for (size_t tileY = 0; tileY < gameMap.height; tileY++)
    {
        for (size_t tileX = 0; tileX < gameMap.width; tileX++)
        {
            auto tile = gameMap.getEntity(MapLayerType::GROUND, Tile(tileX, tileY));
            auto [tileGraphic, tileTransform] =
                m_stateMan->getComponents<CompGraphics, CompTransform>(tile);

            bool firstSubCell = true;

            for (size_t densityTileRelativeY = 0;
                 densityTileRelativeY < Constants::DENSITY_GRID_RESOLUTION; ++densityTileRelativeY)
            {
                for (size_t densityTileRelativeX = 0;
                     densityTileRelativeX < Constants::DENSITY_GRID_RESOLUTION;
                     ++densityTileRelativeX)
                {

                    const auto& gizmoName =
                        m_densityGridCellGizmoNames.at(densityTileRelativeX, densityTileRelativeY);

                    size_t densityTileX =
                        tileX * Constants::DENSITY_GRID_RESOLUTION + densityTileRelativeX;
                    size_t densityTileY =
                        tileY * Constants::DENSITY_GRID_RESOLUTION + densityTileRelativeY;

                    Feet pos(densityTileX * densityGridTileSize + densityGridTileHalfSize,
                             densityTileY * densityGridTileSize + densityGridTileHalfSize);
                    auto density = densityGrid.getDensitySaturated(pos);
                    Color newColor = Color::NONE;

                    if (density > 0.1)
                    {
                        if (density > 0.4)
                        {
                            newColor = Color::RED.withAlpha40();
                        }
                        else if (density > 0.3)
                        {
                            newColor = Color::YELLOW.withAlpha40();
                        }
                        else
                        {
                            newColor = Color::GREEN.withAlpha40();
                        }

                        int cx = tileTransform.position.x + densityTileRelativeX * subCellFeetSize;
                        int cy = tileTransform.position.y + densityTileRelativeY * subCellFeetSize;

                        std::array<Feet, 4> corners;
                        corners[0] = Feet(cx, cy);
                        corners[1] = Feet(cx + subCellFeetSize, cy);
                        corners[2] = Feet(cx + subCellFeetSize, cy + subCellFeetSize);
                        corners[3] = Feet(cx, cy + subCellFeetSize);
                        Gizmos::drawQuad(tile, gizmoName, corners, newColor, true);
                    }
                    else
                    {
                        Gizmos::clearDrawing(tile, tileGraphic, gizmoName);
                    }
                }
            }
        }
    }

#endif
    return false;
}

bool DebugHelper::onKeyUp(const core::Event& e)
{
    auto& data = e.getData<KeyboardData>();
    if (data.keyCode == SDL_SCANCODE_9)
    {
        m_showDebugDetails = !m_showDebugDetails;
    }
    else if (data.keyCode == SDL_SCANCODE_F10)
    {
        pauseGame(not isGamePaused());
    }
    else if (data.keyCode == SDL_SCANCODE_0)
    {
        hideFogOfWar(not isHidingFogOfWar());
    }
    return false;
}
