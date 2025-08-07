#ifndef UNITMANAGER_H
#define UNITMANAGER_H

#include "EventHandler.h"
#include "UnitSelection.h"
#include "utils/Types.h"

#include <vector>

namespace core
{
class Coordinates;

class UnitManager : public EventHandler
{
  public:
    UnitManager();

  private:
    // Unit selection related
    struct TileMapQueryResult
    {
        uint32_t entity = entt::null;
        MapLayerType layer = MapLayerType::MAX_LAYERS;
    };

    void onMouseButtonUp(const Event& e);
    void onMouseButtonDown(const Event& e);
    void onBuildingPlacementStarted(const Event& e);
    void onBuildingPlacementFinished(const Event& e);
    void onUnitDeletion(const Event& e);
    void addEntitiesToSelection(const std::vector<uint32_t>& selectedEntities,
                                UnitSelection& selection);
    void updateSelection(const UnitSelection& newSelection);
    TileMapQueryResult whatIsAt(const Vec2& screenPos);
    void onClickToSelect(const Vec2& screenPos);
    void resolveSelection(const Vec2& screenPos);
    void resolveAction(const Vec2& screenPos);
    void completeSelectionBox(const Vec2& startScreenPos, const Vec2& endScreenPos);
    void onUnitSelection(const Event& e);
    void onUnitRequested(const Event& e);

  private:
    // Common
    Ref<Coordinates> m_coordinates;

    bool m_buildingPlacementInProgress = false;
    Vec2 m_selectionStartPosScreenUnits;
    bool m_isSelectionBoxInProgress = false;
    UnitSelection m_currentUnitSelection;

    // Other functionalities goes here
};

} // namespace core

#endif