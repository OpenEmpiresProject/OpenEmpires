#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include "GameSettings.h"
#include "GameState.h"
#include "GraphicsRegistry.h"
#include "Renderer.h"
#include "SubSystem.h"

#include <memory>
namespace game
{
class ResourceLoader : public ion::SubSystem
{
  public:
    ResourceLoader(std::stop_token* stopToken,
                   std::shared_ptr<ion::GameSettings> settings,
                   ion::GraphicsRegistry& graphicsRegistry,
                   std::shared_ptr<ion::Renderer> renderer);
    ~ResourceLoader() = default;

  private:
    // SubSystem methods
    void init() override;
    void shutdown() override;

    void loadEntities();

    std::shared_ptr<ion::GameSettings> m_settings;
    ion::GraphicsRegistry& m_graphicsRegistry;
    std::shared_ptr<ion::Renderer> m_renderer;
};
} // namespace game

#endif