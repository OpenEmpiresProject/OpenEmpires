#include "ZOrderStrategyWithSlicing.h"

#include "ServiceRegistry.h"
#include "Settings.h"
#include "StateManager.h"
#include "utils/ObjectPool.h"

using namespace core;

std::list<CompRendering*> slice(CompRendering& rc);

ZOrderStrategyWithSlicing::ZOrderStrategyWithSlicing()
    : m_settings(ServiceRegistry::getInstance().getService<Settings>()),
      m_zBucketsSize(
          ServiceRegistry::getInstance().getService<Settings>()->getWorldSizeInTiles().height *
          Constants::FEET_PER_TILE * 3),
      m_zBuckets(
          ServiceRegistry::getInstance().getService<Settings>()->getWorldSizeInTiles().height *
          Constants::FEET_PER_TILE * 3)
{
    m_finalListToRender.reserve(10000);
    for (size_t i = 0; i < g_graphicLayersOrder.size(); i++)
    {
        m_objectsToRenderByLayer[i].reserve(2000);
    }
}

ZOrderStrategyWithSlicing::~ZOrderStrategyWithSlicing()
{
}

void ZOrderStrategyWithSlicing::preProcess(CompRendering& cr)
{
    if (cr.isBig())
    {
        if (cr.isDestroyed)
        {
            auto existingSubComponents = m_subRenderingByEntityId.find(cr.entityID);

            if (existingSubComponents != m_subRenderingByEntityId.end())
            {
                for (auto& existing : existingSubComponents->second)
                {
                    m_subRenderingComponents.remove(existing);
                    ObjectPool<CompRendering>::release(existing);
                }
                existingSubComponents->second.clear();
                m_subRenderingByEntityId.erase(existingSubComponents);
            }
        }
        else
        {
            auto subComponents = slice(cr);
            auto existingSubComponents = m_subRenderingByEntityId.find(cr.entityID);

            if (existingSubComponents != m_subRenderingByEntityId.end())
            {
                for (auto& existing : existingSubComponents->second)
                {
                    m_subRenderingComponents.remove(existing);
                    ObjectPool<CompRendering>::release(existing);
                }
                existingSubComponents->second.clear();
                existingSubComponents->second = subComponents;
            }
            else
            {
                m_subRenderingByEntityId[cr.entityID] = subComponents;
            }
            m_subRenderingComponents.splice(m_subRenderingComponents.end(), subComponents);
        }
    }
}

const std::vector<CompRendering*>& ZOrderStrategyWithSlicing::zOrder(const Coordinates& coordinates)
{
    ++m_zBucketVersion; // it will take 4 trillion years to overflow this
    m_finalListToRender.clear();

    for (size_t i = 0; i < g_graphicLayersOrder.size(); i++)
    {
        m_objectsToRenderByLayer[i].clear();
    }

    ServiceRegistry::getInstance().getService<StateManager>()->getEntities<CompRendering>().each(
        [this, &coordinates](CompRendering& rc)
        {
            if (!rc.isBig())
            {
                addRenderingCompToZBuckets(&rc, coordinates);
            }
        });

    for (auto& sub : m_subRenderingComponents)
    {
        addRenderingCompToZBuckets(sub, coordinates);
    }

    for (int z = 0; z < m_zBucketsSize; ++z)
    {
        if (m_zBuckets[z].version != m_zBucketVersion)
        {
            continue; // Skip if the version doesn't match
        }

        for (auto& rc : m_zBuckets[z].graphicsComponents)
        {
            if (rc->layer != GraphicLayer::NONE)
            {
                m_objectsToRenderByLayer[static_cast<int>(rc->layer)].emplace_back(rc);
            }
            else
            {
                spdlog::error("Graphic {} doesn't have a layer specified to render",
                              rc->toString());
            }
        }
    }

    for (size_t i = 0; i < g_graphicLayersOrder.size(); i++)
    {
        m_finalListToRender.insert(m_finalListToRender.end(), m_objectsToRenderByLayer[i].begin(),
                                   m_objectsToRenderByLayer[i].end());
    }
    return m_finalListToRender;
}

