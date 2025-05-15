#ifndef GRAPHICSREGISTRY_H
#define GRAPHICSREGISTRY_H

#include "Vec2d.h"
#include "utils/Types.h"
#include "utils/Size.h"

#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

namespace aion
{
class GraphicsID
{
  public:
    // Bit layout
    // 63            49 48         39 38        29 28     24 23       19 18         9 8          0
    //  +---------------+-------------+-------------+---------+-----------+-----------+----------+
    //  |  entityType   | entitySubTp |   action    | frame   | direction | variation | reserved |
    //  |   (15 bits)   |  (10 bits)  |  (10 bits)  | (5 bits)|  (4 bits) | (10 bits) | (9 bits) |
    //  +---------------+-------------+-------------+---------+-----------+-----------+----------+

    int entityType = 0;                    // 32,768 values
    int entitySubType = 0;                 // 1,024 values
    int action = 0;                        // 1,024 values
    int frame = 0;                         // 32 values
    Direction direction = Direction::NONE; // 16 values
    int variation = 0;                     // 1,024 values
    int reserved = 0;                      // 512 values

    bool isValid() const
    {
        return entityType != 0;
    }

    bool operator==(const GraphicsID& other) const
    {
        return entityType == other.entityType && action == other.action &&
               direction == other.direction && frame == other.frame &&
               entitySubType == other.entitySubType && variation == other.variation &&
               reserved == other.reserved;
    }

    int64_t hashWithClearingFrame() const
    {
        int64_t h = hash();
        h &= ~(0x1FLL << 24); // Clear 5-bit frame field
        return h;
    }

    int64_t hash() const
    {
        return (static_cast<int64_t>(entityType) << 49) |
               (static_cast<int64_t>(entitySubType) << 39) | (static_cast<int64_t>(action) << 29) |
               (static_cast<int64_t>(frame) << 24) | (static_cast<int64_t>(direction) << 19) |
               (static_cast<int64_t>(variation) << 9) | static_cast<int64_t>(reserved);
    }

    std::string toString() const
    {
        return "GraphicsID(T" + std::to_string(entityType) + ", S" + std::to_string(entitySubType) +
               ", A" + std::to_string(action) + ", F" + std::to_string(frame) + ", D" +
               std::to_string(static_cast<int>(direction)) + ", V" + std::to_string(variation) +
               ", " + std::to_string(reserved) + ")";
    }

    static GraphicsID fromHash(int64_t hash)
    {
        GraphicsID id;
        id.entityType = (hash >> 49) & 0x7FFF;                     // 15 bits
        id.entitySubType = (hash >> 39) & 0x3FF;                   // 10 bits
        id.action = (hash >> 29) & 0x3FF;                          // 10 bits
        id.frame = (hash >> 24) & 0x1F;                            // 5 bits
        id.direction = static_cast<Direction>((hash >> 19) & 0xF); // 4 bits
        id.variation = (hash >> 9) & 0x3FF;                        // 10 bits
        id.reserved = hash & 0x1FF;                                // 9 bits
        return id;
    }
};

struct Texture
{
    SDL_Texture* image = nullptr;
    SDL_FRect* srcRect = nullptr; // Source rectangle for the texture
    Vec2d anchor{0, 0};
    Size size{0, 0};
    bool flip = false;
};

struct Animation
{
    std::vector<int64_t> frames; // Frames in terms of TextureIDs
    bool repeatable = false;
    int speed = 10; // 10 FPS
};

class GraphicsRegistry
{
  public:
    GraphicsRegistry() = default;
    ~GraphicsRegistry() = default;

    void registerTexture(const GraphicsID& graphicID, const Texture& entry);
    const Texture& getTexture(const GraphicsID& graphicID) const;
    bool hasTexture(const GraphicsID& graphicID) const;
    size_t getTextureCount() const;
    const std::unordered_map<int64_t, Texture>& getTextures() const;
    void registerAnimation(const GraphicsID& graphicID, const Animation& entry);
    const Animation& getAnimation(const GraphicsID& graphicID) const;
    bool hasAnimation(const GraphicsID& graphicID) const;
    size_t getAnimationCount() const;

  private:
    std::unordered_map<int64_t, Texture> m_textureMap;
    std::unordered_map<int64_t, Animation> m_animationMap;
};
} // namespace aion

#endif