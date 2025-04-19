#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include "SubSystem.h"
#include "GameState.h"
#include "GameSettings.h"
#include "GraphicsRegistry.h"
#include "Renderer.h"

namespace game
{
    class ResourceLoader : public aion::SubSystem
    {
    public:
        ResourceLoader(
            const aion::GameSettings& settings, 
            aion::GraphicsRegistry& graphicsRegistry,
            aion::Renderer& renderer);
        ~ResourceLoader() = default;

        void loadTextures();
        void loadEntities();

    private:
        // SubSystem methods
        void init() override;

        void shutdown() override
        {
            // Cleanup code for resource loading
        }

        const aion::GameSettings& _settings;
        aion::GraphicsRegistry& graphicsRegistry;
        aion::Renderer& renderer;
    };
} // namespace game


#endif