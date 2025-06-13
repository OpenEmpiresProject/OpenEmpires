#include "Renderer.h"

#include "AtlasGeneratorBasic.h"
#include "Coordinates.h"
#include "Event.h"
#include "FPSCounter.h"
#include "GameSettings.h"
#include "GameState.h"
#include "Renderer.h"
#include "SDL3_gfxPrimitives.h"
#include "ServiceRegistry.h"
#include "StatsCounter.h"
#include "ThreadQueue.h"
#include "Version.h"
#include "ZOrderStrategyBase.h"
#include "ZOrderStrategyWithSlicing.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompEntityInfo.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h> // Ensure SDL_image is included
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <list>
#include <memory>
#include <readerwriterqueue.h>
#include <string>
#include <thread>
#include <vector>
#include <stdio.h>
#include <unordered_map>

#ifdef __cplusplus
extern "C" {
#endif
#include <microui.h>
#ifdef __cplusplus
}
#endif

namespace fs = std::filesystem;
using namespace ion;
using namespace std::chrono;
using namespace std;

static  char logbuf[64000];
static   int logbuf_updated = 0;
static float bg[3] = { 90, 95, 100 };

extern void mu_init(struct mu_Context*);
static auto* _force_link_mu_init = (void*)&mu_init;

static void write_log(const char *text) {
  spdlog::debug(text);
}


static void test_window(mu_Context *ctx) {
  /* do window */
  if (mu_begin_window(ctx, "Demo Window", mu_rect(40, 40, 300, 450))) {
    mu_Container *win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 240);
    win->rect.h = mu_max(win->rect.h, 300);

    /* window info */
    if (mu_header(ctx, "Window Info")) {
      mu_Container *win = mu_get_current_container(ctx);
      char buf[64];
      int widths[] = { 54, -1 };
      mu_layout_row(ctx, 2, widths, 0);
      mu_label(ctx,"Position:");
      sprintf(buf, "%d, %d", win->rect.x, win->rect.y); mu_label(ctx, buf);
      mu_label(ctx, "Size:");
      sprintf(buf, "%d, %d", win->rect.w, win->rect.h); mu_label(ctx, buf);
    }

    /* labels + buttons */
    if (mu_header_ex(ctx, "Test Buttons", MU_OPT_EXPANDED)) {
        int widths[] = { 86, -110, -1 };
      mu_layout_row(ctx, 3, widths, 0);
      mu_label(ctx, "Test buttons 1:");
      if (mu_button(ctx, "Button 1")) { write_log("Pressed button 1"); }
      if (mu_button(ctx, "Button 2")) { write_log("Pressed button 2"); }
      mu_label(ctx, "Test buttons 2:");
      if (mu_button(ctx, "Button 3")) { write_log("Pressed button 3"); }
      if (mu_button(ctx, "Popup")) { mu_open_popup(ctx, "Test Popup"); }
      if (mu_begin_popup(ctx, "Test Popup")) {
        mu_button(ctx, "Hello");
        mu_button(ctx, "World");
        mu_end_popup(ctx);
      }
    }

    /* tree */
    if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
        int widths[] = { 140, -1 };
      mu_layout_row(ctx, 2, widths, 0);
      mu_layout_begin_column(ctx);
      if (mu_begin_treenode(ctx, "Test 1")) {
        if (mu_begin_treenode(ctx, "Test 1a")) {
          mu_label(ctx, "Hello");
          mu_label(ctx, "world");
          mu_end_treenode(ctx);
        }
        if (mu_begin_treenode(ctx, "Test 1b")) {
          if (mu_button(ctx, "Button 1")) { write_log("Pressed button 1"); }
          if (mu_button(ctx, "Button 2")) { write_log("Pressed button 2"); }
          mu_end_treenode(ctx);
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 2")) {
        int widths[] = { 54, 54};
        mu_layout_row(ctx, 2, widths, 0);
        if (mu_button(ctx, "Button 3")) { write_log("Pressed button 3"); }
        if (mu_button(ctx, "Button 4")) { write_log("Pressed button 4"); }
        if (mu_button(ctx, "Button 5")) { write_log("Pressed button 5"); }
        if (mu_button(ctx, "Button 6")) { write_log("Pressed button 6"); }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 3")) {
        static int checks[3] = { 1, 0, 1 };
        mu_checkbox(ctx, "Checkbox 1", &checks[0]);
        mu_checkbox(ctx, "Checkbox 2", &checks[1]);
        mu_checkbox(ctx, "Checkbox 3", &checks[2]);
        mu_end_treenode(ctx);
      }
      mu_layout_end_column(ctx);

      mu_layout_begin_column(ctx);
      int widths2[] = {-1};
      mu_layout_row(ctx, 1, widths2, 0);
      mu_text(ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing "
        "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
        "ipsum, eu varius magna felis a nulla.");
      mu_layout_end_column(ctx);
    }

    /* background color sliders */
    if (mu_header_ex(ctx, "Background Color", MU_OPT_EXPANDED)) {
      int widths[] = { -78, -1 };

      mu_layout_row(ctx, 2, widths, 74);
      /* sliders */
      mu_layout_begin_column(ctx);
      int widths2[] = { 46, -1 };

      mu_layout_row(ctx, 2, widths2, 0);
      mu_label(ctx, "Red:");   mu_slider(ctx, &bg[0], 0, 255);
      mu_label(ctx, "Green:"); mu_slider(ctx, &bg[1], 0, 255);
      mu_label(ctx, "Blue:");  mu_slider(ctx, &bg[2], 0, 255);
      mu_layout_end_column(ctx);
      /* color preview */
      mu_Rect r = mu_layout_next(ctx);
      mu_draw_rect(ctx, r, mu_color(bg[0], bg[1], bg[2], 255));
      char buf[32];
      sprintf(buf, "#%02X%02X%02X", (int) bg[0], (int) bg[1], (int) bg[2]);
      mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
    }

    mu_end_window(ctx);
  }
}



