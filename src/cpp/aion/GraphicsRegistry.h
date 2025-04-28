#ifndef GRAPHICSREGISTRY_H
#define GRAPHICSREGISTRY_H

#include "Types.h"
#include "Vec2d.h"
#include "WidthHeight.h"

#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

namespace aion
{
class GraphicsID
{
  public:
    int entityType = -1;                                  // 15bits: 0-32767
    int action = 0;                                   // 15bits: 0-32767
    int frame = 0;                                        // 5bits: 0-31
    utils::Direction direction = utils::Direction::NORTH; // 3bits: 0-7
    int entitySubType = 0;                                // 10bits: 0-1023
    int variation = 0;                                    // 10bits: 0-1023
    int custom3 = 0;                                      // 4bits: 0-15

    bool isValid() const
    {
        return entityType != -1 && action != -1 && frame != -1;
    }

    bool operator==(const GraphicsID& other) const
    {
        return entityType == other.entityType && action == other.action &&
               direction == other.direction && frame == other.frame &&
               entitySubType == other.entitySubType && variation == other.variation &&
               custom3 == other.custom3;
    }

    int64_t hashWithClearingFrame() const
    {
       int64_t h = hash();
       h &= ~(0x1F << 23); // Clear the frame bits
       return h;
    }

    int64_t hash() const
    {
        return (static_cast<int64_t>(entityType) << 46) |
               (static_cast<int64_t>(action) << 31) | (static_cast<int64_t>(direction) << 28) |
               (static_cast<int64_t>(frame) << 23) | (static_cast<int64_t>(entitySubType) << 13) |
               (static_cast<int64_t>(variation) << 3) | static_cast<int64_t>(custom3);
    }

    std::string toString() const
    {
        return "GraphicsID(" + std::to_string(entityType) + ", " + std::to_string(action) +
               ", " + std::to_string(frame) + ", " + std::to_string(static_cast<int>(direction)) +
               ", " + std::to_string(entitySubType) + ", " + std::to_string(variation) + ", " +
               std::to_string(custom3) + ")";
    }

    static GraphicsID fromHash(int64_t hash)
    {
        GraphicsID id;
        id.entityType = (hash >> 46) & 0x7FFF;                            // 15 bits
        id.action = (hash >> 31) & 0x7FFF;                             // 15 bits
        id.direction = static_cast<utils::Direction>((hash >> 28) & 0x7); // 3 bits
        id.frame = (hash >> 23) & 0x1F;                                    // 5 bits
        id.entitySubType = (hash >> 13) & 0x3FF;                           // 10 bits
        id.variation = (hash >> 3) & 0x3FF;                                // 10 bits
        id.custom3 = hash & 0xF;                                          // 4 bits
        return id;
    }
};

struct Texture
{
    SDL_Texture* image = nullptr;
    Vec2d anchor{0, 0};
    utils::WidthHeight size{0, 0};
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
    size_t getTextureCount() const
    {
        return m_textureMap.size();
    }
    auto getTextures() const
    {
        return m_textureMap;
    }
    
    void registerAnimation(const GraphicsID& graphicID, const Animation& entry);
    const Animation& getAnimation(const GraphicsID& graphicID) const;
    bool hasAnimation(const GraphicsID& graphicID) const
    {
        int64_t hash = graphicID.hashWithClearingFrame();

        return m_animationMap.find(hash) != m_animationMap.end();
    }
    size_t getAnimationCount() const
    {
        return m_animationMap.size();
    }

  private:
    std::unordered_map<int64_t, Texture> m_textureMap;
    std::unordered_map<int64_t, Animation> m_animationMap;
};
} // namespace aion

#endif