#ifndef CORE_LOGLEVELCONTROLLER_H
#define CORE_LOGLEVELCONTROLLER_H

#include "EventHandler.h"

namespace core
{
class LogLevelController : public EventHandler
{
  public:
    LogLevelController();

  private:
    void onKeyUp(const Event& e);

  private:
    bool m_showSpamLogs = false;
};
} // namespace core

#endif // CORE_LOGLEVELCONTROLLER_H
