#include "GaiaSystem.h"

using namespace core;

GaiaSystem::GaiaSystem()
{
    // registerCallback(Event::Type::CORPSE_REQUEST, this, &GaiaSystem::onCorpseRequest);
}

GaiaSystem::~GaiaSystem()
{
    // destructor
}

bool GaiaSystem::onCorpseRequest(const Event& e)
{
    return false;
}
