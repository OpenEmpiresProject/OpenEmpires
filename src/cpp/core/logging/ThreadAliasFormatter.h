#ifndef CORE_THREADALIASFORMATTER_H
#define CORE_THREADALIASFORMATTER_H

#include "spdlog/spdlog.h"

namespace core
{
class ThreadAliasFormatter : public spdlog::custom_flag_formatter
{
  public:
    void format(const spdlog::details::log_msg& msg,
                const std::tm&,
                spdlog::memory_buf_t& dest) override;

    std::unique_ptr<spdlog::custom_flag_formatter> clone() const override;

  private:
    class ThreadIndexInitializer
    {
      public:
        ThreadIndexInitializer();
        int getIndex() const;

      private:
        int m_threadIndex = -1;

        static int m_nextThreadIndex;
        static std::mutex m_mutex;
    };

    static thread_local ThreadIndexInitializer s_initializer;
};
} // namespace core

#endif // CORE_THREADALIASFORMATTER_H
