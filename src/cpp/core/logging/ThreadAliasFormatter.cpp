#include "ThreadAliasFormatter.h"

using namespace core;

int ThreadAliasFormatter::ThreadIndexInitializer::m_nextThreadIndex = 0;
std::mutex ThreadAliasFormatter::ThreadIndexInitializer::m_mutex;
thread_local ThreadAliasFormatter::ThreadIndexInitializer ThreadAliasFormatter::s_initializer;

void ThreadAliasFormatter::format(const spdlog::details::log_msg& msg,
                                  const std::tm&,
                                  spdlog::memory_buf_t& dest)
{
    fmt::format_to(std::back_inserter(dest), "T{}", s_initializer.getIndex());
}

std::unique_ptr<spdlog::custom_flag_formatter> ThreadAliasFormatter::clone() const
{
    return std::make_unique<ThreadAliasFormatter>();
}

ThreadAliasFormatter::ThreadIndexInitializer::ThreadIndexInitializer()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_threadIndex = m_nextThreadIndex++;
}

int ThreadAliasFormatter::ThreadIndexInitializer::getIndex() const
{
    return m_threadIndex;
}
