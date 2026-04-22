

#include "doctest_compatibility.h"

DOCTEST_GCC_SUPPRESS_WARNING_PUSH
DOCTEST_GCC_SUPPRESS_WARNING("-Wnoexcept")

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class sax_no_exception : public nlohmann::detail::json_sax_dom_parser<json>
{
  public:
    explicit sax_no_exception(json& j) : nlohmann::detail::json_sax_dom_parser<json>(j, false) {}

    static bool parse_error(std::size_t  , const std::string&  , const json::exception& ex)
    {
        error_string = new std::string(ex.what());
        return false;
    }

    static std::string* error_string;
};

std::string* sax_no_exception::error_string = nullptr;

TEST_CASE("Tests with disabled exceptions")
{
    SECTION("issue #2824 - encoding of json::exception::what()")
    {
        json j;
        sax_no_exception sax(j);

        CHECK (!json::sax_parse("xyz", &sax));
        CHECK(*sax_no_exception::error_string == "[json.exception.parse_error.101] parse error at line 1, column 1: syntax error while parsing value - invalid literal; last read: 'x'");
        delete sax_no_exception::error_string;
    }
}

DOCTEST_GCC_SUPPRESS_WARNING_POP
