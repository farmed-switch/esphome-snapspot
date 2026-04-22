

#include "doctest_compatibility.h"

#include <locale>

#include <nlohmann/json.hpp>
using nlohmann::json;

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "make_test_data_available.hpp"

DOCTEST_CLANG_SUPPRESS_WARNING_PUSH
DOCTEST_CLANG_SUPPRESS_WARNING("-Wexit-time-destructors")

namespace
{
extern size_t calls;
size_t calls = 0;

void check_utf8dump(bool success_expected, int byte1, int byte2, int byte3, int byte4);

void check_utf8dump(bool success_expected, int byte1, int byte2 = -1, int byte3 = -1, int byte4 = -1)
{
    static std::string json_string;
    json_string.clear();

    CAPTURE(byte1)
    CAPTURE(byte2)
    CAPTURE(byte3)
    CAPTURE(byte4)

    json_string += std::string(1, static_cast<char>(byte1));

    if (byte2 != -1)
    {
        json_string += std::string(1, static_cast<char>(byte2));
    }

    if (byte3 != -1)
    {
        json_string += std::string(1, static_cast<char>(byte3));
    }

    if (byte4 != -1)
    {
        json_string += std::string(1, static_cast<char>(byte4));
    }

    CAPTURE(json_string)

    static json j;
    static json j2;
    j = json_string;
    j2 = "abc" + json_string + "xyz";

    static std::string s_ignored;
    static std::string s_ignored2;
    static std::string s_ignored_ascii;
    static std::string s_ignored2_ascii;
    static std::string s_replaced;
    static std::string s_replaced2;
    static std::string s_replaced_ascii;
    static std::string s_replaced2_ascii;

    s_ignored = j.dump(-1, ' ', false, json::error_handler_t::ignore);
    s_ignored2 = j2.dump(-1, ' ', false, json::error_handler_t::ignore);
    s_ignored_ascii = j.dump(-1, ' ', true, json::error_handler_t::ignore);
    s_ignored2_ascii = j2.dump(-1, ' ', true, json::error_handler_t::ignore);
    s_replaced = j.dump(-1, ' ', false, json::error_handler_t::replace);
    s_replaced2 = j2.dump(-1, ' ', false, json::error_handler_t::replace);
    s_replaced_ascii = j.dump(-1, ' ', true, json::error_handler_t::replace);
    s_replaced2_ascii = j2.dump(-1, ' ', true, json::error_handler_t::replace);

    if (success_expected)
    {
        static std::string s_strict;

        s_strict = j.dump();

        CHECK(s_strict == s_ignored);
        CHECK(s_strict == s_replaced);
    }
    else
    {

        CHECK_THROWS_AS(j.dump(), json::type_error&);

        CHECK(s_ignored != s_replaced);

        CHECK(s_replaced.find("\xEF\xBF\xBD") != std::string::npos);
    }

    CHECK(s_ignored2.substr(1, 3) == "abc");
    CHECK(s_ignored2.substr(s_ignored2.size() - 4, 3) == "xyz");
    CHECK(s_ignored2_ascii.substr(1, 3) == "abc");
    CHECK(s_ignored2_ascii.substr(s_ignored2_ascii.size() - 4, 3) == "xyz");
    CHECK(s_replaced2.substr(1, 3) == "abc");
    CHECK(s_replaced2.substr(s_replaced2.size() - 4, 3) == "xyz");
    CHECK(s_replaced2_ascii.substr(1, 3) == "abc");
    CHECK(s_replaced2_ascii.substr(s_replaced2_ascii.size() - 4, 3) == "xyz");
}

void check_utf8string(bool success_expected, int byte1, int byte2, int byte3, int byte4);

void check_utf8string(bool success_expected, int byte1, int byte2 = -1, int byte3 = -1, int byte4 = -1)
{
    if (++calls % 100000 == 0)
    {
        std::cout << calls << " of 1641521 UTF-8 strings checked" << std::endl;
    }

    static std::string json_string;
    json_string = "\"";

    CAPTURE(byte1)
    json_string += std::string(1, static_cast<char>(byte1));

    if (byte2 != -1)
    {
        CAPTURE(byte2)
        json_string += std::string(1, static_cast<char>(byte2));
    }

    if (byte3 != -1)
    {
        CAPTURE(byte3)
        json_string += std::string(1, static_cast<char>(byte3));
    }

    if (byte4 != -1)
    {
        CAPTURE(byte4)
        json_string += std::string(1, static_cast<char>(byte4));
    }

    json_string += "\"";

    CAPTURE(json_string)

    json _;
    if (success_expected)
    {
        CHECK_NOTHROW(_ = json::parse(json_string));
    }
    else
    {
        CHECK_THROWS_AS(_ = json::parse(json_string), json::parse_error&);
    }
}
}

