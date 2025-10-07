#include "GraphicsLoaderFromDRS.h"

#include "AtlasGenerator.h"
#include "DRSFile.h"
#include "EntityDefinitionLoader.h"
#include "EntityTypeRegistry.h"
#include "GameTypes.h"
#include "SLPFile.h"
#include "ServiceRegistry.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/Types.h"

#include <SDL3_image/SDL_image.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

using namespace game;
using namespace std;
using namespace core;
using namespace drs;

void loadSLP(shared_ptr<DRSFile> drs,
             uint32_t slpId,
             uint32_t entityType,
             uint32_t entitySubType,
             uint32_t action,
             uint32_t playerId,
             SDL_Renderer& renderer,
             GraphicsRegistry& graphicsRegistry,
             AtlasGenerator& atlasGenerator,
             Rect<int> clipRect = Rect<int>());
void loadSLP(const EntityDefinitionLoader::EntityDRSData& drs,
             const GraphicsID& baseId,
             SDL_Renderer& renderer,
             GraphicsRegistry& graphicsRegistry,
             AtlasGenerator& atlasGenerator);
shared_ptr<DRSFile> loadDRSFile(const string& drsFilename);
void adjustDirections(GraphicsRegistry& graphicsRegistry);
void registerDummyTexture(int entityType, int entitySubType, GraphicsRegistry& graphicsRegistry);

void GraphicsLoaderFromDRS::loadAllGraphics(SDL_Renderer& renderer,
                                            GraphicsRegistry& graphicsRegistry,
                                            AtlasGenerator& atlasGenerator)
{
    registerDummyTexture(EntityTypes::ET_UI_ELEMENT, EntitySubTypes::EST_DEFAULT, graphicsRegistry);
}

