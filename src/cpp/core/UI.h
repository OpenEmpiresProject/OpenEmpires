#ifndef UI_H
#define UI_H

#include "Color.h"
#include "GraphicsRegistry.h"
#include "Property.h"
#include "Rect.h"
#include "utils/Types.h"

#include <string>
#include <vector>

#define DEFINE_GETTER_SETTER(Field, Capital)                                                       \
    const decltype(Field)& get##Capital() const                                                    \
    {                                                                                              \
        return Field;                                                                              \
    }                                                                                              \
    void set##Capital(const decltype(Field)& value)                                                \
    {                                                                                              \
        if (Field != value)                                                                        \
        {                                                                                          \
            dirty = true;                                                                          \
            Field = value;                                                                         \
        }                                                                                          \
    }                                                                                              \
    auto with##Capital(const decltype(Field)& value)                                               \
        -> decltype(std::static_pointer_cast<std::remove_reference_t<decltype(*this)>>(            \
            this->shared_from_this()))                                                             \
    {                                                                                              \
        set##Capital(value);                                                                       \
        return std::static_pointer_cast<std::remove_reference_t<decltype(*this)>>(                 \
            this->shared_from_this());                                                             \
    }

namespace core
{
class Event;

namespace ui
{
class Widget : public std::enable_shared_from_this<Widget>, public core::PropertyInitializer
{
  protected:
    const uint32_t id = 0;
    const Ref<Widget> parent;
    Rect<int> rect;
    std::string name;
    bool enabled = true;
    bool visible = true;
    bool hot = false;
    bool dirty = true;
    GraphicsID backgroundImage;
    std::vector<Ref<Widget>> children;
    // Indicate whether the parent's visibility. Internal use only.
    bool parentVisible = true;

  public:
    Widget(Ref<Widget> parent);

    template <typename T> Ref<T> createChild()
    {
        auto child = CreateRef<T>(shared_from_this());
        children.push_back(child);
        return child;
    }

    Ref<Widget> findChild(const std::string& name) const;

    virtual void feedInput(const Event& e);
    virtual void updateGraphicCommand();
    Rect<int> getAbsoluteRect() const;

    void setPos(int x, int y);
    void setSize(int width, int height);

    auto withPos(int x, int y)
    {
        setPos(x, y);
        return std::static_pointer_cast<std::remove_reference_t<decltype(*this)>>(
            this->shared_from_this());
    }

    auto withSize(int width, int height)
    {
        setSize(width, height);
        return std::static_pointer_cast<std::remove_reference_t<decltype(*this)>>(
            this->shared_from_this());
    }

    DEFINE_GETTER_SETTER(rect, Rect);
    DEFINE_GETTER_SETTER(name, Name);
    DEFINE_GETTER_SETTER(enabled, Enabled);
    DEFINE_GETTER_SETTER(visible, Visible);
    DEFINE_GETTER_SETTER(backgroundImage, BackgroundImage);

    static inline int s_entityType = 0;
    static inline int s_entitySubType = 0;

  private:
    bool inside(const Vec2& pos) const;
};

class Label : public Widget
{
  protected:
    std::string text;
    Color textColor;

  public:
    DEFINE_GETTER_SETTER(text, Text);
    DEFINE_GETTER_SETTER(textColor, TextColor);

    Label(Ref<Widget> parent);
    void updateGraphicCommand() override;
};

class Button : public Widget
{
  protected:
    std::string label;
    bool active = false;

    std::function<void()> onClick;

  public:
    DEFINE_GETTER_SETTER(label, Label);
    DEFINE_GETTER_SETTER(active, Active);

    Button(Ref<Widget> parent);
    void feedInput(const Event& e) override;
};

class Window : public Widget
{
  public:
    Window();
};

enum class LayoutDirection
{
    Horizontal,
    Vertical
};

class Layout : public Widget
{
  protected:
    uint32_t margin = 0;
    uint32_t spacing = 0;
    LayoutDirection direction = LayoutDirection::Horizontal;

  public:
    DEFINE_GETTER_SETTER(margin, Margin);
    DEFINE_GETTER_SETTER(spacing, Spacing);
    DEFINE_GETTER_SETTER(direction, Direction);

    Layout(Ref<Widget> parent);
    void updateGraphicCommand() override;
};

} // namespace ui

} // namespace core

#endif