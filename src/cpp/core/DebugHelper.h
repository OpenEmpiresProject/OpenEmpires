#ifndef GAME_DEBUGHELPER_H
#define GAME_DEBUGHELPER_H
#include "EventHandler.h"
#include "StateManager.h"

namespace core
{
class DebugHelper : public core::EventHandler
{
  public:
    DebugHelper();
    ~DebugHelper();

    void onInit(EventLoop& eventLoop) override
    {
    }

    void onInit(Ref<EventLoop> eventLoop)
    {
        m_eventloop = eventLoop;
    }

    bool isGamePaused() const
    {
        return m_gamePaused;
    }

    void pauseGame(bool val)
    {
        m_eventloop->setPaused(val);
        m_gamePaused = val;
    }

    bool isShowingGizmos() const
    {
        return m_showGizmos;
    }

    void showGizmos(bool val)
    {
        m_showGizmos = val;
    }

    bool isShowGizmosOnlyWhenSelected() const
    {
        return m_showGizmosOnlyWhenSelected;
    }

    void showGizmosOnlyWhenSelected(bool val)
    {
        m_showGizmosOnlyWhenSelected = val;
    }

    bool isGizmoTypeHidden(const std::string& type) const
    {
        return m_hiddenGizmoTypes.contains(type);
    }

    void hideGizmoType(const std::string& type, bool hide)
    {
        if (hide)
        {
            m_hiddenGizmoTypes.insert(type);
        }
        else
        {
            m_hiddenGizmoTypes.erase(type);
        }
    }

    std::string getGizmoFilterStr() const
    {
        std::string result;
        for (auto& n : m_hiddenGizmoTypes)
        {
            result += "_" + n;
        }
        result += "_";

        return result;
    }

    bool isShowingFrameStats() const
    {
        return m_showFrameStats;
    }

    void showFrameStats(bool val)
    {
        m_showFrameStats = val;
    }

    bool isHidingFogOfWar() const
    {
        return m_hideFogOfWar;
    }

    void hideFogOfWar(bool val)
    {
        m_hideFogOfWar = val;
    }

    void forwardFrames(int frameCount)
    {
        if (frameCount > 0)
            m_eventloop->setFramesRemainingToPlay(frameCount);
    }

  private:
    bool onTick(const core::Event& e);
    bool onKeyUp(const core::Event& e);

    core::LazyServiceRef<core::StateManager> m_stateMan;
    Ref<EventLoop> m_eventloop;
    bool m_showDebugDetails = false;
    Flat2DArray<std::string> m_densityGridCellGizmoNames;

    bool m_gamePaused = false;
    bool m_showGizmos = false;
    bool m_showGizmosOnlyWhenSelected = false;
    bool m_showFrameStats = false;
    bool m_hideFogOfWar = false;
    std::set<std::string> m_hiddenGizmoTypes;
};
} // namespace core

#endif // GAME_DEBUGHELPER_H