void ZOrderStrategyWithSlicing::addRenderingCompToZBuckets(CompRendering* rc,
                                                           const Coordinates& coordinates)
{
    if (rc->isDestroyed || rc->isEnabled == false)
    {
        return;
    }

    int zOrder = 0;

    if (rc->positionInFeet.isNull() == false)
    {
        zOrder = rc->positionInFeet.y + rc->positionInFeet.x + rc->additionalZOffset;

        if (zOrder < 0 || zOrder >= m_zBucketsSize)
        {
            spdlog::error("Z-order out of bounds: {}", zOrder);
            return;
        }

        SDL_Rect viewportRect = {0, 0, m_settings->getWindowDimensions().width,
                                 m_settings->getWindowDimensions().height};

        auto screenPos = coordinates.feetToScreenUnits(rc->positionInFeet) - rc->anchor;

        SDL_Rect dstRectInt = {screenPos.x, screenPos.y, rc->srcRect.w, rc->srcRect.h};

        // Skip any texture that doesn't overlap with viewport (i.e. outside of screen)
        // This has reduced frame rendering time from 6ms to 3ms for 1366x768 window on the
        // development setup for 50x50 map in debug mode on Windows.
        if (!SDL_HasRectIntersection(&viewportRect, &dstRectInt))
        {
            return;
        }
    }
    else
    {
        zOrder = rc->positionInScreenUnits.y;
    }

    if (m_zBucketVersion != m_zBuckets[zOrder].version)
    {
        m_zBuckets[zOrder].version = m_zBucketVersion;
        m_zBuckets[zOrder].graphicsComponents.clear();
    }
    m_zBuckets[zOrder].graphicsComponents.push_back(rc);
}

std::list<CompRendering*> slice(CompRendering& rc)
{
    static Color colors[] = {Color::RED, Color::GREEN, Color::BLUE, Color::PURPLE, Color::YELLOW};
    std::list<CompRendering*> subComponentsToReturn;

    if (rc.landSize.width > 0 && rc.landSize.height > 0)
    {
        // Slice at the original image's anchor
        auto slice = ObjectPool<CompRendering>::acquire();
        *slice = rc;
        slice->srcRect.w = Constants::TILE_PIXEL_WIDTH;
        slice->srcRect.x = rc.srcRect.x + rc.anchor.x - (Constants::TILE_PIXEL_WIDTH / 2);
        slice->anchor.y = slice->srcRect.h;
        slice->anchor.x = slice->srcRect.w / 2;
        slice->additionalZOffset += Constants::FEET_PER_TILE;

        subComponentsToReturn.push_back(slice);

        // Slice left
        for (int i = 1; i < rc.landSize.width; i++)
        {
            auto slice = ObjectPool<CompRendering>::acquire();
            *slice = rc;
            slice->anchor.y = rc.srcRect.h;

            // DEBUG: Slice coloring
            // slice->shading = colors[i - 1];

            // For the left-most slice, we need to capture everything to left, but not only
            // a tile width. Because there can be small details (like shadow) just outside the
            // tile width.
            if (i == (rc.landSize.width - 1))
            {
                // Set the source texture width to capture everything to left in the left-most slice
                slice->srcRect.w = rc.anchor.x - (Constants::TILE_PIXEL_WIDTH / 2 * i);

                // Left most sclice's left edge is image-half-width left from image-center.
                // slice->anchor.x = rc.anchor.x;
            }
            else
            {
                // Distance from the center of the original texture
                slice->anchor.x = Constants::TILE_PIXEL_WIDTH / 2 * (i + 1);

                // Intermediate slices are always half of a tile width
                slice->srcRect.w = Constants::TILE_PIXEL_WIDTH / 2;

                // Move the source texture selection to right
                slice->srcRect.x = rc.srcRect.x + rc.anchor.x - slice->anchor.x;
            }

            // auto sliceZOrder = slice->positionInFeet.y + slice->positionInFeet.x -
            //                     Constants::FEET_PER_TILE * (i + 1);
            slice->additionalZOffset += -1 * Constants::FEET_PER_TILE * (i);
            subComponentsToReturn.push_back(slice);
        }

        // Slice right
        for (int i = 1; i < rc.landSize.height; i++)
        {
            auto slice = ObjectPool<CompRendering>::acquire();
            *slice = rc;
            slice->anchor.y = rc.srcRect.h;

            // DEBUG: Slice coloring
            // slice->shading = colors[i - 1];

            // For the right-most slice, we need to capture everything to right just like left
            if (i == (rc.landSize.height - 1))
            {
                // Set the source texture width to capture everything to right in the right-most
                // slice
                slice->srcRect.w =
                    (rc.srcRect.w - rc.anchor.x) - (Constants::TILE_PIXEL_WIDTH / 2 * i);

                // Right most sclice's left edge is image-half-width left from image-center.
                slice->anchor.x = -1 * (rc.srcRect.w - rc.anchor.x - slice->srcRect.w);

                // Move the source texture selection to right
                slice->srcRect.x = rc.srcRect.x + rc.anchor.x - slice->anchor.x;
            }
            else
            {
                // Move the source texture selection to right
                slice->srcRect.x =
                    rc.srcRect.x + rc.anchor.x + (Constants::TILE_PIXEL_WIDTH / 2 * i);

                // Intermediate slices are always half of a tile width
                slice->srcRect.w = Constants::TILE_PIXEL_WIDTH / 2;

                // Distance from the center of the original texture
                slice->anchor.x = -1 * ((Constants::TILE_PIXEL_WIDTH / 2) * i);
            }
            slice->additionalZOffset += -1 * Constants::FEET_PER_TILE * (i);
            subComponentsToReturn.push_back(slice);
        }
    }
    return subComponentsToReturn;
}