#include "UI.h"

#include "Coordinates.h"
#include "Event.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "UIManager.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "components/CompUIElement.h"

using namespace core;
using namespace core::ui;

Widget::Widget(Ref<Widget> parent)
    : id(ServiceRegistry::getInstance().getService<StateManager>()->createEntity()), parent(parent)
{
    ServiceRegistry::getInstance().getService<StateManager>()->addComponent(id, CompUIElement());
    ServiceRegistry::getInstance().getService<StateManager>()->addComponent(id, CompTransform());

    CompGraphics graphics;
    graphics.entityID = id;
    graphics.layer = GraphicLayer::UI;
    ServiceRegistry::getInstance().getService<StateManager>()->addComponent(id, graphics);
    ServiceRegistry::getInstance().getService<StateManager>()->addComponent(id, CompRendering());

    CompEntityInfo entityInfo(s_entityType, s_entitySubType, 0);
    entityInfo.entityId = id;
    ServiceRegistry::getInstance().getService<StateManager>()->addComponent(id, entityInfo);
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
    bool wasDirty = dirty;

    if (dirty)
    {
        auto coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
        auto [ui, transform, info] =
            ServiceRegistry::getInstance()
                .getService<StateManager>()
                ->getComponents<CompUIElement, CompTransform, CompEntityInfo>(id);
        auto pixelPos = getAbsoluteRect().position();
        // HACK: We are hacking transform's position to carry UI element positions as well.
        // But it is usually meant to carry positions in Feet.
        transform.position = {pixelPos.x, pixelPos.y};
        ui.rect = getAbsoluteRect();
        ui.backgroundImage = backgroundImage;
        ui.isVisible = visible && parentVisible;
        if (backgroundImage.isValid())
            ui.type = UIRenderingType::TEXTURE;
        else
            ui.type = UIRenderingType::NONE;

        StateManager::markDirty(id); // TODO: not optimal
        this->dirty = false;
    }

    // OPTIMIZATION
    // Either the parent should be visible in order to display children or
    // this is a dirty tick (where parent might be getting hidden) to let children
    // update graphic commands accordingly
    //
    if (visible || wasDirty)
    {
        for (auto& child : children)
        {
            // Stretch child to fit to this widget
            auto& childRect = child->getRect();
            int newWidth = childRect.w;
            int newHeight = childRect.h;

            if (childRect.w == 0)
            {
                newWidth = rect.w;
            }
            if (childRect.h == 0)
            {
                newHeight = rect.h;
            }
            // Both the child's dirty and parentVisible should be based parent (this)
            // Eg:
            // 1) If I am dirty then my children are dirty too
            // 2) If I am hidden or my parent is hidden, then my children are hidden too
            //
            child->dirty |= wasDirty;
            child->parentVisible = visible && parentVisible;
            child->setSize(newWidth, newHeight);
            child->updateGraphicCommand();
        }
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
        auto settings = ServiceRegistry::getInstance().getService<Settings>();

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

void Widget::setPos(int x, int y)
{
    rect.x = x;
    rect.y = y;
}

void Widget::setSize(int width, int height)
{
    rect.w = width;
    rect.h = height;
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
        ServiceRegistry::getInstance().getService<StateManager>()->getComponent<CompUIElement>(id);
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
    /*
     *   Approach: Check whether visible children of this layout would exceed the size
     *   (width or height depending on the layout direction). Then calculate the offset
     *   that each child has to bring back (i.e. adjust) to avoid overflowing children.
     *   These new positions will be assigned forcefully to children (only the width
     *   and height of children will be preserved).
     */

    if (direction == LayoutDirection::Horizontal)
    {
        int overflowNegationOffset = 0;

        if (rect.w > 0)
        {
            int totalWidthRequiredForChildren = 0;
            int visibleChildCount = 0;
            for (auto& child : children)
            {
                if (child->getVisible())
                {
                    totalWidthRequiredForChildren += child->getRect().w + spacing;
                    visibleChildCount++;
                }
            }

            int availableTotalWidth = rect.w - 2 * margin; // Margins are on both ends
            int overflowWidth = std::max(0, totalWidthRequiredForChildren - availableTotalWidth);
            overflowNegationOffset = overflowWidth / std::max(1, visibleChildCount);
        }

        int newX = leftMargin >= 0 ? leftMargin : margin;
        int i = 0;
        for (auto& child : children)
        {
            if (child->getVisible())
            {
                // Taking a copy to modify coordinates
                auto rect = child->getRect();
                rect.x = newX;
                rect.y = topMargin >= 0 ? topMargin : margin;
                child->setRect(rect);
                newX += rect.w + spacing - (overflowNegationOffset * 1);
            }
        }
    }
    else if (direction == LayoutDirection::Vertical)
    {
        int overflowNegationOffset = 0;

        if (rect.h > 0)
        {
            int totalHeightRequiredForChildren = 0;
            int visibleChildCount = 0;
            for (auto& child : children)
            {
                if (child->getVisible())
                {
                    totalHeightRequiredForChildren += child->getRect().h + spacing;
                    visibleChildCount++;
                }
            }

            int availableTotalHeight = rect.h - 2 * margin; // Margins are on both ends
            int overflowHeight = std::max(0, totalHeightRequiredForChildren - availableTotalHeight);
            overflowNegationOffset = overflowHeight / std::max((size_t) 1, children.size());
        }

        int newY = topMargin >= 0 ? topMargin : margin;
        int i = 0;
        for (auto& child : children)
        {
            if (child->getVisible())
            {
                // Taking a copy to modify coordinates
                auto rect = child->getRect();
                rect.y = newY;
                rect.x = leftMargin >= 0 ? leftMargin : margin;
                child->setRect(rect);
                newY += rect.h + spacing - (overflowNegationOffset * 1);
            }
        }
    }
    Widget::updateGraphicCommand();
}
