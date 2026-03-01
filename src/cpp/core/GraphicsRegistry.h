#ifndef GRAPHICSREGISTRY_H
#define GRAPHICSREGISTRY_H

#include "GraphicsID.h"
#include "utils/Size.h"
#include "utils/Types.h"

#include <SDL3/SDL.h>
#include <fmt/core.h>
#include <string>
#include <unordered_map>

namespace core
{
struct Texture
{
    SDL_Texture* image = nullptr;
    SDL_FRect* srcRect = nullptr; // Source rectangle for the texture
    Vec2 anchor;
    Size size{0, 0};
    bool flip = false;
    SDL_Cursor* cursor = nullptr;
};

// struct Animation
//{
//     std::vector<int64_t> frames; // Frames in terms of TextureIDs
//     bool repeatable = false;
//     int speed = 10; // FPS
// };

class GraphicsRegistry
{
  public:
    void registerTexture(const GraphicsID& graphicID, const Texture& entry);
    const Texture& getTexture(const GraphicsID& graphicID) const;
    bool hasTexture(const GraphicsID& graphicID) const;
    size_t getTextureCount() const;
    const std::unordered_map<GraphicsID, Texture>& getTextures() const;
    size_t getVariationCount(const GraphicsID& graphicID) const;

  private:
    std::unordered_map<GraphicsID, Texture> m_textureMap;

#ifndef NDEBUG
    std::unordered_map<int, std::vector<GraphicsID>> m_graphicIdsByEntityType;
#endif
};
} // namespace core

#endif