#include "GraphicsLoader.h"

#include "GraphicsRegistry.h"
#include "utils/Logger.h"
#include "utils/Size.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cctype>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace aion;
namespace fs = std::filesystem;
using namespace std;

GraphicsLoader::GraphicsLoader(SDL_Renderer* renderer,
                               GraphicsRegistry& graphicsRegistry,
                               AtlasGenerator& atlasGenerator)
    : m_renderer(renderer), m_graphicsRegistry(graphicsRegistry), m_atlasGenerator(atlasGenerator)
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

std::vector<Vec2d> loadAnchorsFromCSV(const std::filesystem::path& filepath)
{
    std::vector<Vec2d> anchors;
    std::ifstream file(filepath);
    std::string line;

    auto trim = [](std::string_view s) -> std::string_view
    {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
            ++start;
        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
            --end;
        return s.substr(start, end - start);
    };

    while (std::getline(file, line))
    {
        std::string_view sv = trim(line);
        if (sv.empty())
            continue;

        size_t comma = sv.find(',');
        if (comma == std::string_view::npos)
            continue;

        std::string_view x_str = trim(sv.substr(0, comma));
        std::string_view y_str = trim(sv.substr(comma + 1));

        int x = 0, y = 0;
        auto [px, ecx] = std::from_chars(x_str.data(), x_str.data() + x_str.size(), x);
        auto [py, ecy] = std::from_chars(y_str.data(), y_str.data() + y_str.size(), y);
        if (ecx == std::errc() && ecy == std::errc())
        {
            anchors.emplace_back(Vec2d(x, y));
        }
    }

    return anchors;
}

void GraphicsLoader::loadTextures()
{
    spdlog::info("Loading textures from assets/images...");

    std::map<int, std::vector<std::filesystem::path>> entityTypeToPaths;
    std::map<std::string, Vec2d> anchorsByFileName;
    int loadedCount = 0;

    // Group textures by entityType
    for (const auto& entry : fs::recursive_directory_iterator("assets/images"))
    {
        if (!entry.is_regular_file())
            continue;

        std::vector<Vec2d> anchors;

        if (entry.path().extension() == ".csv")
        {
            anchors = loadAnchorsFromCSV(entry.path());

            for (size_t i = 0; i < anchors.size(); i++)
            {
                auto ii = i + 1;
                auto imageName =
                    entry.path().stem().string() + "_" + (ii < 10 ? "0" : "") + std::to_string(ii);
                anchorsByFileName[imageName] = anchors[i];
            }
            continue;
        }

        const auto& path = entry.path();
        int entityType = determineEntityType(path); // Implement a helper to determine entityType
        if (entityType == 0)
            continue; // Skip unknown entity types
        entityTypeToPaths[entityType].push_back(path);
    }

    // Create atlas textures for each entityType
    for (const auto& [entityType, paths] : entityTypeToPaths)
    {
        createAtlasForEntityType(entityType, paths, anchorsByFileName);
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
    if (path.string().find("mill") != std::string::npos)
        return 5; // Mill
    if (path.string().find("marketplace") != std::string::npos)
        return 6;
    if (path.string().find("tower") != std::string::npos)
        return 7;
    return 0; // Unknown
}

void GraphicsLoader::createAtlasForEntityType(int entityType,
                                              const std::vector<std::filesystem::path>& paths,
                                              const std::map<std::string, Vec2d>& anchors)
{

    std::vector<SDL_Rect> srcRects;

    auto atlasTexture = m_atlasGenerator.generateAtlas(m_renderer, entityType, paths, srcRects);
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
        Size imageSize = {srcRect.w, srcRect.h};

        GraphicsID id;
        id.entityType = entityType;
        id.action = 0; // Deduce action if applicable
        // id.m_variation = i; // Variation index

        Vec2d anchor = {imageSize.width / 2 + 1, imageSize.height};
        std::string pathStr = path.string();

        if (entityType == 2)
        {
            auto imageName = path.stem().string();
            auto frameId = std::stoi(imageName.substr(imageName.find_last_of('_') + 1, 3));
            id.variation = frameId;
            anchor = {imageSize.width / 2 + 1, 0};
        }
        else if (entityType == 3)
        {
            // get anchor from anchors based on filename
            auto imageName = path.stem().string();
            if (anchors.find(imageName) != anchors.end())
            {
                anchor = anchors.at(imageName);
            }

            auto actionFolder = path.parent_path().filename().string();
            auto actionStr = actionFolder.substr(0, actionFolder.find('_'));
            id.action = std::stoi(actionStr);

            /* if the file name is 1388_01.bmp pattern last two digit represents the frame. each
                direction animation contains 15 frames. 1-15 for south, 16-30 for southwest, 31-45
               for west, 46-60 for northwest, and so on. implement this logic to manipulate frame
               number and direction of id (type of GraphicsID) */
            auto index = std::stoi(pathStr.substr(pathStr.find_last_of('_') + 1, 2)) - 1;
            id.frame = index % 15;               // 0-14
            auto direction = (int) (index / 15); // 0-3
            if (direction == 0)
            {
                id.direction = Direction::SOUTH;
            }
            else if (direction == 1)
            {
                id.direction = Direction::SOUTHWEST;
            }
            else if (direction == 2)
            {
                id.direction = Direction::WEST;
            }
            else if (direction == 3)
            {
                id.direction = Direction::NORTHWEST;
            }
            else if (direction == 4)
            {
                id.direction = Direction::NORTH;
            }
        }
        else if (entityType == 4) // Trees
        {
            for (size_t i = 0; i < 100; i++)
            {
                id.variation = i;
                if (!m_graphicsRegistry.hasTexture(id))
                {
                    break;
                }
            }
        }
        else if (entityType == 5)
        {
        }

        SDL_FRect* srcRectF = new SDL_FRect{(float) srcRect.x, (float) srcRect.y, (float) srcRect.w,
                                            (float) srcRect.h};

        Texture entry{atlasTexture, srcRectF, anchor, imageSize, false};
        m_graphicsRegistry.registerTexture(id, entry);
    }
}

