#ifndef COMPUIELEMENT_H
#define COMPUIELEMENT_H

#include "GraphicsRegistry.h"
#include "Rect.h"

#include <string>

namespace core
{
enum class UIRenderingType
{
    NONE,
    TEXT,
    TEXTURE
};

class CompUIElement
{
  public:
    UIRenderingType type;
    std::string text;
    Rect<int> rect;
    Color color;
    uint32_t id = 0;
    GraphicsID backgroundImage;
    bool isVisible = true;
};
} // namespace core

#endif