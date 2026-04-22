

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>
using nlohmann::detail::dtoa_impl::reinterpret_bits;

namespace
{
float make_float(uint32_t sign_bit, uint32_t biased_exponent, uint32_t significand)
{
    assert(sign_bit == 0 || sign_bit == 1);
    assert(biased_exponent <= 0xFF);
    assert(significand <= 0x007FFFFF);

    uint32_t bits = 0;

    bits |= sign_bit << 31;
    bits |= biased_exponent << 23;
    bits |= significand;

    return reinterpret_bits<float>(bits);
}

float make_float(uint64_t f, int e)
{
    constexpr uint64_t kHiddenBit               = 0x00800000;
    constexpr uint64_t kSignificandMask         = 0x007FFFFF;
    constexpr int      kPhysicalSignificandSize = 23;
    constexpr int      kExponentBias            = 0x7F + kPhysicalSignificandSize;
    constexpr int      kDenormalExponent        = 1 -    kExponentBias;
    constexpr int      kMaxExponent             = 0xFF - kExponentBias;

    while (f > kHiddenBit + kSignificandMask)
    {
        f >>= 1;
        e++;
    }
    if (e >= kMaxExponent)
    {
        return std::numeric_limits<float>::infinity();
    }
    if (e < kDenormalExponent)
    {
        return 0.0;
    }
    while (e > kDenormalExponent && (f & kHiddenBit) == 0)
    {
        f <<= 1;
        e--;
    }

    const uint64_t biased_exponent = (e == kDenormalExponent && (f & kHiddenBit) == 0)
                                     ? 0
                                     : static_cast<uint64_t>(e + kExponentBias);

    const uint64_t bits = (f & kSignificandMask) | (biased_exponent << kPhysicalSignificandSize);
    return reinterpret_bits<float>(static_cast<uint32_t>(bits));
}

double make_double(uint64_t sign_bit, uint64_t biased_exponent, uint64_t significand)
{
    assert(sign_bit == 0 || sign_bit == 1);
    assert(biased_exponent <= 0x7FF);
    assert(significand <= 0x000FFFFFFFFFFFFF);

    uint64_t bits = 0;

    bits |= sign_bit << 63;
    bits |= biased_exponent << 52;
    bits |= significand;

    return reinterpret_bits<double>(bits);
}

double make_double(uint64_t f, int e)
{
    constexpr uint64_t kHiddenBit               = 0x0010000000000000;
    constexpr uint64_t kSignificandMask         = 0x000FFFFFFFFFFFFF;
    constexpr int      kPhysicalSignificandSize = 52;
    constexpr int      kExponentBias            = 0x3FF + kPhysicalSignificandSize;
    constexpr int      kDenormalExponent        = 1     - kExponentBias;
    constexpr int      kMaxExponent             = 0x7FF - kExponentBias;

    while (f > kHiddenBit + kSignificandMask)
    {
        f >>= 1;
        e++;
    }
    if (e >= kMaxExponent)
    {
        return std::numeric_limits<double>::infinity();
    }
    if (e < kDenormalExponent)
    {
        return 0.0;
    }
    while (e > kDenormalExponent && (f & kHiddenBit) == 0)
    {
        f <<= 1;
        e--;
    }

    const uint64_t biased_exponent = (e == kDenormalExponent && (f & kHiddenBit) == 0)
                                     ? 0
                                     : static_cast<uint64_t>(e + kExponentBias);

    const uint64_t bits = (f & kSignificandMask) | (biased_exponent << kPhysicalSignificandSize);
    return reinterpret_bits<double>(bits);
}
}

