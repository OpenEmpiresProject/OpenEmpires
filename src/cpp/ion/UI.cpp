#include "UI.h"

#include "Coordinates.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "components/CompUIElement.h"

using namespace ion;
using namespace ion::ui;

Element::Element(const GraphicsID& graphicsId, Ref<Element> parent)
    : id(GameState::getInstance().createEntity()), parent(parent)
{
    GameState::getInstance().addComponent(id, CompUIElement());
    GameState::getInstance().addComponent(id, CompTransform());

    CompGraphics graphics;
    graphics.entityID = id;
    graphics.layer = GraphicLayer::UI;
    GameState::getInstance().addComponent(id, graphics);
    GameState::getInstance().addComponent(id, CompRendering());
    GameState::getInstance().addComponent(id, CompDirty());
    GameState::getInstance().addComponent(
        id, CompEntityInfo(graphicsId.entityType, graphicsId.entitySubType, graphicsId.variation));
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
        GameState::getInstance()
            .getComponents<CompUIElement, CompTransform, CompEntityInfo, CompDirty>(id);
    transform.position = getAbsoluteRect().position();
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

bool Element::inside(const Vec2d& pos) const
{
    return getAbsoluteRect().contains(pos);
}

void Label::updateGraphicCommand()
{
    Element::updateGraphicCommand();

    auto& ui = GameState::getInstance().getComponent<CompUIElement>(id);
    ui.text = text;
    ui.type = UIRenderingType::TEXT;
}

std::vector<Ref<Element>> Window::g_windows;

Window::Window(const GraphicsID& id) : Element(id, nullptr)
{
}
