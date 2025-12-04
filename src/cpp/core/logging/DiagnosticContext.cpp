#include "DiagnosticContext.h"

using namespace core;

DiagnosticContext::DiagnosticContext()
{
    // constructor
}

DiagnosticContext::~DiagnosticContext()
{
    // destructor
}

DiagnosticContext& DiagnosticContext::getInstance()
{
    static thread_local DiagnosticContext context;
    return context;
}

void DiagnosticContext::remove(const std::string& property)
{
    if (m_values.erase(property) > 0)
    {
        m_isDirty = true;
        m_keysSorted.erase(std::remove(m_keysSorted.begin(), m_keysSorted.end(), property),
                           m_keysSorted.end());
    }
}

void DiagnosticContext::clear()
{
    if (!m_values.empty())
    {
        m_isDirty = true;
        m_values.clear();
        m_cached.clear();
        m_keysSorted.clear();
    }
}

// Return cached formatted string if nothing changed since last format() call.
std::string DiagnosticContext::format()
{
    if (!m_isDirty)
        return m_cached;

    if (m_values.empty())
    {
        m_cached.clear();
        m_isDirty = false;
        return std::string();
    }

    // Build formatted string: "k1=v1, k2=v2; ..."
    fmt::memory_buffer buf;
    bool first = true;
    for (const auto& k : m_keysSorted)
    {
        const std::string& v = m_values.at(k);
        if (!first)
            fmt::format_to(std::back_inserter(buf), ", ");
        first = false;
        fmt::format_to(std::back_inserter(buf), "{}={}", k, v);
    }

    m_cached = fmt::to_string(buf);
    m_isDirty = false;
    return m_cached;
}

void DiagnosticContext::putInternal(const std::string& property, const std::string& value)
{
    auto it = m_values.find(property);
    if (it == m_values.end() || it->second != value)
    {
        m_values[property] = value;
        m_isDirty = true;
    }

    if (it == m_values.end()) // new property
    {
        m_keysSorted.push_back(property);
        std::sort(m_keysSorted.begin(), m_keysSorted.end());
    }
}

ScopedDiagnosticContext::~ScopedDiagnosticContext()
{
    for (const auto& key : m_keys)
    {
        DiagnosticContext::getInstance().remove(key);
    }
}
