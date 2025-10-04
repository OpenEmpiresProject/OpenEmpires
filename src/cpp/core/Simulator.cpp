#include "Simulator.h"

#include "Coordinates.h"
#include "Event.h"
#include "PlayerController.h"
#include "PlayerFactory.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "UI.h"
#include "commands/CmdGatherResource.h"
#include "commands/CmdIdle.h"
#include "commands/CmdMove.h"
#include "commands/cmdBuild.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUIElement.h"
#include "components/CompUnit.h"
#include "utils/ObjectPool.h"

#include <algorithm>
#include <entt/entity/registry.hpp>

using namespace core;

char scancodeToChar(SDL_Scancode scancode, bool shiftPressed);

Simulator::Simulator(ThreadSynchronizer<FrameData>& synchronizer)
    : m_synchronizer(synchronizer),
      m_coordinates(ServiceRegistry::getInstance().getService<Coordinates>())
{
    CompDirty::g_dirtyEntities.reserve(10000);

    registerCallback(Event::Type::TICK, this, &Simulator::onTick);
    registerCallback(Event::Type::KEY_UP, this, &Simulator::onKeyUp);
    registerCallback(Event::Type::ENTITY_SELECTION, this, &Simulator::onUnitSelection);
}

void Simulator::onInit(EventLoop& eventLoop)
{
    ObjectPool<CompGraphics>::reserve(1000);
}

void Simulator::onTick(const Event& e)
{
    onTickStart();
    updateGraphicComponents();
    sendGraphicsInstructions();
    onTickEnd();
}

void Simulator::onKeyUp(const Event& e)
{
    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);

    if (scancode == SDL_SCANCODE_T)
    {
        m_showSpamLogs = !m_showSpamLogs;
        if (m_showSpamLogs)
        {
            spdlog::default_logger()->set_level(spdlog::level::trace);
        }
        else
        {
            spdlog::default_logger()->set_level(spdlog::level::debug);
        }
    }
}

void Simulator::onTickStart()
{
    // Read and send data
    auto player = ServiceRegistry::getInstance().getService<PlayerController>()->getPlayer();
    m_synchronizer.getSenderFrameData().fogOfWar = *(player->getFogOfWar().get());
    m_synchronizer.getSenderFrameData().frameNumber = m_frame;
    m_coordinates->setViewportPositionInPixels(
        m_synchronizer.getSenderFrameData().viewportPositionInPixels);

    if (!m_initialized)
    {
        ServiceRegistry::getInstance().getService<StateManager>()->getEntities<CompDirty>().each(
            [this](uint32_t entity, CompDirty& dirty) { dirty.markDirty(entity); });
        m_initialized = true;
    }
}

void Simulator::onTickEnd()
{
    m_frame++;
    m_synchronizer.waitForReceiver([&]() { onSynchorizedBlock(); });
    CompDirty::g_dirtyEntities.clear();
}

void Simulator::onSynchorizedBlock()
{
    // TODO: Make this work, might need to delay destroying entities
    // ServiceRegistry::getInstance().getService<GameState>()->destroyAllPendingEntities();
}

void Simulator::onUnitSelection(const Event& e)
{
    auto& selectedEntities = e.getData<EntitySelectionData>().selection.selectedEntities;
    if (selectedEntities.size() == 1)
    {
        auto resourceEntity = selectedEntities[0];
        auto state = ServiceRegistry::getInstance().getService<StateManager>();

        if (state->hasComponent<CompResource>(resourceEntity))
        {
            auto resource = state->getComponent<CompResource>(resourceEntity);
            spdlog::info("Selected entity has {} resources", resource.remainingAmount);
        }
    }
}

void Simulator::sendGraphicsInstructions()
{
    auto state = ServiceRegistry::getInstance().getService<StateManager>();

    for (auto entity : CompDirty::g_dirtyEntities)
    {
        auto& gc = state->getComponent<CompGraphics>(entity);
        auto instruction = ObjectPool<CompGraphics>::acquire();
        *instruction = gc;

        if (gc.entityID != entt::null)
            sendGraphiInstruction(instruction);
        else
            spdlog::error("Invalid entity found during simulation for id: {}", gc.toString());
    }
}

