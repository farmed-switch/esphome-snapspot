

#include "doctest_compatibility.h"

#include <locale>
#include <nlohmann/json.hpp>
using nlohmann::json;

#include <fstream>
#include <sstream>
#include <iomanip>
#include "make_test_data_available.hpp"

TEST_CASE("Unicode (1/5)" * doctest::skip())
{
    SECTION("\\uxxxx sequences")
    {

        const auto codepoint_to_unicode = [](std::size_t cp)
        {

            std::stringstream ss;
            ss << "\\u" << std::setw(4) << std::setfill('0') << std::hex << cp;
            return ss.str();
        };

        SECTION("correct sequences")
        {

            for (std::size_t cp = 0; cp <= 0x10FFFFu; ++cp)
            {

                std::string json_text = "\"";

                if (cp < 0x10000u)
                {

                    if (cp >= 0xD800u && cp <= 0xDFFFu)
                    {

                        continue;
                    }

                    json_text += codepoint_to_unicode(cp);
                }
                else
                {

                    const auto codepoint1 = 0xd800u + (((cp - 0x10000u) >> 10) & 0x3ffu);
                    const auto codepoint2 = 0xdc00u + ((cp - 0x10000u) & 0x3ffu);
                    json_text += codepoint_to_unicode(codepoint1) + codepoint_to_unicode(codepoint2);
                }

                json_text += "\"";
                CAPTURE(json_text)
                json _;
                CHECK_NOTHROW(_ = json::parse(json_text));
            }
        }

        SECTION("incorrect sequences")
        {
            SECTION("incorrect surrogate values")
            {
                json _;

                CHECK_THROWS_WITH_AS(_ = json::parse("\"\\uDC00\\uDC00\""), "[json.exception.parse_error.101] parse error at line 1, column 7: syntax error while parsing value - invalid string: surrogate U+DC00..U+DFFF must follow U+D800..U+DBFF; last read: '\"\\uDC00'", json::parse_error&);

                CHECK_THROWS_WITH_AS(_ = json::parse("\"\\uD7FF\\uDC00\""), "[json.exception.parse_error.101] parse error at line 1, column 13: syntax error while parsing value - invalid string: surrogate U+DC00..U+DFFF must follow U+D800..U+DBFF; last read: '\"\\uD7FF\\uDC00'", json::parse_error&);

                CHECK_THROWS_WITH_AS(_ = json::parse("\"\\uD800]\""), "[json.exception.parse_error.101] parse error at line 1, column 8: syntax error while parsing value - invalid string: surrogate U+D800..U+DBFF must be followed by U+DC00..U+DFFF; last read: '\"\\uD800]'", json::parse_error&);

                CHECK_THROWS_WITH_AS(_ = json::parse("\"\\uD800\\v\""), "[json.exception.parse_error.101] parse error at line 1, column 9: syntax error while parsing value - invalid string: surrogate U+D800..U+DBFF must be followed by U+DC00..U+DFFF; last read: '\"\\uD800\\v'", json::parse_error&);

                CHECK_THROWS_WITH_AS(_ = json::parse("\"\\uD800\\u123\""), "[json.exception.parse_error.101] parse error at line 1, column 13: syntax error while parsing value - invalid string: '\\u' must be followed by 4 hex digits; last read: '\"\\uD800\\u123\"'", json::parse_error&);

                CHECK_THROWS_WITH_AS(_ = json::parse("\"\\uD800\\uDBFF\""), "[json.exception.parse_error.101] parse error at line 1, column 13: syntax error while parsing value - invalid string: surrogate U+D800..U+DBFF must be followed by U+DC00..U+DFFF; last read: '\"\\uD800\\uDBFF'", json::parse_error&);

                CHECK_THROWS_WITH_AS(_ = json::parse("\"\\uD800\\uE000\""), "[json.exception.parse_error.101] parse error at line 1, column 13: syntax error while parsing value - invalid string: surrogate U+D800..U+DBFF must be followed by U+DC00..U+DFFF; last read: '\"\\uD800\\uE000'", json::parse_error&);
            }
        }

#if 0
        SECTION("incorrect sequences")
        {
            SECTION("high surrogate without low surrogate")
            {

                for (std::size_t cp = 0xD800u; cp <= 0xDBFFu; ++cp)
                {
                    std::string json_text = "\"" + codepoint_to_unicode(cp) + "\"";
                    CAPTURE(json_text)
                    CHECK_THROWS_AS(json::parse(json_text), json::parse_error&);
                }
            }

            SECTION("high surrogate with wrong low surrogate")
            {

                for (std::size_t cp1 = 0xD800u; cp1 <= 0xDBFFu; ++cp1)
                {
                    for (std::size_t cp2 = 0x0000u; cp2 <= 0xFFFFu; ++cp2)
                    {
                        if (0xDC00u <= cp2 && cp2 <= 0xDFFFu)
                        {
                            continue;
                        }

                        std::string json_text = "\"" + codepoint_to_unicode(cp1) + codepoint_to_unicode(cp2) + "\"";
                        CAPTURE(json_text)
                        CHECK_THROWS_AS(json::parse(json_text), json::parse_error&);
                    }
                }
            }

            SECTION("low surrogate without high surrogate")
            {

                for (std::size_t cp = 0xDC00u; cp <= 0xDFFFu; ++cp)
                {
                    std::string json_text = "\"" + codepoint_to_unicode(cp) + "\"";
                    CAPTURE(json_text)
                    CHECK_THROWS_AS(json::parse(json_text), json::parse_error&);
                }
            }

        }
#endif
    }

    SECTION("read all unicode characters")
    {

        std::ifstream f(TEST_DATA_DIRECTORY "/json_nlohmann_tests/all_unicode.json");
        json j;
        CHECK_NOTHROW(f >> j);

        CHECK(j.size() == 1112065);

        SECTION("check JSON Pointers")
        {
            for (const auto& s : j)
            {

                if (!s.is_string())
                {
                    continue;
                }

                auto ptr = s.get<std::string>();

                if (ptr == "~")
                {
                    ptr += "0";
                }

                ptr.insert(0, "/");

                CHECK_NOTHROW(json::json_pointer("/" + ptr));

                auto escaped = nlohmann::detail::escape(ptr);
                nlohmann::detail::unescape(escaped);
                CHECK(escaped == ptr);
            }
        }
    }

    SECTION("ignore byte-order-mark")
    {
        SECTION("in a stream")
        {

            std::ifstream f(TEST_DATA_DIRECTORY "/json_nlohmann_tests/bom.json");
            json j;
            CHECK_NOTHROW(f >> j);
        }

        SECTION("with an iterator")
        {
            std::string i = "\xef\xbb\xbf{\n   \"foo\": true\n}";
            json _;
            CHECK_NOTHROW(_ = json::parse(i.begin(), i.end()));
        }
    }

    SECTION("error for incomplete/wrong BOM")
    {
        json _;
        CHECK_THROWS_AS(_ = json::parse("\xef\xbb"), json::parse_error&);
        CHECK_THROWS_AS(_ = json::parse("\xef\xbb\xbb"), json::parse_error&);
    }
}

