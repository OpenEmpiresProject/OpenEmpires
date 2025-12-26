#ifndef CORE_GRAPHICSLOADUPDATAPROVIDER_H
#define CORE_GRAPHICSLOADUPDATAPROVIDER_H

namespace core
{
class GraphicsID;
class GraphicsLoadupDataProvider
{
public:
    struct Data{};

    virtual Data getData(const GraphicsID& id) = 0;
};
} // namespace core

#endif // CORE_GRAPHICSLOADUPDATAPROVIDER_H
