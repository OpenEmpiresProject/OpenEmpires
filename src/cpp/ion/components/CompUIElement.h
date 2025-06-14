#ifndef COMPUIELEMENT_H
#define COMPUIELEMENT_H

#include "Rect.h"

#include <string>

namespace ion
{
enum class UIRenderingType
{
    TEXT,
    TEXTURE,
    RECT
};

class CompUIElement
{
  public:
    UIRenderingType type;
    std::string text;
    Rect<int> rect;
    Color color;
};
} // namespace ion

#endif