void Simulator::updateGraphicComponents()
{
    auto state = ServiceRegistry::getInstance().getService<StateManager>();
    for (auto entity : CompDirty::g_dirtyEntities)
    {
        auto [transform, entityInfo, gc] =
            state->getComponents<CompTransform, CompEntityInfo, CompGraphics>(entity);

        gc.positionInFeet = transform.position;
        gc.direction = static_cast<uint64_t>(transform.getIsometricDirection());
        gc.variation = entityInfo.variation;
        gc.entitySubType = entityInfo.entitySubType;
        gc.entityType = entityInfo.entityType;
        gc.isDestroyed = entityInfo.isDestroyed;
        gc.isEnabled = entityInfo.isEnabled;
        gc.entityID = entityInfo.entityId;

        if (state->hasComponent<CompSelectible>(entity))
        {
            auto& select = state->getComponent<CompSelectible>(entity);
            if (select.isSelected)
            {
                // TODO: Respect other addons
                gc.addons = {select.selectionIndicator};
            }
            else
            {
                gc.addons.clear();
            }
        }

        if (state->hasComponent<CompAnimation>(entity))
        {
            auto& animation = state->getComponent<CompAnimation>(entity);
            gc.frame = animation.frame;
        }

        if (state->hasComponent<CompAction>(entity))
        {
            auto& action = state->getComponent<CompAction>(entity);
            gc.action = action.action;
        }

        if (state->hasComponent<CompBuilding>(entity))
        {
            auto& building = state->getComponent<CompBuilding>(entity);
            if (building.validPlacement)
                gc.shading = Color::NONE;
            else
                gc.shading = Color::RED;

            gc.landSize = building.size;
        }

        if (state->hasComponent<CompUIElement>(entity))
        {
            gc.positionInFeet = Feet::null;
            gc.positionInScreenUnits = {transform.position.x, transform.position.y};

            auto& ui = state->getComponent<CompUIElement>(entity);
            gc.isEnabled = ui.isVisible;

            if (ui.type == UIRenderingType::TEXT)
            {
                GraphicAddon addon;
                addon.type = GraphicAddon::Type::TEXT;
                addon.data = GraphicAddon::Text{.text = ui.text, .color = ui.color};
                gc.addons = {addon};
            }
            else if (ui.type == UIRenderingType::TEXTURE)
            {
                if (ui.backgroundImage.isValid())
                {
                    gc.entityType = ui.backgroundImage.entityType;
                    gc.entitySubType = ui.backgroundImage.entitySubType;
                    gc.variation = ui.backgroundImage.variation;
                }
            }
        }

        if (state->hasComponent<CompPlayer>(entity))
        {
            auto& comp = state->getComponent<CompPlayer>(entity);
            gc.playerId = comp.player->getId();
        }
    }
}

void Simulator::sendGraphiInstruction(CompGraphics* instruction)
{
    m_synchronizer.getSenderFrameData().graphicUpdates.push_back(instruction);
}

char scancodeToChar(SDL_Scancode scancode, bool shiftPressed)
{
    // Handle letters (A-Z)
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
    {
        return shiftPressed ? 'A' + (scancode - SDL_SCANCODE_A) : 'a' + (scancode - SDL_SCANCODE_A);
    }

    // Handle numbers (0–9)
    if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0)
    {
        if (!shiftPressed)
        {
            static const char numMap[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
            return numMap[(scancode - SDL_SCANCODE_1) % 10];
        }
        else
        {
            static const char shiftNumMap[] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
            return shiftNumMap[(scancode - SDL_SCANCODE_1) % 10];
        }
    }

    // Special cases (US QWERTY)
    static const std::unordered_map<SDL_Scancode, std::pair<char, char>> specialMap = {
        {SDL_SCANCODE_SPACE, {' ', ' '}},        {SDL_SCANCODE_MINUS, {'-', '_'}},
        {SDL_SCANCODE_EQUALS, {'=', '+'}},       {SDL_SCANCODE_LEFTBRACKET, {'[', '{'}},
        {SDL_SCANCODE_RIGHTBRACKET, {']', '}'}}, {SDL_SCANCODE_BACKSLASH, {'\\', '|'}},
        {SDL_SCANCODE_SEMICOLON, {';', ':'}},    {SDL_SCANCODE_APOSTROPHE, {'\'', '"'}},
        {SDL_SCANCODE_GRAVE, {'`', '~'}},        {SDL_SCANCODE_COMMA, {',', '<'}},
        {SDL_SCANCODE_PERIOD, {'.', '>'}},       {SDL_SCANCODE_SLASH, {'/', '?'}}};

    auto it = specialMap.find(scancode);
    if (it != specialMap.end())
    {
        return shiftPressed ? it->second.second : it->second.first;
    }

    return '\0'; // Unhandled key
}