#ifndef CORE_RENDERINGCONTEXT_H
#define CORE_RENDERINGCONTEXT_H

struct SDL_Renderer;

namespace core
{
class Coordinates;
struct FontAtlas;

struct RenderingContext
{
    SDL_Renderer* renderer;
    const Coordinates& coordinates;
    const FontAtlas& fontAtlas;
};
} // namespace core
#endif
