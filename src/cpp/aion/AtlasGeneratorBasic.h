#ifndef ATLASGENERATORBASIC_H
#define ATLASGENERATORBASIC_H

#include "AtlasGenerator.h"
#include "utils/Logger.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

namespace aion
{
class AtlasGeneratorBasic : public AtlasGenerator
{
  public:
    SDL_Texture* generateAtlas(SDL_Renderer* renderer,
                               int entityType,
                               const std::vector<std::filesystem::path>& images,
                               std::vector<SDL_Rect>& sourceRects) override
    {
        // Load all surfaces for this entityType
        std::vector<SDL_Surface*> surfaces;
        for (const auto& path : images)
        {
            SDL_Surface* surface = IMG_Load(path.string().c_str());
            if (!surface)
            {
                spdlog::warn("Failed to load image: {}, error: {}", path.string(), SDL_GetError());
                continue;
            }
            surfaces.push_back(surface);
        }

        if (surfaces.empty())
            return nullptr;

        // Calculate atlas dimensions
        int atlasWidth = 0, atlasHeight = 0;
        for (const auto& surface : surfaces)
        {
            atlasWidth = std::max(atlasWidth, surface->w);
            atlasHeight += surface->h;
        }

        // Create the atlas surface
        SDL_Surface* atlasSurface = SDL_CreateSurface(atlasWidth, atlasHeight, surfaces[0]->format);
        if (!atlasSurface)
        {
            spdlog::error("Failed to create atlas surface for entity type {}. {}", entityType,
                          SDL_GetError());
            return nullptr;
        }

        // Set the palette to the atlas surface
        if (!SDL_SetSurfacePalette(atlasSurface, SDL_GetSurfacePalette(surfaces[0])))
        {
            spdlog::warn("Failed to set palette for atlas surface: {}", SDL_GetError());
            return nullptr;
        }

        // TODO: images are assumed to be bmp type
        auto formatDetails = SDL_GetPixelFormatDetails(atlasSurface->format);
        auto palette = SDL_GetSurfacePalette(atlasSurface);
        auto rgb = SDL_MapRGB(formatDetails, palette, 0xFF, 0, 0xFF); // magenta key
        SDL_SetSurfaceColorKey(atlasSurface, true, rgb);

        // Blit individual surfaces onto the atlas and calculate srcRects
        int yOffset = 0;
        for (const auto& surface : surfaces)
        {
            SDL_Rect destRect = {0, yOffset, surface->w, surface->h};
            SDL_BlitSurface(surface, nullptr, atlasSurface, &destRect);
            sourceRects.push_back(destRect);
            yOffset += surface->h;
            SDL_DestroySurface(surface);
        }

        // Create texture from the atlas surface
        SDL_Texture* atlasTexture = SDL_CreateTextureFromSurface(renderer, atlasSurface);
        SDL_DestroySurface(atlasSurface);

        if (!atlasTexture)
        {
            spdlog::error("Failed to create atlas texture for entity type {}. {}", entityType,
                          SDL_GetError());
            return nullptr;
        }
        return atlasTexture;
    }
};
} // namespace aion

#endif