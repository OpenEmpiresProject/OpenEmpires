#ifndef GAME_SPECIALBUILDINGMANAGER_H
#define GAME_SPECIALBUILDINGMANAGER_H
#include "EventHandler.h"

namespace game
{
class SpecialBuildingManager : public core::EventHandler
{
  public:
    SpecialBuildingManager();
    ~SpecialBuildingManager();

  private:
    void onEnterLOS(const core::Event& e);
    void onExitLOS(const core::Event& e);
    void onBuildingConstructed(const core::Event& e);
    void onInit(core::EventLoop& eventLoop) override;

    void lookupEntityTypes();
    void updatePassabilityForGate(uint32_t gateEntity, bool gateIsOpen);
    void tryOpeningGateFor(uint32_t gate, uint32_t target, bool open);
    bool isAGate(uint32_t entity) const;

    uint32_t m_gateEntityType = entt::null;
    const std::string m_gateEntityName = "stone_gate";
    core::LazyServiceRef<core::StateManager> m_stateMan;
};
} // namespace game

#endif // GAME_SPECIALBUILDINGMANAGER_H
