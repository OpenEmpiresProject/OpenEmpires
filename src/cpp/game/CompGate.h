#ifndef GAME_COMPGATE_H
#define GAME_COMPGATE_H

namespace game
{
enum class GateStatus
{
    CLOSED = 0,
    OPENED = 1
};

class CompGate
{
  public:
    bool isOpen = false;
    bool isLocked = false;
};
} // namespace game

#endif // GAME_COMPGATE_H
