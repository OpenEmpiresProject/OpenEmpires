#include "CompResourceGatherer.h"

using namespace core;

std::unordered_map<uint8_t, UnitAction> CompResourceGatherer::gatheringActions;
std::unordered_map<uint8_t, UnitAction> CompResourceGatherer::carryingActions;