static int uint8_slider(mu_Context *ctx, unsigned char *value, int low, int high) {
  static float tmp;
  mu_push_id(ctx, &value, sizeof(value));
  tmp = *value;
  int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
  *value = tmp;
  mu_pop_id(ctx);
  return res;
}


static void style_window(mu_Context *ctx) {
  static struct { const char *label; int idx; } colors[] = {
    { "text:",         MU_COLOR_TEXT        },
    { "border:",       MU_COLOR_BORDER      },
    { "windowbg:",     MU_COLOR_WINDOWBG    },
    { "titlebg:",      MU_COLOR_TITLEBG     },
    { "titletext:",    MU_COLOR_TITLETEXT   },
    { "panelbg:",      MU_COLOR_PANELBG     },
    { "button:",       MU_COLOR_BUTTON      },
    { "buttonhover:",  MU_COLOR_BUTTONHOVER },
    { "buttonfocus:",  MU_COLOR_BUTTONFOCUS },
    { "base:",         MU_COLOR_BASE        },
    { "basehover:",    MU_COLOR_BASEHOVER   },
    { "basefocus:",    MU_COLOR_BASEFOCUS   },
    { "scrollbase:",   MU_COLOR_SCROLLBASE  },
    { "scrollthumb:",  MU_COLOR_SCROLLTHUMB },
    { NULL }
  };

  if (mu_begin_window(ctx, "Style Editor", mu_rect(350, 250, 300, 240))) {
    int sw = mu_get_current_container(ctx)->body.w * 0.14;
    int widths[] = { 80, sw, sw, sw, sw, -1 };
    mu_layout_row(ctx, 6, widths, 0);
    for (int i = 0; colors[i].label; i++) {
      mu_label(ctx, colors[i].label);
      uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
      mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[i]);
    }
    mu_end_window(ctx);
  }
}


static void process_frame(mu_Context *ctx) {
  mu_begin(ctx);
  style_window(ctx);
  test_window(ctx);
  mu_end(ctx);
}

char getMUButon(int sdlButton)
{
    if (sdlButton == SDL_BUTTON_LEFT)
    {
        return MU_MOUSE_LEFT;
    }
    else if (sdlButton == SDL_BUTTON_RIGHT)
    {
        return MU_MOUSE_RIGHT;
    }
    else if (sdlButton == SDL_BUTTON_MIDDLE)
    {
        return MU_MOUSE_MIDDLE;
    }
    return 0;
}

char getMUKey(int sdlKey)
{
    if (sdlKey == SDLK_LSHIFT)
    {
        return MU_KEY_SHIFT;
    }
    else if (sdlKey == SDLK_RSHIFT)
    {
        return MU_KEY_SHIFT;
    }
    else if (sdlKey == SDLK_LCTRL)
    {
        return MU_KEY_CTRL;
    }
    else if (sdlKey == SDLK_RCTRL)
    {
        return MU_KEY_CTRL;
    }
    else if (sdlKey == SDLK_LALT)
    {
        return MU_KEY_ALT;
    }
    else if (sdlKey == SDLK_RALT)
    {
        return MU_KEY_ALT;
    }
    else if (sdlKey == SDLK_RETURN)
    {
        return MU_KEY_RETURN;
    }
    else if (sdlKey == SDLK_BACKSPACE)
    {
        return MU_KEY_BACKSPACE;
    }
    return 0;
}

