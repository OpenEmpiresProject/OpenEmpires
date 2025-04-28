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
    adjustDirections();
    loadAnimations();
    loadCursor(4);
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

    if (!texture)
    {
        SDL_DestroySurface(surface);
        spdlog::warn("Failed to convert surface to texture: {}, error: {}", pathStr,
                     SDL_GetError());
        return;
    }

    // Infer or assign graphics ID based on filename or a mapping file
    aion::GraphicsID id;
    id.frame = 0;
    id.entitySubType = 0;
    Vec2d anchor = {imageSize.width / 2 + 1, imageSize.height};

    if (pathStr.find("terrain") != std::string::npos)
    {
        id.action = 0;
        id.entityType = 2;          // Tile
        id.variation = variation++; // Different grass tiles. TODO: This doesn't scale or work well.
        id.direction = utils::Direction::NORTHEAST;
        anchor = {imageSize.width / 2 + 1, 0};

        SDL_DestroySurface(surface);
    }
    else if (pathStr.find("villager") != std::string::npos)
    {
        id.entityType = 3;

        // Deduce action from the folder name (e.g., "0_idle" -> action = 0, "1_walk" -> action = 1)
        auto actionFolder = path.parent_path().filename().string();
        auto actionStr = actionFolder.substr(0, actionFolder.find('_'));
        id.action = std::stoi(actionStr);

        /* if the file name is 1388_01.bmp pattern last two digit represents the frame. each
         direction animation contains 15 frames. 1-15 for south, 16-30 for southwest, 31-45 for
         west, 46-60 for northwest, and so on. implement this logic to manipulate frame number and
         direction of id (type of GraphicsID) */
        auto index = std::stoi(pathStr.substr(pathStr.find_last_of('_') + 1, 2)) - 1; // 0-14
        id.frame = index % 15;
        auto direction = (int) (index / 15); // 0-3
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
        SDL_DestroySurface(surface);

    } else if (pathStr.find("cursor") != std::string::npos)
    {
        id.entityType = 1;
        anchor = {0, 0};
        // if the file name of cursors are like 51000_01.bmp extract last two digits and assign it to variation field

        auto indexStr = pathStr.substr(pathStr.find_last_of('_') + 1, 2);
        auto index = std::stoi(indexStr);
        id.variation = index;

        // TODO: temp hack
        loadedSurfaces[id.hash()] = surface; // Store the surface for later use
    }
    else
    {
        SDL_DestroySurface(surface);

        spdlog::warn("Unknown texture type: {}", pathStr);
        return;
    }

    aion::Texture entry{texture,
                        anchor,
                        imageSize}; // TODO: Load this anchor from configs
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
                idFull.action == otherIdFull.action && idFull.direction == otherIdFull.direction)
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

            spdlog::debug(
                "Animation created for entityType: {}, action: {}, direction: {} with {} frames.",
                idFull.entityType, idFull.action, static_cast<int>(idFull.direction),
                animationFrames.size());
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

void aion::GraphicsLoader::adjustDirections()
{
    std::list<std::pair<GraphicsID, Texture>> graphicsToFlip;
    for (const auto& [id, texture] : graphicsRegistry.getTextures())
    {
        GraphicsID idFull = GraphicsID::fromHash(id);
        if (isTextureFlippingNeededEntity(idFull.entityType) &&
            isTextureFlippingNeededDirection(idFull.direction))
        {
            graphicsToFlip.push_back(std::make_pair(idFull, texture));
        }
    }

    for (auto& [id, texture] : graphicsToFlip)
    {
        texture.flip = true; // Mark the texture for flipping
        id.direction =
            static_cast<utils::Direction>(getFlippedDirection(id.direction)); // Flip the direction

        graphicsRegistry.registerTexture(id, texture);
    }
}

bool aion::GraphicsLoader::isTextureFlippingNeededEntity(int entityType) const
{
    return entityType == 3;
}

bool aion::GraphicsLoader::isTextureFlippingNeededDirection(utils::Direction direction) const
{
    return direction == utils::Direction::SOUTHWEST || direction == utils::Direction::NORTHWEST ||
           direction == utils::Direction::WEST;
}

utils::Direction aion::GraphicsLoader::getFlippedDirection(utils::Direction direction) const
{
    switch (direction)
    {
    case utils::Direction::SOUTHWEST:
        return utils::Direction::SOUTHEAST;
    case utils::Direction::NORTHWEST:
        return utils::Direction::NORTHEAST;
    case utils::Direction::WEST:
        return utils::Direction::EAST;
    default:
        return direction; // No flipping needed
    }
}

void aion::GraphicsLoader::loadCursor(int variation)
{
    // use variation, look cursor in registry and set it as colored cursor in SDL

    auto it = loadedSurfaces.find(GraphicsID(1, 0, 0, utils::Direction::NORTH, 0, variation, 0).hash());
    if (it == loadedSurfaces.end())
    {
        spdlog::error("Cursor surface not found for variation: {}", variation);
        return;
    }
    SDL_Surface* surface = it->second;
    SDL_Cursor* cursor = SDL_CreateColorCursor(surface, 0, 0);

    if (!cursor)
    {
        spdlog::error("Failed to create color cursor: {}", SDL_GetError());
        return;
    }

    SDL_SetCursor(cursor);
}
