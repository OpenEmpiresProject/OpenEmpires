#include "GraphicsLoader.h"

#include "GraphicsRegistry.h"
#include "Logger.h"
#include "WidthHeight.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

using namespace aion;
using namespace utils;
namespace fs = std::filesystem;

GraphicsLoader::GraphicsLoader(SDL_Renderer* renderer, GraphicsRegistry& graphicsRegistry)
    : renderer_(renderer), graphicsRegistry(graphicsRegistry)
{
}

void GraphicsLoader::loadAllGraphics()
{
    loadTextures();
    loadAnimations();
}

void GraphicsLoader::loadTexture(const std::filesystem::path& path)
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
    WidthHeight imageSize = {surface->w, surface->h};
    SDL_DestroySurface(surface);

    if (!texture)
    {
        spdlog::warn("Failed to convert surface to texture: {}, error: {}", pathStr,
                     SDL_GetError());
        return;
    }

    // Infer or assign graphics ID based on filename or a mapping file
    aion::GraphicsID id;
    id.action = 0;
    id.frame = 0;
    id.entitySubType = 0;

    if (pathStr.find("terrain") != std::string::npos)
    {
        id.entityType = 2; // Tile
        id.variation = variation++; // Different grass tiles. TODO: This doesn't scale or work well.
        id.direction = utils::Direction::NORTH;
    }
    else if (pathStr.find("villager") != std::string::npos)
    {
        id.entityType = 3;
        /* if the file name is 1388_01.bmp pattern last two digit represents the frame. each direction animation
         contains 15 frames. 1-15 for south, 16-30 for southwest, 31-45 for west, 46-60 for northwest, and so on.
         implement thie logic to manipulate frame number and direction of id (type of GraphicsID)*/
        auto index = std::stoi(pathStr.substr(pathStr.find_last_of('_') + 1, 2)) - 1; // 0-14
        id.frame = index % 15;
        auto direction = (int)(index / 15); // 0-3
        if (direction == 0)
        {
            id.direction = utils::Direction::SOUTH;
        }
        else if (direction == 1)
        {
            id.direction = utils::Direction::SOUTHWEST;
        }
        else if (direction == 2)
        {
            id.direction = utils::Direction::WEST;
        }
        else if (direction == 3)
        {
            id.direction = utils::Direction::NORTHWEST;
        }
        else if (direction == 4)
        {
            id.direction = utils::Direction::NORTH;
        }

        spdlog::debug("Village image size: {}x{}", imageSize.width, imageSize.height);
    }
    
    aion::Texture entry{texture, {imageSize.width / 2 + 1, imageSize.height}, imageSize}; // TODO: Load this anchor from configs
    graphicsRegistry.registerTexture(id, entry);
}

void aion::GraphicsLoader::loadAnimations()
{
    for (const auto& [id, texture] : graphicsRegistry.getTextures())
    {
        auto idFull = GraphicsID::fromHash(id);
        // Find all textures with the same entityType, action, and direction
        std::vector<int64_t> animationFrames;
        for (const auto& [otherId, otherTexture] : graphicsRegistry.getTextures())
        {
            auto otherIdFull = GraphicsID::fromHash(otherId);
            if (idFull.entityType == otherIdFull.entityType &&
                idFull.action == otherIdFull.action &&
                idFull.direction == otherIdFull.direction)
            {
                animationFrames.push_back(otherId);
            }
        }

        if (!animationFrames.empty())
        {
            GraphicsID animationID;
            animationID.entityType = idFull.entityType;
            animationID.action = idFull.action;
            animationID.direction = idFull.direction;
            Animation animation{animationFrames, true, 10}; // 10 FPS
            graphicsRegistry.registerAnimation(animationID, animation);

            spdlog::debug("Animation created for entityType: {}, action: {}, direction: {} with {} frames.",
                         idFull.entityType, idFull.action, static_cast<int>(idFull.direction), animationFrames.size());
        }
    }

}

void aion::GraphicsLoader::loadTextures()
{
    spdlog::info("Loading textures from assets/images...");

    int variation = 0;
    int loadedCount = 0;

    for (const auto& entry : fs::recursive_directory_iterator("assets/images"))
    {
        if (!entry.is_regular_file())
            continue;
        loadTexture(entry.path());
        ++loadedCount;
    }
    spdlog::info("{} textures loaded.", loadedCount);
}
