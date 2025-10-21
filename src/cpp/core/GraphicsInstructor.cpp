#include "GraphicsInstructor.h"

#include "Coordinates.h"
#include "Event.h"
#include "PlayerController.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilding.h"
#include "components/CompCursor.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUIElement.h"
#include "components/CompUnit.h"
#include "utils/ObjectPool.h"

#include <algorithm>
#include <entt/entity/registry.hpp>

using namespace core;

GraphicsInstructor::GraphicsInstructor(ThreadSynchronizer<FrameData>& synchronizer)
    : m_synchronizer(synchronizer),
      m_coordinates(ServiceRegistry::getInstance().getService<Coordinates>())
{
    CompDirty::g_dirtyEntities.reserve(10000);
    ObjectPool<CompGraphics>::reserve(1000);

    registerCallback(Event::Type::TICK, this, &GraphicsInstructor::onTick);
}

void GraphicsInstructor::onTick(const Event& e)
{
    onTickStart();
    updateGraphicComponents();
    sendGraphicsInstructions();
    onTickEnd();
}

void GraphicsInstructor::onTickStart()
{
    // Read and send data
    auto player = ServiceRegistry::getInstance().getService<PlayerController>()->getPlayer();
    m_synchronizer.getSenderFrameData().fogOfWar = *(player->getFogOfWar().get());
    m_synchronizer.getSenderFrameData().frameNumber = m_frameCount;
    m_coordinates->setViewportPositionInPixels(
        m_synchronizer.getSenderFrameData().viewportPositionInPixels);

    if (!m_initialized)
    {
        ServiceRegistry::getInstance().getService<StateManager>()->getEntities<CompDirty>().each(
            [this](uint32_t entity, CompDirty& dirty) { dirty.markDirty(entity); });
        m_initialized = true;
    }
}

void GraphicsInstructor::onTickEnd()
{
    m_frameCount++;
    m_synchronizer.waitForReceiver([&]() {});
    CompDirty::g_dirtyEntities.clear();
}

void GraphicsInstructor::sendGraphicsInstructions()
{
    auto state = ServiceRegistry::getInstance().getService<StateManager>();

    for (auto entity : CompDirty::g_dirtyEntities)
    {
        auto& gc = state->getComponent<CompGraphics>(entity);

        if (gc.bypass == false)
        {
            auto instruction = ObjectPool<CompGraphics>::acquire();
            *instruction = gc;

            if (gc.entityID != entt::null)
                sendGraphiInstruction(instruction);
            else
                spdlog::error("Invalid entity found during simulation for id: {}", gc.toString());
        }
    }
}

void GraphicsInstructor::updateGraphicComponents()
{
    m_synchronizer.getSenderFrameData().cursor = GraphicsID();

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
        gc.bypass = false;

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

        if (auto unit = state->tryGetComponent<CompUnit>(entity))
        {
            gc.isEnabled = unit->isGarrisoned == false;
        }

        if (auto cursorComp = state->tryGetComponent<CompCursor>(entity))
        {
            // Avoid sending cursor as a regular graphics instruction, but set to
            // frame data directly.
            gc.bypass = true;
            m_synchronizer.getSenderFrameData().cursor = cursorComp->cursor;
        }
    }
}

void GraphicsInstructor::sendGraphiInstruction(CompGraphics* instruction)
{
    m_synchronizer.getSenderFrameData().graphicUpdates.push_back(instruction);
}