TEST_CASE("Unicode (3/5)" * doctest::skip())
{
    SECTION("RFC 3629")
    {

        SECTION("UTF8-4 (xF0 x90-BF UTF8-tail UTF8-tail)")
        {
            SECTION("well-formed")
            {
                for (int byte1 = 0xF0; byte1 <= 0xF0; ++byte1)
                {
                    for (int byte2 = 0x90; byte2 <= 0xBF; ++byte2)
                    {
                        for (int byte3 = 0x80; byte3 <= 0xBF; ++byte3)
                        {
                            for (int byte4 = 0x80; byte4 <= 0xBF; ++byte4)
                            {
                                check_utf8string(true, byte1, byte2, byte3, byte4);
                                check_utf8dump(true, byte1, byte2, byte3, byte4);
                            }
                        }
                    }
                }
            }

            SECTION("ill-formed: missing second byte")
            {
                for (int byte1 = 0xF0; byte1 <= 0xF0; ++byte1)
                {
                    check_utf8string(false, byte1);
                    check_utf8dump(false, byte1);
                }
            }

            SECTION("ill-formed: missing third byte")
            {
                for (int byte1 = 0xF0; byte1 <= 0xF0; ++byte1)
                {
                    for (int byte2 = 0x90; byte2 <= 0xBF; ++byte2)
                    {
                        check_utf8string(false, byte1, byte2);
                        check_utf8dump(false, byte1, byte2);
                    }
                }
            }

            SECTION("ill-formed: missing fourth byte")
            {
                for (int byte1 = 0xF0; byte1 <= 0xF0; ++byte1)
                {
                    for (int byte2 = 0x90; byte2 <= 0xBF; ++byte2)
                    {
                        for (int byte3 = 0x80; byte3 <= 0xBF; ++byte3)
                        {
                            check_utf8string(false, byte1, byte2, byte3);
                            check_utf8dump(false, byte1, byte2, byte3);
                        }
                    }
                }
            }

            SECTION("ill-formed: wrong second byte")
            {
                for (int byte1 = 0xF0; byte1 <= 0xF0; ++byte1)
                {
                    for (int byte2 = 0x00; byte2 <= 0xFF; ++byte2)
                    {

                        if (0x90 <= byte2 && byte2 <= 0xBF)
                        {
                            continue;
                        }

                        for (int byte3 = 0x80; byte3 <= 0xBF; ++byte3)
                        {
                            for (int byte4 = 0x80; byte4 <= 0xBF; ++byte4)
                            {
                                check_utf8string(false, byte1, byte2, byte3, byte4);
                                check_utf8dump(false, byte1, byte2, byte3, byte4);
                            }
                        }
                    }
                }
            }

            SECTION("ill-formed: wrong third byte")
            {
                for (int byte1 = 0xF0; byte1 <= 0xF0; ++byte1)
                {
                    for (int byte2 = 0x90; byte2 <= 0xBF; ++byte2)
                    {
                        for (int byte3 = 0x00; byte3 <= 0xFF; ++byte3)
                        {

                            if (0x80 <= byte3 && byte3 <= 0xBF)
                            {
                                continue;
                            }

                            for (int byte4 = 0x80; byte4 <= 0xBF; ++byte4)
                            {
                                check_utf8string(false, byte1, byte2, byte3, byte4);
                                check_utf8dump(false, byte1, byte2, byte3, byte4);
                            }
                        }
                    }
                }
            }

            SECTION("ill-formed: wrong fourth byte")
            {
                for (int byte1 = 0xF0; byte1 <= 0xF0; ++byte1)
                {
                    for (int byte2 = 0x90; byte2 <= 0xBF; ++byte2)
                    {
                        for (int byte3 = 0x80; byte3 <= 0xBF; ++byte3)
                        {
                            for (int byte4 = 0x00; byte4 <= 0xFF; ++byte4)
                            {

                                if (0x80 <= byte3 && byte3 <= 0xBF)
                                {
                                    continue;
                                }

                                check_utf8string(false, byte1, byte2, byte3, byte4);
                                check_utf8dump(false, byte1, byte2, byte3, byte4);
                            }
                        }
                    }
                }
            }
        }
    }
}

DOCTEST_CLANG_SUPPRESS_WARNING_POP
