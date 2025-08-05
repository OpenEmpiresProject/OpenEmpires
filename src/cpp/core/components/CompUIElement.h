#ifndef COMPUIELEMENT_H
#define COMPUIELEMENT_H

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
    int64_t backgroundImage = 0;
    bool isVisible = true;
};
} // namespace core

#endif