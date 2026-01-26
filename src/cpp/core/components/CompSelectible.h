#ifndef COMPSELECTIBLE_H
#define COMPSELECTIBLE_H

#include "Feet.h"
#include "GraphicAddon.h"
#include "GraphicsRegistry.h"
#include "Property.h"
#include "Rect.h"
#include "utils/Size.h"
#include "utils/Types.h"
#include "logging/Logger.h"
#include "Flat2DArray.h"

namespace core
{
class CompSelectible
{
  public:
    using Rect2DArray = Flat2DArray<Rect<int>>;
    Property<GraphicsID> icon;
    Property<std::string> displayName;
    Property<Rect2DArray> boundingBoxes;

  public:
    // NOTE: Rect's x, y doesn't represent the position of the rect, but only the local
    // anchor. Bounding box alone doesn't have a location concept anyhow, location comes
    // from the transform component
    GraphicAddon selectionIndicator;
    bool isSelected = false;

    const Rect<int>& getBoundingBox(Direction direction) const
    {
        return boundingBoxes.value().at(0, (int) direction);
    }

    const Rect<int>& getBoundingBox(size_t state, Direction direction) const
    {
        if (state < boundingBoxes.value().width())
        {
            return boundingBoxes.value().at(state, (int) direction);
        }
        else
        {
            // A bit of forgiving - return the first state's bounding box
            spdlog::warn("Requested bounding box for invalid state {}", state);
            return boundingBoxes.value().at(0, (int) direction);
        }
    }
};
} // namespace core

#endif