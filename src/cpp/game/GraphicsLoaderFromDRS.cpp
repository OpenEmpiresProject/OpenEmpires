#include "GraphicsLoaderFromDRS.h"

#include "DRSFile.h"
#include "EntityDefinitionLoader.h"
#include "GameTypes.h"
#include "SLPFile.h"
#include "ServiceRegistry.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/Types.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

using namespace game;
using namespace std;
using namespace ion;
using namespace drs;

void loadSLP(shared_ptr<DRSFile> drs,
             uint32_t slpId,
             uint32_t entityType,
             uint32_t entitySubType,
             uint32_t action,
             uint32_t playerId,
             SDL_Renderer* renderer,
             GraphicsRegistry& graphicsRegistry,
             AtlasGenerator& atlasGenerator,
             Rect<int> clipRect = Rect<int>());
shared_ptr<DRSFile> loadDRSFile(const string& drsFilename);
void adjustDirections(GraphicsRegistry& graphicsRegistry);
void registerDummyTexture(int entityType, int entitySubType, GraphicsRegistry& graphicsRegistry);

void GraphicsLoaderFromDRS::loadAllGraphics(SDL_Renderer* renderer,
                                            GraphicsRegistry& graphicsRegistry,
                                            AtlasGenerator& atlasGenerator)
{
    registerDummyTexture(EntityTypes::ET_UI_ELEMENT, EntitySubTypes::UI_LABEL, graphicsRegistry);
    registerDummyTexture(EntityTypes::ET_UI_ELEMENT, EntitySubTypes::UI_BUTTON, graphicsRegistry);
}

void GraphicsLoaderFromDRS::loadGraphics(SDL_Renderer* renderer,
                                         GraphicsRegistry& graphicsRegistry,
                                         AtlasGenerator& atlasGenerator,
                                         const std::list<GraphicsID>& idsToLoad)
{
    auto entityFactory = ServiceRegistry::getInstance().getService<EntityFactory>();
    Ref<EntityDefinitionLoader> defLoader =
        dynamic_pointer_cast<EntityDefinitionLoader>(entityFactory);

    std::map<int64_t, GraphicsID> uniqueBaseIds;
    for (auto& id : idsToLoad)
    {
        auto baseId = id.getBaseId();
        uniqueBaseIds[baseId.hash()] = id;
    }

    for (auto& id : uniqueBaseIds)
    {
        auto baseId = GraphicsID::fromHash(id.first);
        baseId.playerId = 0;

        auto drsData = defLoader->getDRSData(baseId);
        if (drsData.drsFile != nullptr)
        {
            loadSLP(drsData.drsFile, drsData.slpId, id.second.entityType, id.second.entitySubType,
                    id.second.action, id.second.playerId, renderer, graphicsRegistry,
                    atlasGenerator, drsData.clipRect);
        }
    }
    adjustDirections(graphicsRegistry);
}

void registerDummyTexture(int entityType, int entitySubType, GraphicsRegistry& graphicsRegistry)
{
    GraphicsID id;
    id.entityType = entityType;
    id.entitySubType = entitySubType;
    SDL_FRect* srcRectF = new SDL_FRect{0, 0, 0, 0};
    Texture entry{nullptr, srcRectF, {0, 0}, Size(), false};
    graphicsRegistry.registerTexture(id, entry);
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

    if (SDL_LockSurface(surface) == false)
    {
        SDL_Log("Failed to lock surface: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        return nullptr;
    }

    uint8_t* pixels = static_cast<uint8_t*>(surface->pixels);
    int pitch = surface->pitch;

    auto formatDetails = SDL_GetPixelFormatDetails(surface->format);
    auto palette = SDL_GetSurfacePalette(surface);
    auto keyColor = SDL_MapRGBA(formatDetails, palette, 0xFF, 0, 0xFF, 0xFF); // magenta key
    SDL_SetSurfaceColorKey(surface, true, keyColor);

    for (int y = 0; y < height; ++y)
    {
        uint32_t* row = reinterpret_cast<uint32_t*>(pixels + y * pitch);
        for (int x = 0; x < width; ++x)
        {
            const drs::Color& c = image[y][x];
            /*
             * DRS library uses two key colors;
             *   1) 255,0,255 for typical background removal
             *   2) 255,0,254 for semi-transparent black color drawing (currently used for shadows)
             */
            if (c.r == 0xFF && c.g == 0 && c.b == 0xFE)
                row[x] = SDL_MapRGBA(formatDetails, palette, 0, 0, 0, 100);
            else
                row[x] = SDL_MapRGBA(formatDetails, palette, c.r, c.g, c.b, 0xFF);
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
    return drs;
}

void loadSLP(shared_ptr<DRSFile> drs,
             uint32_t slpId,
             uint32_t entityType,
             uint32_t entitySubType,
             uint32_t action,
             uint32_t playerId,
             SDL_Renderer* renderer,
             GraphicsRegistry& graphicsRegistry,
             AtlasGenerator& atlasGenerator,
             Rect<int> clipRect)
{
    auto slp = drs->getSLPFile(slpId);

    std::vector<SDL_Surface*> surfaces;

    auto frames = slp.getFrames(playerId);
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
        auto& srcRect = srcRects[i];

        if (clipRect.w != 0 && clipRect.h != 0)
        {
            srcRect.w = min(clipRect.w, srcRect.w);
            srcRect.h = min(clipRect.h, srcRect.h);

            srcRect.x = clipRect.x;
            srcRect.y = clipRect.y;
        }

        Size imageSize = {srcRect.w, srcRect.h};

        GraphicsID id;
        id.entityType = entityType;
        id.entitySubType = entitySubType;
        id.action = action;
        id.playerId = playerId;

        if (playerId == 1)
            int rerer = 0;

        if (playerId == 2)
            int rerer = 0;

        auto anchorPair = frames[i].getAnchor();
        Vec2 anchor(anchorPair.first, anchorPair.second);

        if (entityType == EntityTypes::ET_TILE)
        {
            id.variation = frames[i].getId();
            anchor = Vec2(imageSize.width / 2 + 1,
                          0); // Must override tile anchoring since their anchors don't work here
        }
        else if (entityType == EntityTypes::ET_VILLAGER)
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
        else
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
    return entityType == EntityTypes::ET_VILLAGER;
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
        texture.anchor.x = texture.size.width - texture.anchor.x;
        texture.flip = true; // Mark the texture for flipping
        id.direction =
            static_cast<Direction>(getFlippedDirection(id.direction)); // Flip the direction

        if (graphicsRegistry.hasTexture(id) == false)
            graphicsRegistry.registerTexture(id, texture);
    }
}