struct FontAtlas {
    SDL_Texture* texture = nullptr; // final atlas texture
    std::unordered_map<char, SDL_Rect> glyphRects; // source rects per glyph
    int atlasWidth = 0;
    int atlasHeight = 0;
};

FontAtlas createFontAtlas(SDL_Renderer* renderer, TTF_Font* font, int padding = 2) {
    const int startChar = 32;
    const int endChar = 126;

    std::unordered_map<char, SDL_Surface*> glyphSurfaces;
    SDL_Surface* someSurface = nullptr;
    int maxGlyphHeight = 0, totalWidth = 0;

    // Step 1: Render each character to individual surfaces
    for (char c = startChar; c <= endChar; ++c) {
        // if (!TTF_GlyphIsProvided(font, c)) continue;

        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* glyphSurface = TTF_RenderGlyph_Blended(font, c, white);
        if (!glyphSurface) continue;

        someSurface = glyphSurface;
        glyphSurfaces[c] = glyphSurface;
        maxGlyphHeight = std::max(maxGlyphHeight, glyphSurface->h);
        totalWidth += glyphSurface->w + padding;
    }

    // Step 2: Create a surface to hold the full atlas
    SDL_Surface* atlasSurface = SDL_CreateSurface(totalWidth, maxGlyphHeight, someSurface->format);
    if (!atlasSurface)
    {
        spdlog::error("Failed to create atlas surface. {}", SDL_GetError());
        return FontAtlas();
    }

    // Set the palette to the atlas surface
    // if (!SDL_SetSurfacePalette(atlasSurface, SDL_GetSurfacePalette(someSurface)))
    // {
    //     spdlog::warn("Failed to set palette for atlas surface: {}", SDL_GetError());
    //     return;
    // }
    
    // SDL_Surface* atlasSurface = SDL_CreateRGBSurfaceWithFormat(0, totalWidth, maxGlyphHeight, 32, SDL_PIXELFORMAT_RGBA32);
    if (!atlasSurface) {
        // Cleanup before early return
        for (auto& [c, s] : glyphSurfaces) SDL_DestroySurface(s);
        return {};
    }

    // Step 3: Blit each glyph onto the atlas surface
    std::unordered_map<char, SDL_Rect> srcRects;
    int xOffset = 0;
    for (char c = startChar; c <= endChar; ++c) {
        auto it = glyphSurfaces.find(c);
        if (it == glyphSurfaces.end()) continue;

        SDL_Surface* glyphSurface = it->second;

        SDL_Rect dst = {xOffset, 0, glyphSurface->w, glyphSurface->h};
        SDL_BlitSurface(glyphSurface, nullptr, atlasSurface, &dst);
        srcRects[c] = dst;

        xOffset += glyphSurface->w + padding;
        SDL_DestroySurface(glyphSurface);
    }

    // Step 4: Convert atlas surface to texture
    SDL_Texture* atlasTexture = SDL_CreateTextureFromSurface(renderer, atlasSurface);
    FontAtlas result = {
        .texture = atlasTexture,
        .glyphRects = std::move(srcRects),
        .atlasWidth = atlasSurface->w,
        .atlasHeight = atlasSurface->h
    };

    SDL_DestroySurface(atlasSurface);
    return result;
}


// static const char button_map[256] = {
//   [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
//   [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
//   [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
// };

// static const char key_map[256] = {
//   [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
//   [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
//   [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
//   [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
//   [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
//   [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
//   [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
//   [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
// };


static int text_width(mu_Font font, const char *text, int len) {
//   if (len == -1) { len = strlen(text); }
//   return r_get_text_width(text, len);
    return 100;
}

static int text_height(mu_Font font) {
//   return r_get_text_height();
    return 20;
}


namespace ion
{

class RendererImpl
{
  public:
    RendererImpl(std::stop_source* stopSource,
                 GraphicsRegistry& graphicsRegistry,
                 ThreadSynchronizer<FrameData>& synchronizer,
                 GraphicsLoader& graphicsLoader);
    ~RendererImpl();

