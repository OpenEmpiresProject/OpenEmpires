#ifndef CORE_BASEUNITFORMATION_H
#define CORE_BASEUNITFORMATION_H
#include "Feet.h"
#include "Path.h"
#include "Player.h"
#include "Settings.h"

#include <entt/entity/registry.hpp>

namespace core
{
class BaseUnitFormation;
class PathService;

struct FormationSlot
{
    size_t index = 0;
    Feet offsetFromAnchor = Feet::zero;

    FormationSlot() = default;
    FormationSlot(uint32_t entity, Ref<BaseUnitFormation> formation);

    uint32_t getEntityId() const;
    Ref<BaseUnitFormation> getFormation() const;
    bool isValid() const;
    bool isInSameFormation(Ref<BaseUnitFormation> other) const;

  private:
    uint32_t unitEntityId = entt::null;
    Ref<BaseUnitFormation> formation;

    friend class BaseUnitFormation;
    void removeFromFormation()
    {
        unitEntityId = entt::null;
        formation = nullptr;
    }
};

enum class FormationState
{
    MOVING,
    REACHED,
};

class BaseUnitFormation
{
  public:
    BaseUnitFormation() = default;
    virtual ~BaseUnitFormation() = default;

    virtual void createFormation(const std::vector<uint32_t>& unitEntityIds,
                                 const Feet& target) = 0;
    virtual void deleteFormation() = 0;
    virtual void move(const Feet& newPos) = 0;
    virtual bool isInsideFormation(const Feet& point) const = 0;

    const Feet& getAnchor() const;
    const std::vector<FormationSlot>& getSlots() const;
    const Feet& getForward() const;
    FormationState getState() const;
    Ref<Player> getControllingPlayer() const;
    void setControllingPlayer(Ref<Player> player);
    void giveUpControl();
    void updateTarget(const Feet& target);
    bool move(int deltaTimeMs);

  protected:
    std::vector<FormationSlot> m_slots;
    Feet m_anchor = Feet::null;
    Feet m_forward = Feet::zero;
    int m_speed = 0;
    FormationState m_state = FormationState::MOVING;
    Ref<Player> m_controllingPlayer;
    Feet m_target = Feet::null;
    Path m_path;
    LazyServiceRef<PathService> m_pathService;
    LazyServiceRef<StateManager> m_stateMan;
    LazyServiceRef<Settings> m_settings;

    const int GOAL_RADIUS_SQ = 10000;

    void removeSlotFromFormation(FormationSlot& slot);
    void removeAllSlots();
    bool isTargetCloseEnough() const;
};
} // namespace core

#endif // CORE_BASEUNITFORMATION_H
