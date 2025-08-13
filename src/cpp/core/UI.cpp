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

using namespace core;
using namespace core::ui;

Widget::Widget(Ref<Widget> parent)
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

    CompEntityInfo entityInfo(s_entityType, s_entitySubType, 0);
    entityInfo.entityId = id;
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(id, entityInfo);
}

void Widget::feedInput(const Event& e)
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

void Widget::updateGraphicCommand()
{
    if (dirty)
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
        ui.backgroundImage = backgroundImage;
        ui.isVisible = visible;
        if (backgroundImage.isValid())
            ui.type = UIRenderingType::TEXTURE;
        else
            ui.type = UIRenderingType::NONE;

        dirty.markDirty(id); // TODO: not optimal
        this->dirty = false;
    }

    for (auto& child : children)
    {
        child->updateGraphicCommand();
    }
}

Rect<int> Widget::getAbsoluteRect() const
{
    auto negativeTranslated = rect;

    if (parent != nullptr)
    {
        auto parentAbsRect = parent->getAbsoluteRect();

        if (rect.x < 0)
            negativeTranslated.x = parentAbsRect.w + rect.x - rect.w;
        if (rect.y < 0)
            negativeTranslated.y = parentAbsRect.h + rect.y - rect.h;

        return negativeTranslated.translated(parentAbsRect);
    }

    if (rect.x < 0 || rect.y < 0)
    {
        auto settings = ServiceRegistry::getInstance().getService<GameSettings>();

        if (rect.x < 0)
            negativeTranslated.x = settings->getWindowDimensions().width + rect.x - rect.w;
        if (rect.y < 0)
            negativeTranslated.y = settings->getWindowDimensions().height + rect.y - rect.h;
    }
    return negativeTranslated;
}

bool Widget::inside(const Vec2& pos) const
{
    return getAbsoluteRect().contains(pos);
}

Ref<Widget> Widget::findChild(const std::string& name) const
{
    for (auto& child : children)
    {
        if (child->name == name)
        {
            return child;
        }
        else
        {
            auto match = child->findChild(name);
            if (match != nullptr)
                return match;
        }
    }
    return nullptr;
}

Label::Label(Ref<Widget> parent) : Widget(parent)
{
}

void Label::updateGraphicCommand()
{
    /*
     *   Approach: Determine whether the label is dirty by checking previous value (from Component)
     *   and new value (from UI element). Then perform common operations in Widget. If earlier, it
     *   was determined that the label is dirty, Widget::updateGraphicCommand will update the
     *   ECS and mark the entity as dirty, so that Simulator would pick it up to send graphic
     *   instructions.
     *   NOTE: Override ui.type has to be done after Widget::updateGraphicCommand, since Widget
     *   only knows to draw rectangles.
     *
     */
    auto& ui =
        ServiceRegistry::getInstance().getService<GameState>()->getComponent<CompUIElement>(id);
    // if (ui.text != text || ui.backgroundImage != backgroundImage)
    //     dirty = true;

    Widget::updateGraphicCommand(); //  This will reset dirty flag

    ui.text = text;
    ui.color = textColor;

    if (backgroundImage.isValid() == false)
        ui.type = UIRenderingType::TEXT;
}

Button::Button(Ref<Widget> parent) : Widget(parent)
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

Window::Window() : Widget(nullptr)
{
}

Layout::Layout(Ref<Widget> parent) : Widget(parent)
{
}

void Layout::updateGraphicCommand()
{
    if (direction == LayoutDirection::Horizontal)
    {
        int newX = margin;
        for (auto& child : children)
        {
            auto& rect = child->getRect();
            rect.x = newX;
            rect.y = margin;
            child->setRect(rect);
            newX += rect.w + spacing;
        }
    }
    else if (direction == LayoutDirection::Vertical)
    {
        int newY = margin;
        for (auto& child : children)
        {
            auto& rect = child->getRect();
            rect.y = newY;
            rect.x = margin;
            child->setRect(rect);
            newY += rect.h + spacing;
        }
    }
    Widget::updateGraphicCommand();
}