    SDL_Renderer* getSDLRenderer();
    void init();
    void shutdown();
    void initSDL();
    void threadEntry();
    void r_set_clip_rect(mu_Rect rect);
    void renderingLoop();
    void cleanup();
    bool handleEvents();
    void updateRenderingComponents();
    void renderDebugInfo(FPSCounter& counter);
    void renderGameEntities();
    void renderBackground();
    void renderSelectionBox();
    void addDebugText(const std::string& text);
    void clearDebugTexts();
    void generateTicks();
    void onTick();
    void handleViewportMovement();
    Vec2d getDebugOverlayPosition(DebugOverlay::FixedPosition anchor, const SDL_FRect& rect);
    void renderCirlceInIsometric(SDL_Renderer* renderer,
                                 Sint16 cx,
                                 Sint16 cy,
                                 Sint16 r,
                                 Uint8 red,
                                 Uint8 green,
                                 Uint8 blue,
                                 Uint8 alpha);
    void loadFonts();

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_texture = nullptr;

    std::thread m_renderThread;
    std::atomic<bool> m_running = false;
    std::shared_ptr<GameSettings> m_settings;
    GraphicsRegistry& m_graphicsRegistry;

    std::condition_variable m_sdlInitCV;
    std::mutex m_sdlInitMutex;
    bool m_sdlInitialized = false;

    std::vector<std::string> m_debugTexts;
    Coordinates m_coordinates;

    std::chrono::steady_clock::time_point m_lastTickTime;
    Vec2d m_lastMouseClickPosInFeet;
    Vec2d m_lastMouseClickPosInTiles;

    int64_t m_tickCount = 0;

    bool m_showDebugInfo = false;

    ThreadSynchronizer<FrameData>& m_synchronizer;

    Vec2d m_selectionStartPosScreenUnits;
    Vec2d m_selectionEndPosScreenUnits;
    bool m_isSelecting = false;

    std::list<CompRendering*> m_subRenderingComponents;
    std::unordered_map<uint32_t, std::list<CompRendering*>> m_subRenderingByEntityId;

    StatsCounter<uint64_t> m_frameTime;
    StatsCounter<uint64_t> m_waitTime;

    size_t m_texturesDrew = 0;

    GraphicsLoader& m_graphicsLoader;

    std::unique_ptr<ZOrderStrategyBase> m_zOrderStrategy;

    std::stop_source* m_stopSource = nullptr;

    mu_Context *ctx = nullptr;
    FontAtlas m_fontAtlas;
};
} // namespace ion

RendererImpl::RendererImpl(std::stop_source* stopSource,
                GraphicsRegistry& graphicsRegistry,
                ThreadSynchronizer<FrameData>& synchronizer,
                GraphicsLoader& graphicsLoader)
    : m_stopSource(stopSource),
        m_settings(ServiceRegistry::getInstance().getService<GameSettings>()),
        m_graphicsRegistry(graphicsRegistry),
        m_coordinates(ServiceRegistry::getInstance().getService<GameSettings>()),
        m_synchronizer(synchronizer), m_graphicsLoader(graphicsLoader),
        m_zOrderStrategy(std::move(std::make_unique<ZOrderStrategyWithSlicing>()))
{
    m_running = false;
    m_lastTickTime = steady_clock::now();
    spdlog::info("Max z-order: {}", m_coordinates.getMaxZOrder());

    auto center = m_coordinates.getMapCenterInFeet();
    auto centerPixels = m_coordinates.feetToPixels(center);
    centerPixels -= Vec2d(m_settings->getWindowDimensions().width / 2,
                            m_settings->getWindowDimensions().height / 2);
    m_coordinates.setViewportPositionInPixels(centerPixels);
}

RendererImpl::~RendererImpl()
{
    cleanup();
    // IMG_Quit();
    SDL_Quit();
}

SDL_Renderer* RendererImpl::getSDLRenderer()
{
    std::unique_lock<std::mutex> lock(m_sdlInitMutex);
    m_sdlInitCV.wait(lock, [this] { return m_sdlInitialized; });
    return m_renderer;
}

void RendererImpl::init()
{
    if (!m_running)
    {
        m_running = true;
        m_renderThread = std::thread(&RendererImpl::threadEntry, this);
    }
}

void RendererImpl::shutdown()
{
    m_running = false;
    if (m_renderThread.joinable())
    {
        m_renderThread.join();
    }
}

void RendererImpl::initSDL()
{
    spdlog::info("Initializing SDL...");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        spdlog::error("SDL_Init failed: {}", SDL_GetError());
        throw std::runtime_error("SDL_Init failed");
    }

    m_window = SDL_CreateWindow(m_settings->getTitle().c_str(),
                                m_settings->getWindowDimensions().width,
                                m_settings->getWindowDimensions().height, SDL_WINDOW_OPENGL);
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
    {
        std::lock_guard<std::mutex> lock(m_sdlInitMutex);
        m_sdlInitialized = true;
    }

    loadFonts();

    m_sdlInitCV.notify_all();
    spdlog::info("SDL initialized successfully");

    /* init microui */
    ctx = (mu_Context*)malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;
}

