

#pragma once

#include <cstddef>
#include <exception>
#if JSON_DIAGNOSTICS
    #include <numeric>
#endif
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/detail/value_t.hpp>
#include <nlohmann/detail/string_escape.hpp>
#include <nlohmann/detail/input/position_t.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/detail/meta/cpp_future.hpp>
#include <nlohmann/detail/meta/type_traits.hpp>
#include <nlohmann/detail/string_concat.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN
namespace detail
{

class exception : public std::exception
{
  public:

    const char* what() const noexcept override
    {
        return m.what();
    }

    const int id;

  protected:
    JSON_HEDLEY_NON_NULL(3)
    exception(int id_, const char* what_arg) : id(id_), m(what_arg) {}

    static std::string name(const std::string& ename, int id_)
    {
        return concat("[json.exception.", ename, '.', std::to_string(id_), "] ");
    }

    static std::string diagnostics(std::nullptr_t  )
    {
        return "";
    }

    template<typename BasicJsonType>
    static std::string diagnostics(const BasicJsonType* leaf_element)
    {
#if JSON_DIAGNOSTICS
        std::vector<std::string> tokens;
        for (const auto* current = leaf_element; current != nullptr && current->m_parent != nullptr; current = current->m_parent)
        {
            switch (current->m_parent->type())
            {
                case value_t::array:
                {
                    for (std::size_t i = 0; i < current->m_parent->m_value.array->size(); ++i)
                    {
                        if (&current->m_parent->m_value.array->operator[](i) == current)
                        {
                            tokens.emplace_back(std::to_string(i));
                            break;
                        }
                    }
                    break;
                }

                case value_t::object:
                {
                    for (const auto& element : *current->m_parent->m_value.object)
                    {
                        if (&element.second == current)
                        {
                            tokens.emplace_back(element.first.c_str());
                            break;
                        }
                    }
                    break;
                }

                case value_t::null:
                case value_t::string:
                case value_t::boolean:
                case value_t::number_integer:
                case value_t::number_unsigned:
                case value_t::number_float:
                case value_t::binary:
                case value_t::discarded:
                default:
                    break;
            }
        }

        if (tokens.empty())
        {
            return "";
        }

        auto str = std::accumulate(tokens.rbegin(), tokens.rend(), std::string{},
                                   [](const std::string & a, const std::string & b)
        {
            return concat(a, '/', detail::escape(b));
        });
        return concat('(', str, ") ");
#else
        static_cast<void>(leaf_element);
        return "";
#endif
    }

  private:

    std::runtime_error m;
};

class parse_error : public exception
{
  public:

    template<typename BasicJsonContext, enable_if_t<is_basic_json_context<BasicJsonContext>::value, int> = 0>
    static parse_error create(int id_, const position_t& pos, const std::string& what_arg, BasicJsonContext context)
    {
        const std::string w = concat(exception::name("parse_error", id_), "parse error",
                                     position_string(pos), ": ", exception::diagnostics(context), what_arg);
        return {id_, pos.chars_read_total, w.c_str()};
    }

    template<typename BasicJsonContext, enable_if_t<is_basic_json_context<BasicJsonContext>::value, int> = 0>
    static parse_error create(int id_, std::size_t byte_, const std::string& what_arg, BasicJsonContext context)
    {
        const std::string w = concat(exception::name("parse_error", id_), "parse error",
                                     (byte_ != 0 ? (concat(" at byte ", std::to_string(byte_))) : ""),
                                     ": ", exception::diagnostics(context), what_arg);
        return {id_, byte_, w.c_str()};
    }

    const std::size_t byte;

  private:
    parse_error(int id_, std::size_t byte_, const char* what_arg)
        : exception(id_, what_arg), byte(byte_) {}

    static std::string position_string(const position_t& pos)
    {
        return concat(" at line ", std::to_string(pos.lines_read + 1),
                      ", column ", std::to_string(pos.chars_read_current_line));
    }
};

class invalid_iterator : public exception
{
  public:
    template<typename BasicJsonContext, enable_if_t<is_basic_json_context<BasicJsonContext>::value, int> = 0>
    static invalid_iterator create(int id_, const std::string& what_arg, BasicJsonContext context)
    {
        const std::string w = concat(exception::name("invalid_iterator", id_), exception::diagnostics(context), what_arg);
        return {id_, w.c_str()};
    }

  private:
    JSON_HEDLEY_NON_NULL(3)
    invalid_iterator(int id_, const char* what_arg)
        : exception(id_, what_arg) {}
};

class type_error : public exception
{
  public:
    template<typename BasicJsonContext, enable_if_t<is_basic_json_context<BasicJsonContext>::value, int> = 0>
    static type_error create(int id_, const std::string& what_arg, BasicJsonContext context)
    {
        const std::string w = concat(exception::name("type_error", id_), exception::diagnostics(context), what_arg);
        return {id_, w.c_str()};
    }

  private:
    JSON_HEDLEY_NON_NULL(3)
    type_error(int id_, const char* what_arg) : exception(id_, what_arg) {}
};

class out_of_range : public exception
{
  public:
    template<typename BasicJsonContext, enable_if_t<is_basic_json_context<BasicJsonContext>::value, int> = 0>
    static out_of_range create(int id_, const std::string& what_arg, BasicJsonContext context)
    {
        const std::string w = concat(exception::name("out_of_range", id_), exception::diagnostics(context), what_arg);
        return {id_, w.c_str()};
    }

  private:
    JSON_HEDLEY_NON_NULL(3)
    out_of_range(int id_, const char* what_arg) : exception(id_, what_arg) {}
};

class other_error : public exception
{
  public:
    template<typename BasicJsonContext, enable_if_t<is_basic_json_context<BasicJsonContext>::value, int> = 0>
    static other_error create(int id_, const std::string& what_arg, BasicJsonContext context)
    {
        const std::string w = concat(exception::name("other_error", id_), exception::diagnostics(context), what_arg);
        return {id_, w.c_str()};
    }

  private:
    JSON_HEDLEY_NON_NULL(3)
    other_error(int id_, const char* what_arg) : exception(id_, what_arg) {}
};

}
NLOHMANN_JSON_NAMESPACE_END
