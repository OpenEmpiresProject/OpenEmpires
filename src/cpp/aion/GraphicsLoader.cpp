#include "GraphicsLoader.h"

#include "GraphicsRegistry.h"
#include "Logger.h"
#include "WidthHeight.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <map>
#include <vector>

using namespace aion;
using namespace utils;
namespace fs = std::filesystem;
using namespace std;

GraphicsLoader::GraphicsLoader(SDL_Renderer* renderer,
                               GraphicsRegistry& graphicsRegistry,
                               AtlasGeneratorBase& atlasGenerator)
    : renderer_(renderer), graphicsRegistry(graphicsRegistry), atlasGenerator(atlasGenerator)
{
}

void GraphicsLoader::loadAllGraphics()
{
    loadTextures();
    adjustDirections();
    loadAnimations();
    loadCursors();
    setCursor(4);
}

void GraphicsLoader::loadTextures()
{
    spdlog::info("Loading textures from assets/images...");

    std::map<int, std::vector<std::filesystem::path>> entityTypeToPaths;
    int loadedCount = 0;

    // Group textures by entityType
    for (const auto& entry : fs::recursive_directory_iterator("assets/images"))
    {
        if (!entry.is_regular_file())
            continue;

        const auto& path = entry.path();
        int entityType = determineEntityType(path); // Implement a helper to determine entityType
        if (entityType == 0)
            continue; // Skip unknown entity types
        entityTypeToPaths[entityType].push_back(path);
    }

    // Create atlas textures for each entityType
    for (const auto& [entityType, paths] : entityTypeToPaths)
    {
        createAtlasForEntityType(entityType, paths);
        loadedCount += paths.size();
    }

    spdlog::info("{} textures loaded into atlases.", loadedCount);
}

int GraphicsLoader::determineEntityType(const std::filesystem::path& path)
{
    if (path.string().find("terrain") != std::string::npos)
        return 2; // Tile
    if (path.string().find("villager") != std::string::npos)
        return 3; // Villager
    if (path.string().find("trees") != std::string::npos)
        return 4; // Trees
    return 0;     // Unknown
}

void GraphicsLoader::createAtlasForEntityType(int entityType,
                                              const std::vector<std::filesystem::path>& paths)
{

    std::vector<SDL_Rect> srcRects;

    auto atlasTexture = atlasGenerator.generateAtlas(renderer_, entityType, paths, srcRects);
    if (!atlasTexture)
    {
        spdlog::error("Failed to create atlas texture for entity type {}.", entityType);
        return;
    }

    // Register textures in the graphics registry
    for (size_t i = 0; i < paths.size(); ++i)
    {
        const auto& path = paths[i];
        const auto& srcRect = srcRects[i];
        WidthHeight imageSize = {srcRect.w, srcRect.h};

        aion::GraphicsID id;
        id.entityType = entityType;
        id.action = 0; // Deduce action if applicable
        // id.variation = i; // Variation index
        id.direction = utils::Direction::NORTH; // Default direction

        Vec2d anchor = {imageSize.width / 2 + 1, imageSize.height};
        std::string pathStr = path.string();

        if (entityType == 2)
        {
            id.variation =
                variation++; // Different grass tiles. TODO: This doesn't scale or work well.
            id.direction = utils::Direction::NORTHEAST;
            anchor = {imageSize.width / 2 + 1, 0};
        }
        else if (entityType == 3)
        {
            auto actionFolder = path.parent_path().filename().string();
            auto actionStr = actionFolder.substr(0, actionFolder.find('_'));
            id.action = std::stoi(actionStr);

            /* if the file name is 1388_01.bmp pattern last two digit represents the frame. each
                direction animation contains 15 frames. 1-15 for south, 16-30 for southwest, 31-45
               for west, 46-60 for northwest, and so on. implement this logic to manipulate frame
               number and direction of id (type of GraphicsID) */
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
        }
        else if (entityType == 4) // Trees
        {
            for (size_t i = 0; i < 100; i++)
            {
                id.variation = i;
                if (!graphicsRegistry.hasTexture(id))
                {
                    break;
                }
            }
        }

        SDL_FRect* srcRectF = new SDL_FRect{(float) srcRect.x, (float) srcRect.y, (float) srcRect.w,
                                            (float) srcRect.h};

        aion::Texture entry{atlasTexture, srcRectF, anchor, imageSize, false};
        graphicsRegistry.registerTexture(id, entry);
    }
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

void aion::GraphicsLoader::loadCursors()
{
    for (const auto& entry : fs::recursive_directory_iterator("assets/images/interfaces/cursors"))
    {
        if (!entry.is_regular_file())
            continue;

        const auto& path = entry.path();
        string pathStr = path.string();

        // Load surface (supporting BMP/PNG/JPG etc. via SDL_image)
        SDL_Surface* surface = IMG_Load(pathStr.c_str());
        if (!surface)
        {
            spdlog::warn("Failed to load image: {}, error: {}", pathStr, SDL_GetError());
            return;
        }

        // apply color keying if needed (e.g., for .bmp files)
        if (path.extension() == ".bmp")
        {
            auto formatDetails = SDL_GetPixelFormatDetails(surface->format);
            auto palette = SDL_GetSurfacePalette(surface);
            auto rgb = SDL_MapRGB(formatDetails, palette, 0xFF, 0, 0xFF); // magenta key
            SDL_SetSurfaceColorKey(surface, true, rgb);
        }

        SDL_Cursor* cursor = SDL_CreateColorCursor(surface, 0, 0);

        if (!cursor)
        {
            spdlog::error("Failed to create color cursor: {}", SDL_GetError());
            return;
        }
        auto variationStr = pathStr.substr(pathStr.find_last_of('_') + 1, 2);
        auto variation = std::stoi(variationStr);
        cursors[variation] = cursor; // Store the cursor in the map
    }
}

void aion::GraphicsLoader::setCursor(int variation)
{
    SDL_SetCursor(cursors[variation]);
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