void RendererImpl::threadEntry()
{
    initSDL();
    AtlasGeneratorBasic atlasGenerator;
    m_graphicsLoader.loadAllGraphics(m_renderer, m_graphicsRegistry, atlasGenerator);
    renderingLoop();
}

void RendererImpl::r_set_clip_rect(mu_Rect rect) {
    SDL_Rect sdl_clip_rect;
    sdl_clip_rect.x = rect.x;
    sdl_clip_rect.w = rect.w;
    sdl_clip_rect.h = rect.h;
    SDL_SetRenderClipRect(m_renderer, &sdl_clip_rect);
}

void RendererImpl::renderingLoop()
{
    spdlog::info("Starting rendering loop...");
    bool running = true;
    FPSCounter fpsCounter;

    while (running)
    {
        auto frame = m_synchronizer.getReceiverFrameData().frameNumber;
        // spdlog::info("Rendering frame {}", frame);

        auto start = SDL_GetTicks();
        fpsCounter.frame();
        running = handleEvents();
        m_texturesDrew = 0;

        if (!running)
        {
            break;
        }
        generateTicks();
        process_frame(ctx);

        updateRenderingComponents();
        renderBackground();
        renderGameEntities();
        renderSelectionBox();
        renderDebugInfo(fpsCounter);

        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_TEXT: 
            {
                SDL_RenderDebugText(m_renderer, cmd->text.pos.x, cmd->text.pos.y, cmd->text.str);
            }
            break;
            case MU_COMMAND_RECT: 
            {
                SDL_SetRenderDrawColor(m_renderer, cmd->rect.color.r, cmd->rect.color.g, cmd->rect.color.b, cmd->rect.color.a);
                SDL_FRect sdl_rect_f = { (float)cmd->rect.rect.x, (float)cmd->rect.rect.y, (float)cmd->rect.rect.w, (float)cmd->rect.rect.h };
                SDL_RenderFillRect(m_renderer, &sdl_rect_f);
            }
            break;
            case MU_COMMAND_ICON: 
            {
                // r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); 
            }
            break;
            case MU_COMMAND_CLIP: 
            {
                // r_set_clip_rect(cmd->clip.rect); 
            }
            break;
        }
        }

        char arr[] = "Maleesh";
        int x = 100;
        for (char c : arr)
        {
            SDL_Rect src = m_fontAtlas.glyphRects[c];

            SDL_FRect srcRect = {src.x, src.y, src.w, src.h};
            SDL_FRect dstRect = {x, 100, src.w, src.h};
            x += src.w + 2;

            SDL_RenderTextureRotated(m_renderer, m_fontAtlas.texture, &srcRect, &dstRect, 0, nullptr,
                                        SDL_FLIP_NONE);

        }

        SDL_RenderPresent(m_renderer);

        m_frameTime.addSample(SDL_GetTicks() - start);

        auto waitStart = SDL_GetTicks();
        m_synchronizer.waitForSender();
        m_waitTime.addSample(SDL_GetTicks() - waitStart);

        int delay = (1000 / m_settings->getTargetFPS()) - (SDL_GetTicks() - start);
        delay = std::max(1, delay);

        SDL_Delay(delay);

        fpsCounter.sleptFor(delay);
        fpsCounter.getTotalSleep();

        m_waitTime.resetIfCountIs(1000);
        m_frameTime.resetIfCountIs(1000);
    }

    spdlog::info("Shutting down renderer...");

    SDL_DestroyWindow(m_window);
    SDL_Quit();
    m_synchronizer.shutdown();
    m_stopSource->request_stop();
}

