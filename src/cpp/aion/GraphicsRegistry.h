#ifndef GRAPHICSREGISTRY_H
#define GRAPHICSREGISTRY_H

#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>
#include "Types.h"
#include "Vec2d.h"

namespace aion
{
    class GraphicsID
    {
    public:
        int entitytType = 0; // 15bits: 0-32767
        int actionType = 0; // 15bits: 0-32767
        int frame = 0; // 5bits: 0-31
        utils::Direction direction = utils::Direction::NORTH; // 3bits: 0-7
        int custom1 = 0; // 10bits: 0-1023
        int custom2 = 0; // 10bits: 0-1023
        int custom3 = 0; // 4bits: 0-15

        GraphicsID() = default;

        bool operator==(const GraphicsID& other) const
        {
            return entitytType == other.entitytType 
                && actionType == other.actionType 
                && direction == other.direction
                && frame == other.frame
                && custom1 == other.custom1
                && custom2 == other.custom2
                && custom3 == other.custom3;
        }

        int64_t hash() const 
        {
            return (static_cast<int64_t>(entitytType) << 46) | 
                   (static_cast<int64_t>(actionType) << 31) | 
                   (static_cast<int64_t>(direction) << 26) | 
                   (static_cast<int64_t>(frame) << 23) | 
                   (static_cast<int64_t>(custom1) << 13) | 
                   (static_cast<int64_t>(custom2) << 3) | 
                   static_cast<int64_t>(custom3);
        }

        std::string toString() const
        {
            return "GraphicsID(" + std::to_string(entitytType) + ", " +
                   std::to_string(actionType) + ", " +
                   std::to_string(frame) + ", " +
                   std::to_string(static_cast<int>(direction)) + ", " +
                   std::to_string(custom1) + ", " +
                   std::to_string(custom2) + ", " +
                   std::to_string(custom3) + ")";
        }

        // TODO: validate this
        static GraphicsID fromHash(int64_t hash)
        {
            GraphicsID id;
            id.entitytType = (hash >> 46) & 0x7FFF; // 15 bits
            id.actionType = (hash >> 31) & 0x7FFF; // 15 bits
            id.direction = static_cast<utils::Direction>((hash >> 26) & 0x07); // 3 bits
            id.frame = (hash >> 23) & 0x1F; // 5 bits
            id.custom1 = (hash >> 13) & 0x3FF; // 10 bits
            id.custom2 = (hash >> 3) & 0x3FF; // 10 bits
            id.custom3 = hash & 0x0F; // 4 bits
            return id;
        }
    };

    struct GraphicsEntry
    {
        SDL_Texture* image = nullptr;
        Vec2d anchor{0, 0};
    };

    class GraphicsRegistry
    {
    public:
        GraphicsRegistry() = default;
        ~GraphicsRegistry() = default;

        void registerGraphic(const GraphicsID& graphicID, const GraphicsEntry& entry);
        const GraphicsEntry& getGraphic(const GraphicsID& graphicID) const;

    private:
        std::unordered_map<int64_t, GraphicsEntry> graphicsMap;
    };
}

#endif