void GraphicsLoaderFromDRS::loadGraphics(SDL_Renderer& renderer,
                                         GraphicsRegistry& graphicsRegistry,
                                         AtlasGenerator& atlasGenerator,
                                         const std::list<GraphicsID>& idsToLoad)
{
    auto entityFactory = ServiceRegistry::getInstance().getService<EntityFactory>();
    Ref<EntityDefinitionLoader> defLoader =
        dynamic_pointer_cast<EntityDefinitionLoader>(entityFactory);

    std::set<GraphicsID> uniqueBaseIds;
    for (auto& id : idsToLoad)
    {
        auto baseId = id.getBaseId();
        uniqueBaseIds.insert(baseId);
    }

    for (auto& id : uniqueBaseIds)
    {
        auto baseId = id;
        baseId.playerId = 0;

        auto drsData = defLoader->getDRSData(baseId);

        if (drsData.parts.size() == 1)
        {
            if (drsData.parts[0].drsFile != nullptr)
            {
                loadSLP(drsData.parts[0].drsFile, drsData.parts[0].slpId, id.entityType,
                        id.entitySubType, id.action, id.playerId, renderer, graphicsRegistry,
                        atlasGenerator, drsData.clipRect);
            }
        }
        else // Multi-part texture
        {
            loadSLP(drsData, id, renderer, graphicsRegistry, atlasGenerator);
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

SDL_Surface* merge_surfaces_with_anchors(const std::vector<SDL_Surface*>& surfaces,
                                         const std::vector<Vec2>& anchors,
                                         int minWidth,
                                         int minHeight)
{
    if (surfaces.empty() || surfaces.size() != anchors.size())
    {
        spdlog::error(
            "Invalid arguments: surfaces and anchors must match in size, and must not be empty.");
        return nullptr;
    }

    if (!surfaces[0])
    {
        spdlog::error("First surface is null. Cannot determine pixel format.");
        return nullptr;
    }

    // Step 1: Compute bounding box with pivot (250,250)
    const int pivotX = 250;
    const int pivotY = 250;

    int minX = INT_MAX, minY = INT_MAX;
    int maxX = INT_MIN, maxY = INT_MIN;

    for (size_t i = 0; i < surfaces.size(); i++)
    {
        auto* surf = surfaces[i];
        if (!surf)
            continue;

        // 🔄 Reversed anchor: subtract anchor instead of add
        int left = pivotX - anchors[i].x;
        int top = pivotY - anchors[i].y;
        int right = left + surf->w;
        int bottom = top + surf->h;

        minX = std::min(minX, left);
        minY = std::min(minY, top);
        maxX = std::max(maxX, right);
        maxY = std::max(maxY, bottom);
    }

    if (minX == INT_MAX || minY == INT_MAX)
    {
        spdlog::error("No valid surfaces.");
        return nullptr;
    }

    int finalW = max(maxX - minX, minWidth);
    int finalH = max(maxY - minY, minHeight);

    // Step 2: Create the final surface with same format as first surface
    SDL_Surface* final_surface = SDL_CreateSurface(finalW, finalH, surfaces[0]->format);
    if (!final_surface)
    {
        spdlog::error("Surface creation failed: {}", SDL_GetError());
        return nullptr;
    }

    // Step 2b: Set magenta as colorkey and fill
    auto fmtDetails = SDL_GetPixelFormatDetails(final_surface->format);
    auto palette = SDL_GetSurfacePalette(final_surface);
    Uint32 colorkey = SDL_MapRGB(fmtDetails, palette, 0xFF, 0, 0xFF);

    SDL_SetSurfaceColorKey(final_surface, true, colorkey);
    SDL_FillSurfaceRect(final_surface, nullptr, colorkey);

    // Step 3: Blit each surface shifted into positive coords
    for (size_t i = 0; i < surfaces.size(); i++)
    {
        auto* surf = surfaces[i];
        if (!surf)
            continue;

        SDL_Rect dest;
        dest.x = pivotX - anchors[i].x - minX;
        dest.y = pivotY - anchors[i].y - minY;
        dest.w = surf->w;
        dest.h = surf->h;

        if (SDL_BlitSurface(surf, nullptr, final_surface, &dest) < 0)
        {
            spdlog::error("Blit failed for surface {}: {}", i, SDL_GetError());
        }
    }

    return final_surface; // caller frees
}

void loadSurfaces(AtlasGenerator& atlasGenerator,
                  SDL_Renderer& renderer,
                  std::vector<SDL_Surface*>& surfaces,
                  uint32_t entityType,
                  Rect<int> clipRect,
                  uint32_t entitySubType,
                  uint32_t action,
                  uint32_t playerId,
                  const std::vector<Frame>& frames,
                  const std::vector<Vec2>& anchors,
                  GraphicsRegistry& graphicsRegistry)
{
    std::vector<SDL_Rect> srcRects;

    auto atlasTexture = atlasGenerator.generateAtlas(renderer, surfaces, srcRects);
    if (!atlasTexture)
    {
        spdlog::error("Failed to create atlas texture for entity type {}.", entityType);
        return;
    }

    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

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

        Vec2 anchor = anchors[i];

        if (entityType == EntityTypes::ET_TILE)
        {
            id.variation = frames[i].getId();
            anchor = Vec2(imageSize.width / 2 + 1,
                          0); // Must override tile anchoring since their anchors don't work here
        }
        else if (typeRegistry->isAUnit(entityType))
        {
            /* if the file name is 1388_01.bmp pattern last two digit represents the frame. each
                direction animation can contains 15 (max) frames. 1-15 for south, 16-30 for
               southwest, 31-45 for west, 46-60 for northwest, and so on. implement this logic to
               manipulate frame number and direction of id (type of GraphicsID) */
            auto index = frames[i].getId() - 1;
            auto framesPerDirection = frames.size() / 5;
            id.frame = index % framesPerDirection;               // 0-14 (max)
            auto direction = (int) (index / framesPerDirection); // 0-4
            if (direction == 0)
            {
                id.direction = static_cast<uint64_t>(Direction::SOUTH);
            }
            else if (direction == 1)
            {
                id.direction = static_cast<uint64_t>(Direction::SOUTHWEST);
            }
            else if (direction == 2)
            {
                id.direction = static_cast<uint64_t>(Direction::WEST);
            }
            else if (direction == 3)
            {
                id.direction = static_cast<uint64_t>(Direction::NORTHWEST);
            }
            else if (direction == 4)
            {
                id.direction = static_cast<uint64_t>(Direction::NORTH);
            }
        }
        else
        {
            for (size_t i = 0; i < 1024; i++)
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

void loadSLP(const EntityDefinitionLoader::EntityDRSData& drsData,
             const GraphicsID& baseId,
             SDL_Renderer& renderer,
             GraphicsRegistry& graphicsRegistry,
             AtlasGenerator& atlasGenerator)
{
    std::vector<SDL_Surface*> surfaces;
    std::vector<Vec2> anchors;
    std::vector<drs::Frame> frames;

    for (auto& part : drsData.parts)
    {
        auto slp = part.drsFile->getSLPFile(part.slpId);
        auto framesPerPart = slp.getFrames(baseId.playerId);
        for (auto& frame : framesPerPart)
        {
            auto surface = frameToSurface(frame);
            surfaces.push_back(surface);

            if (part.anchor.isNull())
            {
                auto anchorPair = frame.getAnchor();
                Vec2 anchor(anchorPair.first, anchorPair.second);
                anchors.push_back(anchor);
            }
            else
            {
                anchors.push_back(part.anchor);
            }

            frames.push_back(frame);
        }
    }

    auto mergedSurface =
        merge_surfaces_with_anchors(surfaces, anchors, drsData.anchor.x, drsData.anchor.y);
    std::vector<Vec2> newAnchors = {drsData.anchor};

    std::vector<SDL_Surface*> mergedSurfaceVec = {mergedSurface};
    loadSurfaces(atlasGenerator, renderer, mergedSurfaceVec, baseId.entityType, drsData.clipRect,
                 baseId.entitySubType, baseId.action, baseId.playerId, frames, newAnchors,
                 graphicsRegistry);
}

void loadSLP(shared_ptr<DRSFile> drs,
             uint32_t slpId,
             uint32_t entityType,
             uint32_t entitySubType,
             uint32_t action,
             uint32_t playerId,
             SDL_Renderer& renderer,
             GraphicsRegistry& graphicsRegistry,
             AtlasGenerator& atlasGenerator,
             Rect<int> clipRect)
{
    auto slp = drs->getSLPFile(slpId);

    std::vector<SDL_Surface*> surfaces;
    std::vector<Vec2> anchors;

    auto frames = slp.getFrames(playerId);
    for (auto& frame : frames)
    {
        auto surface = frameToSurface(frame);
        surfaces.push_back(surface);
        auto anchorPair = frame.getAnchor();
        anchors.push_back(Vec2(anchorPair.first, anchorPair.second));
    }

    loadSurfaces(atlasGenerator, renderer, surfaces, entityType, clipRect, entitySubType, action,
                 playerId, frames, anchors, graphicsRegistry);
}

bool isTextureFlippingNeededEntity(int entityType)
{
    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
    return typeRegistry->isAUnit(entityType);
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
        if (isTextureFlippingNeededEntity(id.entityType) &&
            isTextureFlippingNeededDirection(static_cast<Direction>(id.direction)))
        {
            graphicsToFlip.push_back(std::make_pair(id, texture));
        }
    }

    for (auto& [id, texture] : graphicsToFlip)
    {
        texture.anchor.x = texture.size.width - texture.anchor.x;
        texture.flip = true; // Mark the texture for flipping
        id.direction = static_cast<uint64_t>(
            getFlippedDirection(static_cast<Direction>(id.direction))); // Flip the direction

        if (graphicsRegistry.hasTexture(id) == false)
            graphicsRegistry.registerTexture(id, texture);
    }
}