void RendererImpl::cleanup()
{
    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

bool RendererImpl::handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_EVENT_QUIT: exit(EXIT_SUCCESS); break;
            case SDL_EVENT_MOUSE_MOTION: mu_input_mousemove(ctx, event.motion.x, event.motion.y); break;
            // case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, event.wheel.y * -30); break;
            // case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
            int b = getMUButon(event.button.button);
            if (b && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) { mu_input_mousedown(ctx, event.button.x, event.button.y, b); }
            if (b && event.type ==   SDL_EVENT_MOUSE_BUTTON_UP) { mu_input_mouseup(ctx, event.button.x, event.button.y, b);   }
            break;
            }

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
            int c = getMUKey(event.key.key);
            if (c && event.type == SDL_EVENT_KEY_DOWN) { mu_input_keydown(ctx, c); }
            if (c && event.type ==   SDL_EVENT_KEY_UP) { mu_input_keyup(ctx, c);   }
            break;
            }
        }
        
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            return false;
        }
        // handle mouse click events
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                Vec2d mousePosScreenUnits(event.button.x, event.button.y);
                m_lastMouseClickPosInFeet =
                    m_coordinates.screenUnitsToFeet(mousePosScreenUnits);
                m_lastMouseClickPosInTiles =
                    m_coordinates.feetToTiles(m_lastMouseClickPosInFeet);

                Event clickEvent(
                    Event::Type::MOUSE_BTN_UP,
                    MouseClickData{MouseClickData::Button::LEFT, m_lastMouseClickPosInFeet});

                m_isSelecting = true;
                m_selectionStartPosScreenUnits = mousePosScreenUnits;
                m_selectionEndPosScreenUnits = mousePosScreenUnits;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            m_isSelecting = false;
        }
        else if (event.type == SDL_EVENT_KEY_UP)
        {
            if (event.key.scancode == SDL_SCANCODE_1)
            {
            }
            else if (event.key.scancode == SDL_SCANCODE_2)
            {
                m_showDebugInfo = !m_showDebugInfo;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (m_isSelecting)
            {
                Vec2d mousePosScreenUnits(event.button.x, event.button.y);
                m_selectionEndPosScreenUnits = mousePosScreenUnits;
            }
        }
    }
    return true;
}

/**
 * @brief Updates the graphic components by processing messages from the simulator.
 *
 * This function handles graphic instructions by dequeuing messages from the simulator queue
 * and processing their command buffers. It updates the graphics components of entities
 * based on the instructions provided in the messages. If a texture is not found for an entity,
 * an error is logged. Invalid message types or other unexpected conditions will throw
 * exceptions.
 *
 * @throws std::runtime_error If a message with an invalid type is encountered.
 */
void RendererImpl::updateRenderingComponents()
{
    // spdlog::debug("Handling graphic instructions...");
    auto& frameData = m_synchronizer.getReceiverFrameData().graphicUpdates;

    for (const auto& instruction : frameData)
    {
        auto& rc = GameState::getInstance().getComponent<CompRendering>(instruction->entityID);
        static_cast<CompGraphics&>(rc) = *instruction;
        rc.additionalZOffset = 0;

        // Graphic addons on this entity might draw below (screen's Y) the entity. So we need to
        // consider the overall bottom most position as the Z value. Eg: Unit's selection
        // ellipse should draw on top of the below tile
        for (auto& addon : rc.addons)
        {
            switch (addon.type)
            {
            case GraphicAddon::Type::CIRCLE:
                rc.additionalZOffset = Constants::FEET_PER_TILE + 1;
                break;
            }
        }

        rc.updateTextureDetails(m_graphicsRegistry);

        m_zOrderStrategy->preProcess(rc);

        ObjectPool<CompGraphics>::release(instruction);
    }
    frameData.clear();
}

void RendererImpl::renderDebugInfo(FPSCounter& counter)
{
    // spdlog::debug("Rendering debug info...");

    addDebugText("Average FPS        : " + std::to_string(counter.getAverageFPS()));
    addDebugText("Avg Sleep/frame    : " + std::to_string(counter.getAverageSleepMs()));
    addDebugText("Avg frame time     : " + std::to_string(m_frameTime.average()));
    addDebugText("Avg wait time      : " + std::to_string(m_waitTime.average()));
    addDebugText("Textures Drew      : " + std::to_string(m_texturesDrew));
    addDebugText("Viewport           : " +
                    m_coordinates.getViewportPositionInPixels().toString());
    addDebugText("Mouse clicked feet : " + m_lastMouseClickPosInFeet.toString());
    addDebugText("Mouse clicked tile : " + m_lastMouseClickPosInTiles.toString());
    addDebugText("Graphics loaded    : " +
                    std::to_string(m_graphicsRegistry.getTextureCount()));

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); /* white, full alpha */

    int y = 10;
    for (const auto& line : m_debugTexts)
    {
        SDL_RenderDebugText(m_renderer, 10, y, line.c_str());
        y += 20;
    }

    SDL_RenderDebugText(m_renderer, 10, m_settings->getWindowDimensions().height - 20,
                        OPENEMPIRES_VERSION_STRING);

    clearDebugTexts();
}

