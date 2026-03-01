#ifndef GRAPHICADDON_H
#define GRAPHICADDON_H

#include "Color.h"
#include "Feet.h"
#include "GraphicsID.h"
#include "utils/Types.h"

#include <cstdint>
#include <variant>

namespace core
{
struct RenderingContext;
class CompRendering;
class GraphicsRegistry;

struct Margin
{
    int vertical = 0;
    int horizontal = 0;
};

struct GraphicAddon
{
    enum class Type : uint8_t
    {
        NONE = 0,
        ISO_CIRCLE,
        SQUARE,
        RHOMBUS,
        TEXT,
        HEALTH_BAR,
        TEXTURE
    };

    struct IsoCircle
    {
        int radius = 0;
        Vec2 center; // Relative to anchor. i.e. actual unit position

        void onRender(const RenderingContext& context,
                      const CompRendering& comp,
                      Alignment alignment,
                      const Margin& margin);
    };

    struct Square
    {
        int width = 0;
        int height = 0;
        Vec2 center;

        void onRender(const RenderingContext& context,
                      const CompRendering& comp,
                      Alignment alignment,
                      const Margin& margin);
    };

    struct Rhombus // Relative to anchor
    {
        size_t width = 0;
        size_t height = 0;

        void onRender(const RenderingContext& context,
                      const CompRendering& comp,
                      Alignment alignment,
                      const Margin& margin);
    };

    struct Text
    {
        std::string text;
        Color color;

        void onRender(const RenderingContext& context,
                      const CompRendering& comp,
                      Alignment alignment,
                      const Margin& margin);
    };

    struct HealthBar
    {
        float percentage = 1.0f; // 0.0 to 1.0

        void onRender(const RenderingContext& context,
                      const CompRendering& comp,
                      Alignment alignment,
                      const Margin& margin);
    };

    struct Texture
    {
        GraphicsID graphicsId;
        Vec2 offset; // This will override alignment if provided
        bool updated = false;

        // Following are auto populated by updateTextureDetails
        SDL_Texture* texture = nullptr;
        Vec2 anchor = {0, 0}; // Anchor position in pixels
        SDL_FlipMode flip = SDL_FLIP_NONE;
        SDL_FRect srcRect; // Source rectangle for the texture

        void onRenderAfterParent(const RenderingContext& context,
                                 const CompRendering& comp,
                                 Alignment alignment,
                                 const Margin& margin);
        void updateTextureDetails(const GraphicsRegistry& graphicsRegistry);
    };

    using Data = std::variant<std::monostate, IsoCircle, Square, Rhombus, Text, HealthBar, Texture>;

    Type type = Type::NONE;
    Data data = std::monostate{};
    Alignment alignment = Alignment::CENTER;
    Margin margin;

    // TODO - why only copy returning version?
    template <typename T> const T& getData() const
    {
        return std::get<T>(data);
    }

    template <typename T> T& getData()
    {
        return std::get<T>(data);
    }

    template <typename T> T getData() const
    {
        return std::get<T>(data);
    }
};
} // namespace core

#endif