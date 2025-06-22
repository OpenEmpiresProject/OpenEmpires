#ifndef COMPSELECTIBLE_H
#define COMPSELECTIBLE_H

#include "Feet.h"
#include "GraphicAddon.h"
#include "Rect.h"
#include "utils/Size.h"
#include "utils/Types.h"

namespace ion
{
struct CompSelectible
{
    // NOTE: Rect's x, y doesn't represent the position of the rect, but only the local
    // anchor. Bounding box alone doesn't have a location concept anyhow, location comes
    // from the tranform component
    Rect<int> boundingBoxes[static_cast<int>(Direction::NONE) + 1];

    GraphicAddon selectionIndicator;

    bool isSelected = false;

    const Rect<int>& getBoundingBox(Direction direction) const
    {
        return boundingBoxes[static_cast<int>(direction)];
    }
};
} // namespace ion

#endif