TEST_CASE("digit gen")
{
    SECTION("single precision")
    {
        auto check_float = [](float number, const std::string & digits, int expected_exponent)
        {
            CAPTURE(number)
            CAPTURE(digits)
            CAPTURE(expected_exponent)

            std::array<char, 32> buf{};
            int len = 0;
            int exponent = 0;
            nlohmann::detail::dtoa_impl::grisu2(buf.data(), len, exponent, number);

            CHECK(digits == std::string(buf.data(), buf.data() + len));
            CHECK(expected_exponent == exponent);
        };

        check_float(make_float(0,   0, 0x00000001),        "1", -45);
        check_float(make_float(0,   0, 0x007FFFFF), "11754942", -45);
        check_float(make_float(0,   1, 0x00000000), "11754944", -45);
        check_float(make_float(0,   1, 0x00000001), "11754945", -45);
        check_float(make_float(0,   1, 0x007FFFFF), "23509886", -45);
        check_float(make_float(0,   2, 0x00000000), "23509887", -45);
        check_float(make_float(0,   2, 0x00000001),  "2350989", -44);
        check_float(make_float(0,  24, 0x00000000), "98607613", -39);
        check_float(make_float(0,  30, 0x00000000), "63108872", -37);
        check_float(make_float(0,  31, 0x00000000), "12621775", -36);
        check_float(make_float(0,  57, 0x00000000), "84703295", -29);
        check_float(make_float(0, 254, 0x007FFFFE), "34028233",  31);
        check_float(make_float(0, 254, 0x007FFFFF), "34028235",  31);

        check_float(make_float(12676506, -102),       "25", -25);
        check_float(make_float(12676506, -103),      "125", -26);
        check_float(make_float(15445013,   86),     "1195",  30);
        check_float(make_float(13734123, -138),    "39415", -39);
        check_float(make_float(12428269, -130),   "913085", -38);
        check_float(make_float(15334037, -146),  "1719005", -43);
        check_float(make_float(11518287,  -41), "52379105", -13);
        check_float(make_float(12584953, -145),  "2821644", -43);
        check_float(make_float(15961084, -125), "37524328", -38);
        check_float(make_float(14915817, -146), "16721209", -44);
        check_float(make_float(10845484, -102), "21388946", -31);
        check_float(make_float(16431059,  -61),  "7125836", -18);

        check_float(make_float(16093626,   69),       "95",  26);
        check_float(make_float( 9983778,   25),      "335",  12);
        check_float(make_float(12745034,  104),     "2585",  35);
        check_float(make_float(12706553,   72),    "60005",  24);
        check_float(make_float(11005028,   45),   "387205",  15);
        check_float(make_float(15059547,   71),  "3555835",  22);
        check_float(make_float(16015691,  -99), "25268305", -30);
        check_float(make_float( 8667859,   56),  "6245851",  17);
        check_float(make_float(14855922,  -82), "30721327", -25);
        check_float(make_float(14855922,  -83), "15360663", -25);
        check_float(make_float(10144164, -110),   "781478", -32);
        check_float(make_float(13248074,   95), "52481028",  28);
    }

    SECTION("double precision")
    {
        auto check_double = [](double number, const std::string & digits, int expected_exponent)
        {
            CAPTURE(number)
            CAPTURE(digits)
            CAPTURE(expected_exponent)

            std::array<char, 32> buf{};
            int len = 0;
            int exponent = 0;
            nlohmann::detail::dtoa_impl::grisu2(buf.data(), len, exponent, number);

            CHECK(digits == std::string(buf.data(), buf.data() + len));
            CHECK(expected_exponent == exponent);
        };

        check_double(make_double(0,    0, 0x0000000000000001),                 "5", -324);
        check_double(make_double(0,    0, 0x000FFFFFFFFFFFFF),  "2225073858507201", -323);
        check_double(make_double(0,    1, 0x0000000000000000), "22250738585072014", -324);
        check_double(make_double(0,    1, 0x0000000000000001),  "2225073858507202", -323);
        check_double(make_double(0,    1, 0x000FFFFFFFFFFFFF), "44501477170144023", -324);
        check_double(make_double(0,    2, 0x0000000000000000),  "4450147717014403", -323);
        check_double(make_double(0,    2, 0x0000000000000001),  "4450147717014404", -323);
        check_double(make_double(0,    4, 0x0000000000000000), "17800590868057611", -323);
        check_double(make_double(0,    5, 0x0000000000000000), "35601181736115222", -323);
        check_double(make_double(0,    6, 0x0000000000000000),  "7120236347223045", -322);
        check_double(make_double(0,   10, 0x0000000000000000), "11392378155556871", -321);
        check_double(make_double(0, 2046, 0x000FFFFFFFFFFFFE), "17976931348623155",  292);
        check_double(make_double(0, 2046, 0x000FFFFFFFFFFFFF), "17976931348623157",  292);

        check_double(                  10000,                 "1",    4);
        check_double(                1200000,                "12",    5);
        check_double(4.9406564584124654e-324,                 "5", -324);
        check_double(2.2250738585072009e-308,  "2225073858507201", -323);
        check_double(   1.82877982605164e-99,   "182877982605164", -113);
        check_double( 1.1505466208671903e-09, "11505466208671903",  -25);
        check_double( 5.5645893133766722e+20,  "5564589313376672",    5);
        check_double(     53.034830388866226, "53034830388866226",  -15);
        check_double(  0.0021066531670178605, "21066531670178605",  -19);

        check_double(make_double(8511030020275656,  -342)  ,                "95",  -89);
        check_double(make_double(5201988407066741,  -824)  ,               "465", -235);
        check_double(make_double(6406892948269899,  +237)  ,              "1415",   84);
        check_double(make_double(8431154198732492,   +72)  ,             "39815",   33);
        check_double(make_double(6475049196144587,   +99)  ,            "410405",   40);
        check_double(make_double(8274307542972842,  +726)  ,           "2920845",  228);
        check_double(make_double(5381065484265332,  -456)  ,          "28919465", -129);
        check_double(make_double(6761728585499734, -1057)  ,         "437877185", -311);
        check_double(make_double(7976538478610756,  +376)  ,        "1227701635",  120);
        check_double(make_double(5982403858958067,  +377)  ,       "18415524525",  119);
        check_double(make_double(5536995190630837,   +93)  ,      "548357443505",   32);
        check_double(make_double(7225450889282194,  +710)  ,     "3891901811465",  217);
        check_double(make_double(7225450889282194,  +709)  ,    "19459509057325",  216);
        check_double(make_double(8703372741147379,  +117)  ,   "144609583816055",   37);
        check_double(make_double(8944262675275217, -1001)  ,  "4173677474585315", -301);
        check_double(make_double(7459803696087692,  -707)  , "11079507728788885", -213);
        check_double(make_double(6080469016670379,  -381)  ,  "1234550136632744", -114);
        check_double(make_double(8385515147034757,  +721)  ,   "925031711960365",  218);
        check_double(make_double(7514216811389786,  -828)  ,   "419804715028489", -248);
        check_double(make_double(8397297803260511,  -345)  , "11716315319786511", -104);
        check_double(make_double(6733459239310543,  +202)  ,  "4328100728446125",   61);
        check_double(make_double(8091450587292794,  -473)  ,  "3317710118160031", -142);

        check_double(make_double(6567258882077402,  +952)  ,                "25",  301);
        check_double(make_double(6712731423444934,  +535)  ,               "755",  174);
        check_double(make_double(6712731423444934,  +534)  ,              "3775",  173);
        check_double(make_double(5298405411573037,  -957)  ,             "43495", -277);
        check_double(make_double(5137311167659507,  -144)  ,            "230365",  -33);
        check_double(make_double(6722280709661868,  +363)  ,           "1263005",  119);
        check_double(make_double(5344436398034927,  -169)  ,          "71422105",  -43);
        check_double(make_double(8369123604277281,  -853)  ,         "139345735", -249);
        check_double(make_double(8995822108487663,  -780)  ,        "1414634485", -228);
        check_double(make_double(8942832835564782,  -383)  ,       "45392779195", -110);
        check_double(make_double(8942832835564782,  -384)  ,      "226963895975", -111);
        check_double(make_double(8942832835564782,  -385)  ,     "1134819479875", -112);
        check_double(make_double(6965949469487146,  -249)  ,    "77003665618895",  -73);
        check_double(make_double(6965949469487146,  -250)  ,   "385018328094475",  -74);
        check_double(make_double(6965949469487146,  -251)  ,  "1925091640472375",  -75);
        check_double(make_double(7487252720986826,  +548)  , "68985865317742005",  164);
        check_double(make_double(5592117679628511,  +164)  , "13076622631878654",   49);
        check_double(make_double(8887055249355788,  +665)  , "13605202075612124",  200);
        check_double(make_double(6994187472632449,  +690)  , "35928102174759597",  207);
        check_double(make_double(8797576579012143,  +588)  ,  "8912519771248455",  177);
        check_double(make_double(7363326733505337,  +272)  , "55876975736230114",   81);
        check_double(make_double(8549497411294502,  -448)  , "11762578307285404", -135);

        check_double(make_double(50883641005312716, -172)  ,  "8499999999999999",  -51);
        check_double(make_double(38162730753984537, -170)  ,               "255",  -37);
        check_double(make_double(50832789069151999, -101)  , "20049999999999997",  -30);
        check_double(make_double(51822367833714164, -109)  ,  "7984499999999999",  -32);
        check_double(make_double(66840152193508133, -172)  , "11165499999999999",  -51);
        check_double(make_double(55111239245584393, -138)  ,           "1581615",  -31);
        check_double(make_double(71704866733321482, -112)  ,          "13809855",  -24);
        check_double(make_double(67160949328233173, -142)  , "12046404499999999",  -42);
        check_double(make_double(53237141308040189, -152)  ,  "9325140544999999",  -45);
        check_double(make_double(62785329394975786, -112)  ,       "12092014595",  -27);
        check_double(make_double(48367680154689523,  -77)  ,      "320070458385",  -18);
        check_double(make_double(42552223180606797, -102)  ,  "8391946324354999",  -30);
        check_double(make_double(63626356173011241, -112)  ,    "12253990460585",  -30);
        check_double(make_double(43566388595783643,  -99)  ,   "687356414897605",  -28);
        check_double(make_double(54512669636675272, -159)  ,  "7459816430480385",  -47);
        check_double(make_double(52306490527514614, -167)  ,  "2796058839814255",  -49);
        check_double(make_double(52306490527514614, -168)  , "13980294199071276",  -50);
        check_double(make_double(41024721590449423,  -89)  ,  "6627901237305736",  -26);
        check_double(make_double(37664020415894738, -132)  ,  "6917788004396807",  -39);
        check_double(make_double(37549883692866294,  -93)  ,  "3791569310834971",  -27);
        check_double(make_double(69124110374399839, -104)  , "34080817676591365",  -31);
        check_double(make_double(69124110374399839, -105)  , "17040408838295683",  -31);

        check_double(make_double(49517601571415211,  -94)  ,                "25",  -13);
        check_double(make_double(49517601571415211,  -95)  ,               "125",  -14);
        check_double(make_double(54390733528642804, -133)  , "49949999999999996",  -40);
        check_double(make_double(71805402319113924, -157)  , "39304999999999998",  -47);
        check_double(make_double(40435277969631694, -179)  ,  "5277049999999999",  -53);
        check_double(make_double(57241991568619049, -165)  ,           "1223955",  -39);
        check_double(make_double(65224162876242886,  +58)  , "18799584999999998",   18);
        check_double(make_double(70173376848895368, -138)  ,         "201387715",  -33);
        check_double(make_double(37072848117383207,  -99)  ,  "5849064104999999",  -29);
        check_double(make_double(56845051585389697, -176)  ,       "59349003055",  -47);
        check_double(make_double(54791673366936431, -145)  , "12284718039499998",  -43);
        check_double(make_double(66800318669106231, -169)  ,  "8927076718084999",  -50);
        check_double(make_double(66800318669106231, -170)  , "44635383590424995",  -51);
        check_double(make_double(66574323440112438, -119)  , "10016990862549499",  -35);
        check_double(make_double(65645179969330963, -173)  ,  "5482941262802465",  -51);
        check_double(make_double(61847254334681076, -109)  ,  "9529078328103644",  -32);
        check_double(make_double(39990712921393606, -145)  ,  "8966227936640555",  -43);
        check_double(make_double(59292318184400283, -149)  ,  "8308623441805854",  -44);
        check_double(make_double(69116558615326153, -143)  , "61985873566126555",  -43);
        check_double(make_double(69116558615326153, -144)  , "30992936783063277",  -43);
        check_double(make_double(39462549494468513, -152)  ,  "6912351250617602",  -45);
        check_double(make_double(39462549494468513, -153)  ,  "3456175625308801",  -45);
    }
}

