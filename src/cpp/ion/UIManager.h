#ifndef UIMANAGER_H
#define UIMANAGER_H

#include "EventHandler.h"
#include "UI.h"
#include "utils/Types.h"

#include <vector>

namespace ion
{
class UIManager : public EventHandler
{
  public:
    UIManager();
    void registerWindow(Ref<ui::Window> window);

  private:
    void onEvent(const Event& e) override;
    void onTick(const Event& e);

    std::vector<Ref<ui::Window>> m_windows;
};
} // namespace ion

#endif