void GraphicsLoader::loadAnimations()
{
    for (const auto& [id, texture] : m_graphicsRegistry.getTextures())
    {
        auto idFull = GraphicsID::fromHash(id);
        // Find all textures with the same entityType, action, and direction
        std::vector<int64_t> animationFrames;
        for (const auto& [otherId, otherTexture] : m_graphicsRegistry.getTextures())
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
            m_graphicsRegistry.registerAnimation(animationID, animation);

            // spdlog::debug(
            //     "Animation created for entityType: {}, action: {}, direction: {} with {}
            //     frames.", idFull.entityType, idFull.action, static_cast<int>(idFull.direction),
            //     animationFrames.size());
        }
    }
}

void GraphicsLoader::loadCursors()
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
        auto m_variation = std::stoi(variationStr);
        m_cursors[m_variation] = cursor; // Store the cursor in the map
    }
}

void GraphicsLoader::setCursor(int variation)
{
    SDL_SetCursor(m_cursors[variation]);
}

void GraphicsLoader::adjustDirections()
{
    std::list<std::pair<GraphicsID, Texture>> graphicsToFlip;
    for (const auto& [id, texture] : m_graphicsRegistry.getTextures())
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
            static_cast<Direction>(getFlippedDirection(id.direction)); // Flip the direction

        m_graphicsRegistry.registerTexture(id, texture);
    }
}

bool GraphicsLoader::isTextureFlippingNeededEntity(int entityType) const
{
    return entityType == 3;
}

bool GraphicsLoader::isTextureFlippingNeededDirection(Direction direction) const
{
    return direction == Direction::SOUTHWEST || direction == Direction::NORTHWEST ||
           direction == Direction::WEST;
}

Direction GraphicsLoader::getFlippedDirection(Direction direction) const
{
    switch (direction)
    {
    case Direction::SOUTHWEST:
        return Direction::SOUTHEAST;
    case Direction::NORTHWEST:
        return Direction::NORTHEAST;
    case Direction::WEST:
        return Direction::EAST;
    default:
        return direction; // No flipping needed
    }
}
