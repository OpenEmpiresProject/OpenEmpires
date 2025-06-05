#include "GraphicsLoaderFromDRS.h"

#include "DRSFile.h"
#include "SLPFile.h"
#include "utils/Logger.h"

#include <memory>
#include <string>
#include <unordered_map>

using namespace game;
using namespace std;
using namespace aion;
using namespace drs;

void loadSLP(shared_ptr<DRSFile> drs,
             uint32_t slpId,
             uint32_t entityType,
             uint32_t action,
             SDL_Renderer* renderer,
             GraphicsRegistry& graphicsRegistry,
             AtlasGenerator& atlasGenerator);
shared_ptr<DRSFile> loadDRSFile(const string& drsFilename);
void adjustDirections(GraphicsRegistry& graphicsRegistry);

void GraphicsLoaderFromDRS::loadAllGraphics(SDL_Renderer* renderer,
                                            aion::GraphicsRegistry& graphicsRegistry,
                                            aion::AtlasGenerator& atlasGenerator)
{
    auto terrainDRS = loadDRSFile("assets/terrain.drs");
    auto graphicsDRS = loadDRSFile("assets/graphics.drs");

    loadSLP(terrainDRS, 15001, 2, 0, renderer, graphicsRegistry, atlasGenerator); // Grass tiles
    loadSLP(graphicsDRS, 1388, 3, 0, renderer, graphicsRegistry, atlasGenerator); // Villager idle
    loadSLP(graphicsDRS, 1392, 3, 1, renderer, graphicsRegistry, atlasGenerator); // Villager walk
    loadSLP(graphicsDRS, 1254, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1256, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1258, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1260, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1262, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1264, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1266, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1268, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1270, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 1272, 4, 0, renderer, graphicsRegistry, atlasGenerator); // Tree
    loadSLP(graphicsDRS, 3483, 5, 0, renderer, graphicsRegistry, atlasGenerator); // Mill
    loadSLP(graphicsDRS, 2278, 6, 0, renderer, graphicsRegistry, atlasGenerator); // Marketplace

    adjustDirections(graphicsRegistry);
}

SDL_Surface* frameToSurface(const Frame& frame)
{
    const auto& image = frame.getImage();
    auto [width, height] = frame.getDimensions();

    if (image.empty() || width <= 0 || height <= 0)
    {
        return nullptr;
    }

    // Create a surface with 32-bit RGBA format (alpha will be set to 255)
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!surface)
    {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        return nullptr;
    }

    if (SDL_LockSurface(surface) < 0)
    {
        SDL_Log("Failed to lock surface: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        return nullptr;
    }

    uint8_t* pixels = static_cast<uint8_t*>(surface->pixels);
    int pitch = surface->pitch;

    auto formatDetails = SDL_GetPixelFormatDetails(surface->format);
    auto palette = SDL_GetSurfacePalette(surface);
    auto rgb = SDL_MapRGB(formatDetails, palette, 0xFF, 0, 0xFF); // magenta key
    SDL_SetSurfaceColorKey(surface, true, rgb);

    for (int y = 0; y < height; ++y)
    {
        uint32_t* row = reinterpret_cast<uint32_t*>(pixels + y * pitch);
        for (int x = 0; x < width; ++x)
        {
            const Color& c = image[y][x];
            row[x] = SDL_MapRGB(formatDetails, palette, c.r, c.g, c.b);
        }
    }

    SDL_UnlockSurface(surface);
    return surface;
}

shared_ptr<DRSFile> loadDRSFile(const string& drsFilename)
{
    auto drs = make_shared<DRSFile>();
    if (!drs->load(drsFilename))
    {
        throw runtime_error("Failed to load DRS file: " + drsFilename);
    }
    return std::move(drs);
}

void loadSLP(shared_ptr<DRSFile> drs,
             uint32_t slpId,
             uint32_t entityType,
             uint32_t action,
             SDL_Renderer* renderer,
             GraphicsRegistry& graphicsRegistry,
             AtlasGenerator& atlasGenerator)
{
    auto slp = drs->getSLPFile(slpId);

    std::vector<SDL_Surface*> surfaces;

    auto frames = slp.getFrames();
    for (auto& frame : frames)
    {
        auto surface = frameToSurface(frame);
        surfaces.push_back(surface);
    }

    std::vector<SDL_Rect> srcRects;

    auto atlasTexture = atlasGenerator.generateAtlas(renderer, surfaces, srcRects);
    if (!atlasTexture)
    {
        spdlog::error("Failed to create atlas texture for entity type {}.", entityType);
        return;
    }

    for (size_t i = 0; i < surfaces.size(); ++i)
    {
        const auto& srcRect = srcRects[i];
        Size imageSize = {srcRect.w, srcRect.h};

        GraphicsID id;
        id.entityType = entityType;
        id.action = action;
        auto anchorPair = frames[i].getAnchor();
        Vec2d anchor = {anchorPair.first, anchorPair.second};

        if (entityType == 2)
        {
            id.variation = frames[i].getId();
            anchor = {imageSize.width / 2 + 1,
                      0}; // Must override tile anchoring since their anchors don't work here
        }
        else if (entityType == 3)
        {
            /* if the file name is 1388_01.bmp pattern last two digit represents the frame. each
                direction animation contains 15 frames. 1-15 for south, 16-30 for southwest, 31-45
               for west, 46-60 for northwest, and so on. implement this logic to manipulate frame
               number and direction of id (type of GraphicsID) */
            auto index = frames[i].getId() - 1;
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
                if (!graphicsRegistry.hasTexture(id))
                {
                    break;
                }
            }
        }

        SDL_FRect* srcRectF = new SDL_FRect{(float) srcRect.x, (float) srcRect.y, (float) srcRect.w,
                                            (float) srcRect.h};

        Texture entry{atlasTexture, srcRectF, anchor, imageSize, false};
        graphicsRegistry.registerTexture(id, entry);
    }
}

bool isTextureFlippingNeededEntity(int entityType)
{
    return entityType == 3;
}

bool isTextureFlippingNeededDirection(Direction direction)
{
    return direction == Direction::SOUTHWEST || direction == Direction::NORTHWEST ||
           direction == Direction::WEST;
}

Direction getFlippedDirection(Direction direction)
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

void adjustDirections(GraphicsRegistry& graphicsRegistry)
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
            static_cast<Direction>(getFlippedDirection(id.direction)); // Flip the direction

        graphicsRegistry.registerTexture(id, texture);
    }
}