TEST_CASE("formatting")
{
    SECTION("single precision")
    {
        auto check_float = [](float number, const std::string & expected)
        {
            std::array<char, 33> buf{};
            char* end = nlohmann::detail::to_chars(buf.data(), buf.data() + 32, number);
            std::string actual(buf.data(), end);

            CHECK(actual == expected);
        };

        check_float( -1.2345e-22f, "-1.2345e-22"  );
        check_float( -1.2345e-21f, "-1.2345e-21"  );
        check_float( -1.2345e-20f, "-1.2345e-20"  );
        check_float( -1.2345e-19f, "-1.2345e-19"  );
        check_float( -1.2345e-18f, "-1.2345e-18"  );
        check_float( -1.2345e-17f, "-1.2345e-17"  );
        check_float( -1.2345e-16f, "-1.2345e-16"  );
        check_float( -1.2345e-15f, "-1.2345e-15"  );
        check_float( -1.2345e-14f, "-1.2345e-14"  );
        check_float( -1.2345e-13f, "-1.2345e-13"  );
        check_float( -1.2345e-12f, "-1.2345e-12"  );
        check_float( -1.2345e-11f, "-1.2345e-11"  );
        check_float( -1.2345e-10f, "-1.2345e-10"  );
        check_float( -1.2345e-9f,  "-1.2345e-09"  );
        check_float( -1.2345e-8f,  "-1.2345e-08"  );
        check_float( -1.2345e-7f,  "-1.2345e-07"  );
        check_float( -1.2345e-6f,  "-1.2345e-06"  );
        check_float( -1.2345e-5f,  "-1.2345e-05"  );
        check_float( -1.2345e-4f,  "-0.00012345"  );
        check_float( -1.2345e-3f,  "-0.0012345"   );
        check_float( -1.2345e-2f,  "-0.012345"    );
        check_float( -1.2345e-1f,  "-0.12345"     );
        check_float( -0.0f,        "-0.0"         );
        check_float(  0.0f,         "0.0"         );
        check_float(  1.2345e+0f,   "1.2345"      );
        check_float(  1.2345e+1f,   "12.345"      );
        check_float(  1.2345e+2f,   "123.45"      );
        check_float(  1.2345e+3f,   "1234.5"      );
        check_float(  1.2345e+4f,   "12345.0"     );
        check_float(  1.2345e+5f,   "123450.0"    );
        check_float(  1.2345e+6f,   "1.2345e+06"  );
        check_float(  1.2345e+7f,   "1.2345e+07"  );
        check_float(  1.2345e+8f,   "1.2345e+08"  );
        check_float(  1.2345e+9f,   "1.2345e+09"  );
        check_float(  1.2345e+10f,  "1.2345e+10"  );
        check_float(  1.2345e+11f,  "1.2345e+11"  );
        check_float(  1.2345e+12f,  "1.2345e+12"  );
        check_float(  1.2345e+13f,  "1.2345e+13"  );
        check_float(  1.2345e+14f,  "1.2345e+14"  );
        check_float(  1.2345e+15f,  "1.2345e+15"  );
        check_float(  1.2345e+16f,  "1.2345e+16"  );
        check_float(  1.2345e+17f,  "1.2345e+17"  );
        check_float(  1.2345e+18f,  "1.2345e+18"  );
        check_float(  1.2345e+19f,  "1.2345e+19"  );
        check_float(  1.2345e+20f,  "1.2345e+20"  );
        check_float(  1.2345e+21f,  "1.2345e+21"  );
        check_float(  1.2345e+22f,  "1.2345e+22"  );
    }

    SECTION("double precision")
    {
        auto check_double = [](double number, const std::string & expected)
        {
            std::array<char, 33> buf{};
            char* end = nlohmann::detail::to_chars(buf.data(), buf.data() + 32, number);
            std::string actual(buf.data(), end);

            CHECK(actual == expected);
        };

        check_double( -1.2345e-22,  "-1.2345e-22"             );
        check_double( -1.2345e-21,  "-1.2345e-21"             );
        check_double( -1.2345e-20,  "-1.2345e-20"             );
        check_double( -1.2345e-19,  "-1.2345e-19"             );
        check_double( -1.2345e-18,  "-1.2345e-18"             );
        check_double( -1.2345e-17,  "-1.2345e-17"             );
        check_double( -1.2345e-16,  "-1.2345e-16"             );
        check_double( -1.2345e-15,  "-1.2345e-15"             );
        check_double( -1.2345e-14,  "-1.2345e-14"             );
        check_double( -1.2345e-13,  "-1.2345e-13"             );
        check_double( -1.2345e-12,  "-1.2345e-12"             );
        check_double( -1.2345e-11,  "-1.2345e-11"             );
        check_double( -1.2345e-10,  "-1.2345e-10"             );
        check_double( -1.2345e-9,   "-1.2345e-09"             );
        check_double( -1.2345e-8,   "-1.2345e-08"             );
        check_double( -1.2345e-7,   "-1.2345e-07"             );
        check_double( -1.2345e-6,   "-1.2345e-06"             );
        check_double( -1.2345e-5,   "-1.2345e-05"             );
        check_double( -1.2345e-4,   "-0.00012345"             );
        check_double( -1.2345e-3,   "-0.0012345"              );
        check_double( -1.2345e-2,   "-0.012345"               );
        check_double( -1.2345e-1,   "-0.12345"                );
        check_double( -0.0,         "-0.0"                    );
        check_double(  0.0,          "0.0"                    );
        check_double(  1.2345e+0,    "1.2345"                 );
        check_double(  1.2345e+1,    "12.345"                 );
        check_double(  1.2345e+2,    "123.45"                 );
        check_double(  1.2345e+3,    "1234.5"                 );
        check_double(  1.2345e+4,    "12345.0"                );
        check_double(  1.2345e+5,    "123450.0"               );
        check_double(  1.2345e+6,    "1234500.0"              );
        check_double(  1.2345e+7,    "12345000.0"             );
        check_double(  1.2345e+8,    "123450000.0"            );
        check_double(  1.2345e+9,    "1234500000.0"           );
        check_double(  1.2345e+10,   "12345000000.0"          );
        check_double(  1.2345e+11,   "123450000000.0"         );
        check_double(  1.2345e+12,   "1234500000000.0"        );
        check_double(  1.2345e+13,   "12345000000000.0"       );
        check_double(  1.2345e+14,   "123450000000000.0"      );
        check_double(  1.2345e+15,   "1.2345e+15"             );
        check_double(  1.2345e+16,   "1.2345e+16"             );
        check_double(  1.2345e+17,   "1.2345e+17"             );
        check_double(  1.2345e+18,   "1.2345e+18"             );
        check_double(  1.2345e+19,   "1.2345e+19"             );
        check_double(  1.2345e+20,   "1.2345e+20"             );
        check_double(  1.2345e+21,   "1.2344999999999999e+21" );
        check_double(  1.2345e+22,   "1.2345e+22"             );
    }

    SECTION("integer")
    {
        auto check_integer = [](std::int64_t number, const std::string & expected)
        {
            const nlohmann::json j = number;
            CHECK(j.dump() == expected);
        };

        check_integer(INT64_MIN, "-9223372036854775808");
        check_integer(INT64_MAX, "9223372036854775807");

        check_integer(-3456789012345678901LL, "-3456789012345678901");
        check_integer(3456789012345678901LL, "3456789012345678901");
        check_integer(-5678901234567890123LL, "-5678901234567890123");
        check_integer(5678901234567890123LL, "5678901234567890123");

        check_integer(-1000000000000000000LL, "-1000000000000000000");
        check_integer(-100000000000000000LL, "-100000000000000000");
        check_integer(-10000000000000000LL, "-10000000000000000");
        check_integer(-1000000000000000LL, "-1000000000000000");
        check_integer(-100000000000000LL, "-100000000000000");
        check_integer(-10000000000000LL, "-10000000000000");
        check_integer(-1000000000000LL, "-1000000000000");
        check_integer(-100000000000LL, "-100000000000");
        check_integer(-10000000000LL, "-10000000000");
        check_integer(-1000000000LL, "-1000000000");
        check_integer(-100000000LL, "-100000000");
        check_integer(-10000000LL, "-10000000");
        check_integer(-1000000LL, "-1000000");
        check_integer(-100000LL, "-100000");
        check_integer(-10000LL, "-10000");
        check_integer(-1000LL, "-1000");
        check_integer(-100LL, "-100");
        check_integer(-10LL, "-10");
        check_integer(-1LL, "-1");
        check_integer(0, "0");
        check_integer(1LL, "1");
        check_integer(10LL, "10");
        check_integer(100LL, "100");
        check_integer(1000LL, "1000");
        check_integer(10000LL, "10000");
        check_integer(100000LL, "100000");
        check_integer(1000000LL, "1000000");
        check_integer(10000000LL, "10000000");
        check_integer(100000000LL, "100000000");
        check_integer(1000000000LL, "1000000000");
        check_integer(10000000000LL, "10000000000");
        check_integer(100000000000LL, "100000000000");
        check_integer(1000000000000LL, "1000000000000");
        check_integer(10000000000000LL, "10000000000000");
        check_integer(100000000000000LL, "100000000000000");
        check_integer(1000000000000000LL, "1000000000000000");
        check_integer(10000000000000000LL, "10000000000000000");
        check_integer(100000000000000000LL, "100000000000000000");
        check_integer(1000000000000000000LL, "1000000000000000000");
    }
}
