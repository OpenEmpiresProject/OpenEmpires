#ifndef UIMANAGER_H
#define UIMANAGER_H

#include "EventHandler.h"
#include "UI.h"
#include "utils/Types.h"

#include <vector>

namespace core
{
class UIManager : public EventHandler
{
  public:
    UIManager();
    void registerWindow(Ref<ui::Window> window);
    const std::vector<Ref<ui::Window>>& getWindows() const
    {
        return m_windows;
    }

  private:
    void onEvent(const Event& e) override;
    void onTick(const Event& e);

    std::vector<Ref<ui::Window>> m_windows;
};
} // namespace core

#endif