#ifndef CORE_DIAGNOSTICCONTEXT_H
#define CORE_DIAGNOSTICCONTEXT_H

#include <algorithm>
#include <fmt/core.h>
#include <functional>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace core
{
class DiagnosticContext
{
  public:
    static DiagnosticContext& getInstance();

    template <typename T> void put(const std::string& property, const T& value)
    {
        const std::string valueStr = DiagnosticContext::valueToString(value);
        putInternal(property, valueStr);
    }

    void remove(const std::string& property);
    void clear();
    std::string format();

  private:
    DiagnosticContext();
    ~DiagnosticContext();

    // Detect if T supports operator<< to an ostream
    template <typename T>
    static auto has_ostream_impl(int)
        -> decltype(std::declval<std::ostream&>() << std::declval<T>(), std::true_type{});
    template <typename> static std::false_type has_ostream_impl(...);

    template <typename T>
    static constexpr bool has_ostream = decltype(has_ostream_impl<T>(0))::value;

    // Convert value to string using (in order): fmt if it's one of common types,
    // ostream operator<< if available, otherwise fallback to type info.
    template <typename T> static std::string valueToString(const T& v)
    {
        // Prefer fmt for common builtin and std::string types.
        if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string> ||
                      std::is_same_v<T, const char*> || std::is_same_v<T, char*>)
        {
            // fmt supports arithmetic and string types
            return fmt::format("{}", v);
        }
        else if constexpr (has_ostream<T>)
        {
            std::ostringstream ss;
            ss << v;
            return ss.str();
        }
        else
        {
            // Last resort: type name and pointer-like address representation if possible
            std::ostringstream ss;
            ss << "<unformattable:" << typeid(T).name() << ">";
            return ss.str();
        }
    }

    void putInternal(const std::string& property, const std::string& value);

  private:
    // property -> preformatted string representation of the stored value
    std::unordered_map<std::string, std::string> m_values;
    std::vector<std::string> m_keysSorted;

    // Cached formatted output and dirty flag
    std::string m_cached;
    bool m_isDirty = true;
};

class ScopedDiagnosticContext
{
  public:
    template <typename... Args>
        requires(sizeof...(Args) % 2 == 0) // Must be even number of arguments
    ScopedDiagnosticContext(Args&&... args)
    {
        process_pairs(std::forward<Args>(args)...);
    }
    ~ScopedDiagnosticContext();

  private:
    template <typename K, typename V, typename... Rest>
    void process_pairs(K&& key, V&& value, Rest&&... rest)
    {
        m_keys.push_back(std::forward<K>(key));
        DiagnosticContext::getInstance().put(std::forward<K>(key), std::forward<V>(value));
        if constexpr (sizeof...(Rest) > 0)
            process_pairs(std::forward<Rest>(rest)...);
    }

    std::vector<std::string> m_keys;
};

} // namespace core

#endif // CORE_DIAGNOSTICCONTEXT_H
