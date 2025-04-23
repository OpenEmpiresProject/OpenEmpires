#include "GraphicsLoader.h"

#include "GraphicsRegistry.h"
#include "Logger.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

using namespace aion;
namespace fs = std::filesystem;

GraphicsLoader::GraphicsLoader(SDL_Renderer* renderer, GraphicsRegistry& graphicsRegistry)
    : renderer_(renderer), graphicsRegistry(graphicsRegistry)
{
}

void GraphicsLoader::loadAllGraphics()
{
    spdlog::info("Loading textures from assets/images...");

    int variation = 0;

    for (const auto& entry : fs::recursive_directory_iterator("assets/images"))
    {
        if (!entry.is_regular_file())
            continue;
        loadGraphics(entry.path());
    }
    spdlog::info("All textures loaded.");
}

void GraphicsLoader::loadGraphics(const std::filesystem::path& path)
{
    std::string pathStr = path.string();

    // Load surface (supporting BMP/PNG/JPG etc. via SDL_image)
    SDL_Surface* surface = IMG_Load(pathStr.c_str());
    if (!surface)
    {
        spdlog::warn("Failed to load image: {}, error: {}", pathStr, SDL_GetError());
        return;
    }

    // Optional: apply color keying if needed (e.g., for .bmp files)
    if (path.extension() == ".bmp")
    {
        auto formatDetails = SDL_GetPixelFormatDetails(surface->format);
        auto palette = SDL_GetSurfacePalette(surface);
        auto rgb = SDL_MapRGB(formatDetails, palette, 0xFF, 0, 0xFF); // magenta key
        SDL_SetSurfaceColorKey(surface, true, rgb);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_DestroySurface(surface);

    if (!texture)
    {
        spdlog::warn("Failed to convert surface to texture: {}, error: {}", pathStr,
                     SDL_GetError());
        return;
    }

    // Infer or assign graphics ID based on filename or a mapping file
    aion::GraphicsID id;
    id.entitytType = 2; // Tile
    id.actionType = 0;
    id.frame = 0;
    id.entitySubType = 0;       // Grass
    id.variation = variation++; // Different grass tiles. TODO: This doesn't scale or work well.
    id.direction = utils::Direction::NORTH;

    aion::GraphicsEntry entry{texture, {0, 0}};
    graphicsRegistry.registerGraphic(id, entry);
}
