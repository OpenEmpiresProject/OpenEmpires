#ifndef STUDIO_H
#define STUDIO_H

#include "AtlasGeneratorBasic.h"
#include "GameState.h"
#include "GameTypes.h"
#include "GraphicsLoader.h"
#include "GraphicsLoaderFromDRS.h"
#include "GraphicsRegistry.h"
#include "SDL3_gfxPrimitives.h"
#include "commands/CmdIdle.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

using namespace ion;
using namespace std;

namespace game
{
class Studio
{
  public:
    Studio(/* args */);
    ~Studio();

    void run()
    {
        auto& state = GameState::getInstance();
        bool running = true;
        auto last = SDL_GetTicks();

        while (running)
        {
            running = handleEvents();
            if (!running)
            {
                break;
            }

            SDL_SetRenderDrawColor(m_renderer, 255, 170, 255, 255);
            SDL_RenderClear(m_renderer);

            drawGrid();

            auto& animation = state.getComponent<CompAnimation>(m_villager);
            auto& action = state.getComponent<CompAction>(m_villager);

            if (m_animate)
            {
                auto now = SDL_GetTicks();
                if (now - last > 16)
                {
                    last = now;
                    animation.frame++;
                    auto& actionAnimation = animation.animations[action.action];
                    animation.frame %= actionAnimation.frames;
                }
            }

            auto [transform, entityInfo] =
                state.getComponents<CompTransform, CompEntityInfo>(m_villager);

            m_rc.direction = transform.getIsometricDirection();
            m_rc.variation = entityInfo.variation;
            m_rc.action = action.action;
            m_rc.frame = animation.frame;

            const int scale = 2;

            m_rc.updateTextureDetails(m_graphicsRegistry);
            SDL_FRect dstRect = {250 - m_rc.anchor.x * scale, 250 - m_rc.anchor.y * scale,
                                 m_rc.srcRect.w * scale, m_rc.srcRect.h * scale};

            SDL_SetTextureColorMod(m_rc.texture, m_rc.shading.r, m_rc.shading.g, m_rc.shading.b);
            SDL_RenderTextureRotated(m_renderer, m_rc.texture, &(m_rc.srcRect), &dstRect, 0,
                                     nullptr, m_rc.flip);

            circleRGBA(m_renderer, 250, 250, 10, 255, 0, 0, 255);

            SDL_RenderPresent(m_renderer);
            SDL_Delay(100);
        }

        spdlog::info("Shutting down renderer...");

        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    void init()
    {
        ion::initLogger("logs/studio.log");
        spdlog::info("Studio is starting");

        initSDL();
        AtlasGeneratorBasic atlasGenerator;
        GraphicsLoaderFromDRS drsLoader;
        GraphicsLoader& loader = drsLoader;
        loader.loadAllGraphics(m_renderer, m_graphicsRegistry, atlasGenerator);

        createVillager();
    }

    void initSDL()
    {
        spdlog::info("Initializing SDL...");

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            spdlog::error("SDL_Init failed: {}", SDL_GetError());
            throw std::runtime_error("SDL_Init failed");
        }

        m_window = SDL_CreateWindow("Studio", 500, 500, SDL_WINDOW_OPENGL);
        if (!m_window)
        {
            spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
            throw std::runtime_error("SDL_CreateWindow failed");
        }

        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer)
        {
            spdlog::error("SDL_CreateRenderer failed: {}", SDL_GetError());
            throw std::runtime_error("SDL_CreateRenderer failed");
        }
        spdlog::info("SDL initialized successfully");
    }

