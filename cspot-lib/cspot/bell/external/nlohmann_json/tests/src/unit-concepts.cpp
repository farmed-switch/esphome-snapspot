

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>
using nlohmann::json;

TEST_CASE("concepts")
{
    SECTION("container requirements for json")
    {

        CHECK((std::is_same<json::value_type, json>::value));

        CHECK((std::is_same<json::reference, json&>::value));

        CHECK((std::is_same<json::const_reference, const json&>::value));

        CHECK((std::is_same<json::iterator::value_type, json>::value));

        CHECK((std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<json::iterator>::iterator_category>::value));

        CHECK((std::is_convertible<json::iterator, json::const_iterator>::value));

        CHECK((std::is_same<json::const_iterator::value_type, json>::value));

        CHECK((std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<json::const_iterator>::iterator_category>::value));

        CHECK((std::is_signed<json::difference_type>::value));

        CHECK((std::is_same<json::difference_type, json::iterator::difference_type>::value));

        CHECK((std::is_same<json::difference_type, json::const_iterator::difference_type>::value));

        CHECK((std::is_unsigned<json::size_type>::value));

        CHECK(static_cast<json::size_type>((std::numeric_limits<json::difference_type>::max)()) <=
              (std::numeric_limits<json::size_type>::max)());

        {
            const json u;
            CHECK(u.empty());
        }

        CHECK(json().empty());
    }

    SECTION("class json")
    {
        SECTION("DefaultConstructible")
        {
            CHECK(std::is_nothrow_default_constructible<json>::value);
        }

        SECTION("MoveConstructible")
        {
            CHECK(std::is_move_constructible<json>::value);
            CHECK(std::is_nothrow_move_constructible<json>::value);
        }

        SECTION("CopyConstructible")
        {
            CHECK(std::is_copy_constructible<json>::value);
        }

        SECTION("MoveAssignable")
        {
            CHECK(std::is_nothrow_move_assignable<json>::value);
        }

        SECTION("CopyAssignable")
        {
            CHECK(std::is_copy_assignable<json>::value);
        }

        SECTION("Destructible")
        {
            CHECK(std::is_nothrow_destructible<json>::value);
        }

        SECTION("StandardLayoutType")
        {
            CHECK(std::is_standard_layout<json>::value);
        }
    }

    SECTION("class iterator")
    {
        SECTION("CopyConstructible")
        {
            CHECK(std::is_nothrow_copy_constructible<json::iterator>::value);
            CHECK(std::is_nothrow_copy_constructible<json::const_iterator>::value);
        }

        SECTION("CopyAssignable")
        {

#if !defined(_MSC_VER) || (_ITERATOR_DEBUG_LEVEL == 0)
            CHECK(std::is_nothrow_copy_assignable<json::iterator>::value);
            CHECK(std::is_nothrow_copy_assignable<json::const_iterator>::value);
#endif
        }

        SECTION("Destructible")
        {
            CHECK(std::is_nothrow_destructible<json::iterator>::value);
            CHECK(std::is_nothrow_destructible<json::const_iterator>::value);
        }

        SECTION("Swappable")
        {
            {
                json j {1, 2, 3};
                json::iterator it1 = j.begin();
                json::iterator it2 = j.end();
                swap(it1, it2);
                CHECK(it1 == j.end());
                CHECK(it2 == j.begin());
            }
            {
                json j {1, 2, 3};
                json::const_iterator it1 = j.cbegin();
                json::const_iterator it2 = j.cend();
                swap(it1, it2);
                CHECK(it1 == j.end());
                CHECK(it2 == j.begin());
            }
        }
    }
}
