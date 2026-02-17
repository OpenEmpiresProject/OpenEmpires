#ifndef CORE_GAIASYSTEM_H
#define CORE_GAIASYSTEM_H
#include "EventHandler.h"

namespace core
{
class GaiaSystem : public EventHandler
{
public:
    GaiaSystem();
    ~GaiaSystem();

private:
    void onCorpseRequest(const Event& e);
};
} // namespace core

#endif // CORE_GAIASYSTEM_H