    bool handleEvents()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
            {
                return false;
            }
            else if (event.type == SDL_EVENT_KEY_UP)
            {
                if (event.key.scancode == SDL_SCANCODE_RIGHT)
                {
                    auto& state = GameState::getInstance();
                    auto& animation = state.getComponent<CompAnimation>(m_villager);
                    auto& action = state.getComponent<CompAction>(m_villager);

                    animation.frame++;
                    auto& actionAnimation = animation.animations[action.action];
                    animation.frame %= actionAnimation.frames;
                }
                else if (event.key.scancode == SDL_SCANCODE_LEFT)
                {
                    auto& state = GameState::getInstance();
                    auto& animation = state.getComponent<CompAnimation>(m_villager);
                    auto& action = state.getComponent<CompAction>(m_villager);

                    animation.frame--;
                    auto& actionAnimation = animation.animations[action.action];
                    if (animation.frame < 0)
                        animation.frame = actionAnimation.frames - 1;
                }
                else if (event.key.scancode == SDL_SCANCODE_UP)
                {
                    auto& state = GameState::getInstance();
                    auto& animation = state.getComponent<CompAnimation>(m_villager);
                    auto& action = state.getComponent<CompAction>(m_villager);

                    animation.frame--;
                    auto& actionAnimation = animation.animations[action.action];
                    if (animation.frame < 0)
                        animation.frame = actionAnimation.frames - 1;
                }
                else if (event.key.scancode == SDL_SCANCODE_0)
                {
                    auto& state = GameState::getInstance();
                    auto& action = state.getComponent<CompAction>(m_villager);
                    action.action = 0;
                }
                else if (event.key.scancode == SDL_SCANCODE_1)
                {
                    auto& state = GameState::getInstance();
                    auto& action = state.getComponent<CompAction>(m_villager);
                    action.action = 1;
                }
                else if (event.key.scancode == SDL_SCANCODE_2)
                {
                    auto& state = GameState::getInstance();
                    auto& action = state.getComponent<CompAction>(m_villager);
                    action.action = 2;
                }
                else if (event.key.scancode == SDL_SCANCODE_3)
                {
                    auto& state = GameState::getInstance();
                    auto& action = state.getComponent<CompAction>(m_villager);
                    action.action = 3;
                }
                else if (event.key.scancode == SDL_SCANCODE_P)
                {
                    m_animate = !m_animate;
                }
            }
        }
        return true;
    }

    void createVillager()
    {
        auto& gameState = GameState::getInstance();
        m_villager = gameState.createEntity();
        auto transform = CompTransform(100, 100);
        transform.face(Direction::SOUTH);
        transform.hasRotation = true;
        transform.speed = 256;

        CompAnimation anim;
        anim.animations[UnitAction::IDLE].frames = 15;
        anim.animations[UnitAction::IDLE].repeatable = true;
        anim.animations[UnitAction::IDLE].speed = 10;

        anim.animations[UnitAction::MOVE].frames = 15;
        anim.animations[UnitAction::MOVE].repeatable = true;
        anim.animations[UnitAction::MOVE].speed = 15;

        anim.animations[UnitAction::CHOPPING].frames = 15;
        anim.animations[UnitAction::CHOPPING].repeatable = true;
        anim.animations[UnitAction::CHOPPING].speed = 15;

        anim.animations[UnitAction::MINING].frames = 15;
        anim.animations[UnitAction::MINING].repeatable = true;
        anim.animations[UnitAction::MINING].speed = 15;

        gameState.addComponent(m_villager, transform);
        gameState.addComponent(m_villager, m_rc);
        gameState.addComponent(m_villager, CompEntityInfo(3));
        gameState.addComponent(m_villager, anim);
        gameState.addComponent(m_villager, CompAction(0));

        m_rc.entityID = m_villager;
        m_rc.entityType = EntityTypes::ET_VILLAGER;
        m_rc.layer = GraphicLayer::ENTITIES;
    }

    void drawGrid(int gap = 20)
    {
        // Set draw color to black
        SDL_SetRenderDrawColor(m_renderer, 150, 150, 150, 255);

        // Get the window size
        int w, h;
        SDL_GetWindowSize(m_window, &w, &h);

        // Draw vertical lines
        for (int x = 0; x <= w; x += gap)
        {
            SDL_RenderLine(m_renderer, x, 0, x, h);
        }

        // Draw horizontal lines
        for (int y = 0; y <= h; y += gap)
        {
            SDL_RenderLine(m_renderer, 0, y, w, y);
        }
    }

  private:
    ion::GraphicsRegistry m_graphicsRegistry;
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    CompRendering m_rc;
    uint32_t m_villager = 0;
    bool m_animate = true;
};

Studio::Studio(/* args */)
{
}

Studio::~Studio()
{
}
} // namespace game

#endif