void RendererImpl::renderGameEntities()
{
    SDL_FRect dstRect = {0, 0, 0, 0};

    auto& objectsToRender = m_zOrderStrategy->zOrder(m_coordinates);

    for (auto& rc : objectsToRender)
    {
        auto screenPos = m_coordinates.feetToScreenUnits(rc->positionInFeet) - rc->anchor;

        dstRect.x = screenPos.x;
        dstRect.y = screenPos.y;
        dstRect.w = rc->srcRect.w;
        dstRect.h = rc->srcRect.h;

        for (auto& addon : rc->addons)
        {
            switch (addon.type)
            {
            case GraphicAddon::Type::CIRCLE:
            {
                const auto& circle = addon.getData<GraphicAddon::Circle>();
                auto circleScreenPos = screenPos + rc->anchor + circle.center;

                // TODO: Support colors if required
                renderCirlceInIsometric(m_renderer, circleScreenPos.x, circleScreenPos.y,
                                        circle.radius, 255, 255, 255, 255);
            }
            break;
            case GraphicAddon::Type::RHOMBUS:
            {
                const auto& rhombus = addon.getData<GraphicAddon::Rhombus>();
                auto center = screenPos + rc->anchor;

                lineRGBA(m_renderer, center.x - rhombus.width / 2, center.y, center.x,
                            center.y - rhombus.height / 2, 255, 255, 255, 255);
                lineRGBA(m_renderer, center.x, center.y - rhombus.height / 2,
                            center.x + rhombus.width / 2, center.y, 255, 255, 255, 255);
                lineRGBA(m_renderer, center.x + rhombus.width / 2, center.y, center.x,
                            center.y + rhombus.height / 2, 255, 255, 255, 255);
                lineRGBA(m_renderer, center.x, center.y + rhombus.height / 2,
                            center.x - rhombus.width / 2, center.y, 255, 255, 255, 255);
            }
            break;
            default:
                break;
            }
        }
        SDL_SetTextureColorMod(rc->texture, rc->shading.r, rc->shading.g, rc->shading.b);
        SDL_RenderTextureRotated(m_renderer, rc->texture, &(rc->srcRect), &dstRect, 0, nullptr,
                                    rc->flip);
        ++m_texturesDrew;

        if (m_showDebugInfo)
        {
            for (auto& overlay : rc->debugOverlays)
            {
                auto pos = getDebugOverlayPosition(overlay.anchor, dstRect);

                switch (overlay.type)
                {
                case DebugOverlay::Type::CIRCLE:
                    ellipseRGBA(m_renderer, pos.x, pos.y, 30, 15, 255, 0, 0,
                                255); // green circle
                    break;
                case DebugOverlay::Type::FILLED_CIRCLE:
                    filledEllipseRGBA(m_renderer, pos.x, pos.y, 20, 10, 0, 0, 255,
                                        100); // blue ellipse
                case DebugOverlay::Type::RHOMBUS:
                {
                    auto end1 = getDebugOverlayPosition(overlay.customPos1, dstRect);
                    auto end2 = getDebugOverlayPosition(overlay.customPos2, dstRect);

                    // Lifting the lines by a single pixel to avoid the next tile overriding
                    // these
                    lineRGBA(m_renderer, pos.x, pos.y - 1, end1.x, end1.y - 1, 180, 180, 180,
                                255);
                    lineRGBA(m_renderer, pos.x, pos.y - 1, end2.x, end2.y - 1, 180, 180, 180,
                                255);
                }
                break;
                }
            }
        }
    }

    // Show a small cross at center of the screen.
    if (m_showDebugInfo)
    {
        auto windowSize = m_settings->getWindowDimensions();
        Vec2d center(windowSize.width / 2, windowSize.height / 2);
        auto horiLineStart = center - Vec2d(5, 0);
        auto vertLineStart = center - Vec2d(0, 5);

        lineRGBA(m_renderer, horiLineStart.x, horiLineStart.y, horiLineStart.x + 10,
                    horiLineStart.y, 255, 255, 255, 255);
        lineRGBA(m_renderer, vertLineStart.x, vertLineStart.y, vertLineStart.x,
                    vertLineStart.y + 10, 255, 255, 255, 255);
    }
}

void RendererImpl::renderBackground()
{
    SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
    SDL_RenderClear(m_renderer);
}

void RendererImpl::renderSelectionBox()
{
    if (m_isSelecting && m_selectionStartPosScreenUnits != m_selectionEndPosScreenUnits)
    {
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_FRect rect = {
            (float) m_selectionStartPosScreenUnits.x, (float) m_selectionStartPosScreenUnits.y,
            (float) (m_selectionEndPosScreenUnits.x - m_selectionStartPosScreenUnits.x),
            (float) (m_selectionEndPosScreenUnits.y - m_selectionStartPosScreenUnits.y)};

        SDL_RenderRect(m_renderer, &rect);
    }
}