namespace
{
void roundtrip(bool success_expected, const std::string& s);

void roundtrip(bool success_expected, const std::string& s)
{
    CAPTURE(s)
    json _;

    const json j = s;

    const std::string ps = std::string("\"") + s + "\"";

    if (success_expected)
    {

        CHECK_NOTHROW(j.dump());

        if (s[0] != '\0')
        {

            CHECK_NOTHROW(_ = json::parse(ps));
        }

        CHECK_NOTHROW(_ = json::parse(j.dump()));

        const json jr = json::parse(j.dump());
        CHECK(jr.get<std::string>() == s);
    }
    else
    {

        CHECK_THROWS_AS(j.dump(), json::type_error&);

        CHECK_THROWS_AS(_ = json::parse(ps), json::parse_error&);
    }
}
}

TEST_CASE("Markus Kuhn's UTF-8 decoder capability and stress test")
{

    SECTION("1  Some correct UTF-8 text")
    {
        roundtrip(true, "κόσμε");
    }

    SECTION("2  Boundary condition test cases")
    {
        SECTION("2.1  First possible sequence of a certain length")
        {

            roundtrip(true, std::string("\0", 1));

            roundtrip(true, "\xc2\x80");

            roundtrip(true, "\xe0\xa0\x80");

            roundtrip(true, "\xf0\x90\x80\x80");

            roundtrip(false, "\xF8\x88\x80\x80\x80");

            roundtrip(false, "\xFC\x84\x80\x80\x80\x80");
        }

        SECTION("2.2  Last possible sequence of a certain length")
        {

            roundtrip(true, "\x7f");

            roundtrip(true, "\xdf\xbf");

            roundtrip(true, "\xef\xbf\xbf");

            roundtrip(false, "\xF7\xBF\xBF\xBF");

            roundtrip(false, "\xFB\xBF\xBF\xBF\xBF");

            roundtrip(false, "\xFD\xBF\xBF\xBF\xBF\xBF");
        }

        SECTION("2.3  Other boundary conditions")
        {

            roundtrip(true, "\xed\x9f\xbf");

            roundtrip(true, "\xee\x80\x80");

            roundtrip(true, "\xef\xbf\xbd");

            roundtrip(true, "\xf4\x8f\xbf\xbf");

            roundtrip(false, "\xf4\x90\x80\x80");
        }
    }

    SECTION("3  Malformed sequences")
    {
        SECTION("3.1  Unexpected continuation bytes")
        {

            roundtrip(false, "\x80");

            roundtrip(false, "\xbf");

            roundtrip(false, "\x80\xbf");

            roundtrip(false, "\x80\xbf\x80");

            roundtrip(false, "\x80\xbf\x80\xbf");

            roundtrip(false, "\x80\xbf\x80\xbf\x80");

            roundtrip(false, "\x80\xbf\x80\xbf\x80\xbf");

            roundtrip(false, "\x80\xbf\x80\xbf\x80\xbf\x80");

            roundtrip(false, "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf");
        }

        SECTION("3.2  Lonely start characters")
        {

            roundtrip(false, "\xc0 \xc1 \xc2 \xc3 \xc4 \xc5 \xc6 \xc7 \xc8 \xc9 \xca \xcb \xcc \xcd \xce \xcf \xd0 \xd1 \xd2 \xd3 \xd4 \xd5 \xd6 \xd7 \xd8 \xd9 \xda \xdb \xdc \xdd \xde \xdf");

            roundtrip(false, "\xe0 \xe1 \xe2 \xe3 \xe4 \xe5 \xe6 \xe7 \xe8 \xe9 \xea \xeb \xec \xed \xee \xef");

            roundtrip(false, "\xf0 \xf1 \xf2 \xf3 \xf4 \xf5 \xf6 \xf7");

            roundtrip(false, "\xf8 \xf9 \xfa \xfb");

            roundtrip(false, "\xfc \xfd");
        }

        SECTION("3.3  Sequences with last continuation byte missing")
        {

            roundtrip(false, "\xc0");

            roundtrip(false, "\xe0\x80");

            roundtrip(false, "\xf0\x80\x80");

            roundtrip(false, "\xf8\x80\x80\x80");

            roundtrip(false, "\xfc\x80\x80\x80\x80");

            roundtrip(false, "\xdf");

            roundtrip(false, "\xef\xbf");

            roundtrip(false, "\xf7\xbf\xbf");

            roundtrip(false, "\xfb\xbf\xbf\xbf");

            roundtrip(false, "\xfd\xbf\xbf\xbf\xbf");
        }

        SECTION("3.4  Concatenation of incomplete sequences")
        {

            roundtrip(false, "\xc0\xe0\x80\xf0\x80\x80\xf8\x80\x80\x80\xfc\x80\x80\x80\x80\xdf\xef\xbf\xf7\xbf\xbf\xfb\xbf\xbf\xbf\xfd\xbf\xbf\xbf\xbf");
        }

        SECTION("3.5  Impossible bytes")
        {

            roundtrip(false, "\xfe");

            roundtrip(false, "\xff");

            roundtrip(false, "\xfe\xfe\xff\xff");
        }
    }

    SECTION("4  Overlong sequences")
    {

        SECTION("4.1  Examples of an overlong ASCII character")
        {

            roundtrip(false, "\xc0\xaf");

            roundtrip(false, "\xe0\x80\xaf");

            roundtrip(false, "\xf0\x80\x80\xaf");

            roundtrip(false, "\xf8\x80\x80\x80\xaf");

            roundtrip(false, "\xfc\x80\x80\x80\x80\xaf");
        }

        SECTION("4.2  Maximum overlong sequences")
        {

            roundtrip(false, "\xc1\xbf");

            roundtrip(false, "\xe0\x9f\xbf");

            roundtrip(false, "\xf0\x8f\xbf\xbf");

            roundtrip(false, "\xf8\x87\xbf\xbf\xbf");

            roundtrip(false, "\xfc\x83\xbf\xbf\xbf\xbf");
        }

        SECTION("4.3  Overlong representation of the NUL character")
        {

            roundtrip(false, "\xc0\x80");

            roundtrip(false, "\xe0\x80\x80");

            roundtrip(false, "\xf0\x80\x80\x80");

            roundtrip(false, "\xf8\x80\x80\x80\x80");

            roundtrip(false, "\xfc\x80\x80\x80\x80\x80");
        }
    }

    SECTION("5  Illegal code positions")
    {

        SECTION("5.1 Single UTF-16 surrogates")
        {

            roundtrip(false, "\xed\xa0\x80");

            roundtrip(false, "\xed\xad\xbf");

            roundtrip(false, "\xed\xae\x80");

            roundtrip(false, "\xed\xaf\xbf");

            roundtrip(false, "\xed\xb0\x80");

            roundtrip(false, "\xed\xbe\x80");

            roundtrip(false, "\xed\xbf\xbf");
        }

        SECTION("5.2 Paired UTF-16 surrogates")
        {

            roundtrip(false, "\xed\xa0\x80\xed\xb0\x80");

            roundtrip(false, "\xed\xa0\x80\xed\xbf\xbf");

            roundtrip(false, "\xed\xad\xbf\xed\xb0\x80");

            roundtrip(false, "\xed\xad\xbf\xed\xbf\xbf");

            roundtrip(false, "\xed\xae\x80\xed\xb0\x80");

            roundtrip(false, "\xed\xae\x80\xed\xbf\xbf");

            roundtrip(false, "\xed\xaf\xbf\xed\xb0\x80");

            roundtrip(false, "\xed\xaf\xbf\xed\xbf\xbf");
        }

        SECTION("5.3 Noncharacter code positions")
        {

            roundtrip(true, "\xef\xbf\xbe");

            roundtrip(true, "\xef\xbf\xbf");

            roundtrip(true, "\xEF\xB7\x90");
            roundtrip(true, "\xEF\xB7\x91");
            roundtrip(true, "\xEF\xB7\x92");
            roundtrip(true, "\xEF\xB7\x93");
            roundtrip(true, "\xEF\xB7\x94");
            roundtrip(true, "\xEF\xB7\x95");
            roundtrip(true, "\xEF\xB7\x96");
            roundtrip(true, "\xEF\xB7\x97");
            roundtrip(true, "\xEF\xB7\x98");
            roundtrip(true, "\xEF\xB7\x99");
            roundtrip(true, "\xEF\xB7\x9A");
            roundtrip(true, "\xEF\xB7\x9B");
            roundtrip(true, "\xEF\xB7\x9C");
            roundtrip(true, "\xEF\xB7\x9D");
            roundtrip(true, "\xEF\xB7\x9E");
            roundtrip(true, "\xEF\xB7\x9F");
            roundtrip(true, "\xEF\xB7\xA0");
            roundtrip(true, "\xEF\xB7\xA1");
            roundtrip(true, "\xEF\xB7\xA2");
            roundtrip(true, "\xEF\xB7\xA3");
            roundtrip(true, "\xEF\xB7\xA4");
            roundtrip(true, "\xEF\xB7\xA5");
            roundtrip(true, "\xEF\xB7\xA6");
            roundtrip(true, "\xEF\xB7\xA7");
            roundtrip(true, "\xEF\xB7\xA8");
            roundtrip(true, "\xEF\xB7\xA9");
            roundtrip(true, "\xEF\xB7\xAA");
            roundtrip(true, "\xEF\xB7\xAB");
            roundtrip(true, "\xEF\xB7\xAC");
            roundtrip(true, "\xEF\xB7\xAD");
            roundtrip(true, "\xEF\xB7\xAE");
            roundtrip(true, "\xEF\xB7\xAF");

            roundtrip(true, "\xF0\x9F\xBF\xBF");
            roundtrip(true, "\xF0\xAF\xBF\xBF");
            roundtrip(true, "\xF0\xBF\xBF\xBF");
            roundtrip(true, "\xF1\x8F\xBF\xBF");
            roundtrip(true, "\xF1\x9F\xBF\xBF");
            roundtrip(true, "\xF1\xAF\xBF\xBF");
            roundtrip(true, "\xF1\xBF\xBF\xBF");
            roundtrip(true, "\xF2\x8F\xBF\xBF");
            roundtrip(true, "\xF2\x9F\xBF\xBF");
            roundtrip(true, "\xF2\xAF\xBF\xBF");
        }
    }
}
