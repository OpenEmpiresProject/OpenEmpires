#include "UI.h"

#include "Coordinates.h"
#include "Event.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "UIManager.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "components/CompUIElement.h"

using namespace ion;
using namespace ion::ui;

Element::Element(const GraphicsID& graphicsId, Ref<Element> parent)
    : id(ServiceRegistry::getInstance().getService<GameState>()->createEntity()), parent(parent)
{
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(id, CompUIElement());
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(id, CompTransform());

    CompGraphics graphics;
    graphics.entityID = id;
    graphics.layer = GraphicLayer::UI;
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(id, graphics);
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(id, CompRendering());
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(id, CompDirty());

    CompEntityInfo entityInfo(graphicsId.entityType, graphicsId.entitySubType,
                              graphicsId.variation);
    entityInfo.entityId = id;
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(id, entityInfo);
}

void Element::feedInput(const Event& e)
{
    if (visible && enabled)
    {
        if (e.type == Event::Type::MOUSE_MOVE)
        {
            auto data = e.getData<MouseMoveData>();
            hot = inside(data.screenPos);
        }

        if (hot)
        {
            for (auto& child : children)
            {
                child->feedInput(e);
            }
        }
    }
}

void Element::updateGraphicCommand()
{
    auto coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
    auto [ui, transform, info, dirty] =
        ServiceRegistry::getInstance()
            .getService<GameState>()
            ->getComponents<CompUIElement, CompTransform, CompEntityInfo, CompDirty>(id);
    auto pixelPos = getAbsoluteRect().position();
    // HACK: We are hacking transform's position to carry UI element positions as well.
    // But it is usually meant to carry positions in Feet.
    transform.position = {pixelPos.x, pixelPos.y};
    ui.rect = getAbsoluteRect();
    ui.color = background;
    ui.type = UIRenderingType::RECT;
    dirty.markDirty(id); // TODO: not optimal

    for (auto& child : children)
    {
        child->updateGraphicCommand();
    }
}

Rect<int> Element::getAbsoluteRect() const
{
    if (parent != nullptr)
    {
        return rect.translated(parent->getAbsoluteRect());
    }
    return rect;
}

bool Element::inside(const Vec2& pos) const
{
    return getAbsoluteRect().contains(pos);
}

Label::Label(const GraphicsID& id, Ref<Element> parent) : Element(id, parent)
{
}

void Label::updateGraphicCommand()
{
    Element::updateGraphicCommand();

    auto& ui =
        ServiceRegistry::getInstance().getService<GameState>()->getComponent<CompUIElement>(id);
    ui.text = text;
    ui.type = UIRenderingType::TEXT;
}

Button::Button(const GraphicsID& id, Ref<Element> parent) : Element(id, parent)
{
}

void Button::feedInput(const Event& e)
{
    if (e.type == Event::Type::MOUSE_BTN_DOWN && hot)
    {
        active = true;
    }
    else if (e.type == Event::Type::MOUSE_BTN_UP)
    {
        if (active)
        {
            active = false;
            onClick();
        }
    }
}

Window::Window(const GraphicsID& id) : Element(id, nullptr)
{
}