void RendererImpl::addDebugText(const std::string& text)
{
    m_debugTexts.push_back(text);
}

void RendererImpl::clearDebugTexts()
{
    m_debugTexts.clear();
}

void RendererImpl::generateTicks()
{
    const auto tickDelay = milliseconds(1000 / m_settings->getTicksPerSecond());
    auto now = steady_clock::now();
    if (now - m_lastTickTime >= tickDelay)
    {
        m_lastTickTime = now;
        m_tickCount++;
        onTick();
    }
}

void RendererImpl::onTick()
{
    handleViewportMovement();
    m_synchronizer.getReceiverFrameData().viewportPositionInPixels =
        m_coordinates.getViewportPositionInPixels();
}
void RendererImpl::handleViewportMovement()
{
    auto keyStates = SDL_GetKeyboardState(nullptr);
    if (keyStates[SDL_SCANCODE_W])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() -
            Vec2d(0, m_settings->getViewportMovingSpeed()));
    }
    if (keyStates[SDL_SCANCODE_A])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() -
            Vec2d(m_settings->getViewportMovingSpeed(), 0));
    }
    if (keyStates[SDL_SCANCODE_S])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() +
            Vec2d(0, m_settings->getViewportMovingSpeed()));
    }
    if (keyStates[SDL_SCANCODE_D])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() +
            Vec2d(m_settings->getViewportMovingSpeed(), 0));
    }
}

Vec2d RendererImpl::getDebugOverlayPosition(DebugOverlay::FixedPosition anchor, const SDL_FRect& rect)
{
    int x = rect.x;
    int y = rect.y;
    int w = rect.w;
    int h = rect.h;

    switch (anchor)
    {
    case DebugOverlay::FixedPosition::TOP_LEFT:
        return {x, y};
        break;
    case DebugOverlay::FixedPosition::TOP_CENTER:
        return {x + w / 2, y};
        break;
    case DebugOverlay::FixedPosition::TOP_RIGHT:
        return {x + w, y};
        break;
    case DebugOverlay::FixedPosition::CENTER_LEFT:
        return {x, y + h / 2};
        break;
    case DebugOverlay::FixedPosition::CENTER:
        return {x + w / 2, y + h / 2};
        break;
    case DebugOverlay::FixedPosition::CENTER_RIGHT:
        return {x + w, y + h / 2};
        break;
    case DebugOverlay::FixedPosition::BOTTOM_LEFT:
        return {x, y + h};
        break;
    case DebugOverlay::FixedPosition::BOTTOM_CENTER:
        return {x + w / 2, y + h};
        break;
    case DebugOverlay::FixedPosition::BOTTOM_RIGHT:
        return {x + w, y + h};
        break;
    }
    return {x, y};
}

void RendererImpl::renderCirlceInIsometric(SDL_Renderer* renderer,
                                Sint16 cx,
                                Sint16 cy,
                                Sint16 r,
                                Uint8 red,
                                Uint8 green,
                                Uint8 blue,
                                Uint8 alpha)
{
    // Isometric ellipse radius
    Sint16 rx = r * 2; // Horizontal radius (diameter of the circle)
    Sint16 ry = r;     // Vertical radius (half of the original)

    // Render the isometric ellipse
    ellipseRGBA(renderer, cx, cy, rx, ry, red, green, blue, alpha);
}

void RendererImpl::loadFonts()
{
    if (TTF_Init() == -1) 
    {
        spdlog::error("Failed to initialize TTF: {}", SDL_GetError());
        throw std::runtime_error("TTF_Init failed");
    }

    TTF_Font* font = TTF_OpenFont("assets/fonts/lucida-bright-regular.ttf", 12); // 24 pt font
    if (!font) 
    {
        spdlog::error("Failed to open the font: {}", SDL_GetError());
        throw std::runtime_error("TTF_OpenFont failed");
    }

    m_fontAtlas = createFontAtlas(m_renderer, font);
}

Renderer::Renderer(std::stop_source* stopSource,
                   GraphicsRegistry& graphicsRegistry,
                   ThreadSynchronizer<FrameData>& synchronizer,
                   GraphicsLoader& graphicsLoader)
    : SubSystem(stopSource)
{
    m_impl = new RendererImpl(stopSource, graphicsRegistry, synchronizer, graphicsLoader);
}

Renderer::~Renderer()
{
    delete m_impl;
}

void Renderer::init()
{
    m_impl->init();
}

void Renderer::shutdown()
{
    m_impl->shutdown();
}

SDL_Renderer* Renderer::getSDLRenderer()
{
    return m_impl->getSDLRenderer();
}