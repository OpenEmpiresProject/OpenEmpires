#ifndef COMMAND_H
#define COMMAND_H

#include "GameSettings.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "components/UnitComponentRefs.h"
#include "utils/Types.h"

#include <entt/entity/registry.hpp>
#include <list>

namespace core
{
class Command
{
  public:
    static inline constexpr int DEFAULT_PRIORITY = 0;
    static inline constexpr int CHILD_PRIORITY_OFFSET = 1000;

    Command()
    {
        m_settings = ServiceRegistry::getInstance().getService<GameSettings>();
        m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    }

    virtual ~Command() = default;

    /**
     * @brief Called when the command is started.
     *
     * This pure virtual function should be implemented by derived classes to define
     * the behavior that occurs when the command begins execution.
     */
    virtual void onStart() = 0;
    /**
     * @brief Called when the command is added to the execution queue.
     *
     * This pure virtual function should be implemented by derived classes to define
     * behavior that occurs when the command is queued for execution.
     */
    virtual void onQueue() = 0;
    /**
     * @brief Executes the command logic.
     *
     * This pure virtual function should be implemented by derived classes to define
     * the specific behavior of the command when executed. It is called with the
     * elapsed time since the last execution and a list to which sub-commands can be added.
     *
     * @param deltaTimeMs The time elapsed since the last execution, in milliseconds.
     * @param subCommands A reference to a list where sub-commands generated during execution can be
     * added.
     * @return true if the command completed, false otherwise.
     */
    virtual bool onExecute(int deltaTimeMs, std::list<Command*>& subCommands) = 0;
    virtual std::string toString() const = 0;
    virtual void destroy() = 0;

    void setPriority(int m_priority)
    {
        this->m_priority = m_priority;
    }

    int getPriority() const
    {
        return m_priority;
    }

    static void incrementTotalTicks()
    {
        s_totalTicks++;
    }

    void setEntityID(uint32_t entityID)
    {
        m_entityID = entityID;
        m_components = std::make_shared<UnitComponentRefs>(m_gameState, entityID);
    }

    uint32_t getEntityID() const
    {
        return m_entityID;
    }

    inline bool isExecutedAtLeastOnce() const
    {
        return m_executedAtLeastOnce;
    }

    inline void setExecutedAtLeastOnce(bool executed)
    {
        m_executedAtLeastOnce = executed;
    }

  protected:
    inline static int s_totalTicks = 0;

    std::shared_ptr<GameSettings> m_settings;
    std::shared_ptr<GameState> m_gameState;
    int m_priority = -1;
    uint32_t m_entityID = entt::null;
    Ref<UnitComponentRefs> m_components;

    bool m_executedAtLeastOnce = false;
};

// Comparator to use Commands in priority_queue
struct CompareTaskPtr
{
    bool operator()(const Command* a, const Command* b) const
    {
        return a->getPriority() < b->getPriority(); // max-heap: higher m_priority = top
    }
};

} // namespace core

#endif