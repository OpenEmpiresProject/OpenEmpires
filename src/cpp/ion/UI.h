#ifndef UI_H
#define UI_H

#include "Color.h"
#include "Rect.h"
#include "utils/Types.h"

#include <string>
#include <vector>

namespace ion
{
class Event;
class GraphicsID;

namespace ui
{
struct Element : std::enable_shared_from_this<Element>
{
    const uint32_t id = 0;
    const Ref<Element> parent;
    Rect<int> rect;
    std::string name;
    bool enabled = true;
    bool visible = true;
    bool hot = false;
    Color background;
    std::vector<Ref<Element>> children;

    Element(const GraphicsID& id, Ref<Element> parent);

    template <typename T> Ref<T> createChild(const GraphicsID& graphicsId)
    {
        auto child = CreateRef<T>(graphicsId, shared_from_this());
        children.push_back(child);
        return child;
    }

    virtual void feedInput(const Event& e);
    virtual void updateGraphicCommand();
    Rect<int> getAbsoluteRect() const;

  private:
    bool inside(const Vec2& pos) const;
};

struct Label : public Element
{
    std::string text;

    Label(const GraphicsID& id, Ref<Element> parent);
    void updateGraphicCommand() override;
};

struct Button : public Element
{
    std::string label;
    bool active = false;

    std::function<void()> onClick;

    Button(const GraphicsID& id, Ref<Element> parent);
    void feedInput(const Event& e) override;
};

struct Window : public Element
{
    Window(const GraphicsID& id);
};
} // namespace ui

} // namespace ion

#endif