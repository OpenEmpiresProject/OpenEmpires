#ifndef GRAPHICSREGISTRY_H
#define GRAPHICSREGISTRY_H

#include "Types.h"
#include "Vec2d.h"

#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

namespace aion
{
class GraphicsID
{
  public:
    int entitytType = 0;                                  // 15bits: 0-32767
    int actionType = 0;                                   // 15bits: 0-32767
    int frame = 0;                                        // 5bits: 0-31
    utils::Direction direction = utils::Direction::NORTH; // 3bits: 0-7
    int entitySubType = 0;                                // 10bits: 0-1023
    int variation = 0;                                    // 10bits: 0-1023
    int custom3 = 0;                                      // 4bits: 0-15

    GraphicsID() = default;
    GraphicsID(int entityType, int actionType) : entitytType(entityType), actionType(actionType)
    {
    }
    GraphicsID(int entityType, int actionType, int frame)
        : entitytType(entityType), actionType(actionType), frame(frame)
    {
    }
    GraphicsID(int entityType, int actionType, int frame, utils::Direction direction)
        : entitytType(entityType), actionType(actionType), frame(frame), direction(direction)
    {
    }

    bool operator==(const GraphicsID& other) const
    {
        return entitytType == other.entitytType && actionType == other.actionType &&
               direction == other.direction && frame == other.frame &&
               entitySubType == other.entitySubType && variation == other.variation &&
               custom3 == other.custom3;
    }

    int64_t hash() const
    {
        return (static_cast<int64_t>(entitytType) << 46) |
               (static_cast<int64_t>(actionType) << 31) | (static_cast<int64_t>(direction) << 26) |
               (static_cast<int64_t>(frame) << 23) | (static_cast<int64_t>(entitySubType) << 13) |
               (static_cast<int64_t>(variation) << 3) | static_cast<int64_t>(custom3);
    }

    std::string toString() const
    {
        return "GraphicsID(" + std::to_string(entitytType) + ", " + std::to_string(actionType) +
               ", " + std::to_string(frame) + ", " + std::to_string(static_cast<int>(direction)) +
               ", " + std::to_string(entitySubType) + ", " + std::to_string(variation) + ", " +
               std::to_string(custom3) + ")";
    }

    // TODO: validate this
    static GraphicsID fromHash(int64_t hash)
    {
        GraphicsID id;
        id.entitytType = (hash >> 46) & 0x7FFF;                            // 15 bits
        id.actionType = (hash >> 31) & 0x7FFF;                             // 15 bits
        id.direction = static_cast<utils::Direction>((hash >> 26) & 0x07); // 3 bits
        id.frame = (hash >> 23) & 0x1F;                                    // 5 bits
        id.entitySubType = (hash >> 13) & 0x3FF;                           // 10 bits
        id.variation = (hash >> 3) & 0x3FF;                                // 10 bits
        id.custom3 = hash & 0x0F;                                          // 4 bits
        return id;
    }
};

struct GraphicsEntry
{
    SDL_Texture* image = nullptr;
    Vec2d anchor{0, 0};
};

class GraphicsRegistry
{
  public:
    GraphicsRegistry() = default;
    ~GraphicsRegistry() = default;

    void registerGraphic(const GraphicsID& graphicID, const GraphicsEntry& entry);
    const GraphicsEntry& getGraphic(const GraphicsID& graphicID) const;

  private:
    std::unordered_map<int64_t, GraphicsEntry> graphicsMap;
};
} // namespace aion

#endif