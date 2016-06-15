

#include "catch.hpp"
#include <stdint.h>
#include <type_traits>
#include <limits>
#include "xoroshiro.hpp"
#include <iostream>

namespace {

    XOROSHIRO_ALIGNMENT XoRoShiRo::state_t  state { 0, 1 } ;

    template <typename T_>
        typename std::enable_if<std::is_integral<T_>::value, T_>::type    random () {
            using uval_t = typename std::make_unsigned<T_>::type ;

            auto v = XoRoShiRo::next (state) ;

            static_assert (sizeof (T_) == 8 || sizeof (T_) == 4 || sizeof (T_) == 2 || sizeof (T_) == 1
                          , "Bad integral type supplied.") ;
            switch (sizeof (T_)) {
            case 8:
                return (T_)v ;
            case 4:
                return (T_)( static_cast<uval_t> (v >>  0)
                           ^ static_cast<uval_t> (v >> 32)) ;
            case 2:
                return (T_)( static_cast<uval_t> (v >>  0)
                           ^ static_cast<uval_t> (v >> 16)
                           ^ static_cast<uval_t> (v >> 32)
                           ^ static_cast<uval_t> (v >> 48)) ;
            case 1:
                return (T_)( static_cast<uval_t> (v >>  0)
                           ^ static_cast<uval_t> (v >>  8)
                           ^ static_cast<uval_t> (v >> 16)
                           ^ static_cast<uval_t> (v >> 24)
                           ^ static_cast<uval_t> (v >> 32)
                           ^ static_cast<uval_t> (v >> 40)
                           ^ static_cast<uval_t> (v >> 48)
                           ^ static_cast<uval_t> (v >> 56)) ;
            default:
                /*NOTREACHED*/
                return (T_)0 ;
            }
        }

    template <typename T_>
        typename std::enable_if<std::is_floating_point<T_>::value, T_>::type    random () ;

    template <>
        float random<float> () {
            union {
                float       fval ;
                uint32_t    ival ;
            } tmp ;
            auto vv = XoRoShiRo::next (state) ;
            auto v = (static_cast<uint32_t> (vv >> 0) ^ static_cast<uint32_t> (vv >> 32)) ;
            // Construct 1.0..<2.0 in IEEE-754 format.
            // (cf. http://marupeke296.com/TIPS_No16_flaotrandom.html )
            tmp.ival = (v >> 9) | 0x3F800000u ;
            return (tmp.fval - 1.0f) ;
        }

    template <>
        double random<double> () {
            union {
                double      dval ;
                uint64_t    ival ;
            } tmp ;
            uint64_t v = XoRoShiRo::next (state) ;
            // Construct 1.0..<2.0 in IEEE-754 format.
            // (cf. http://marupeke296.com/TIPS_No16_flaotrandom.html )
            tmp.ival = (v >> 12) | 0x3FF0000000000000ul ;
            return (tmp.dval - 1.0) ;
        }
}

#define TEST_PRNG(T_)   \
    SECTION ("Testing: " #T_) {                             \
        for (size_t i = 0 ; i < NUM_TRY ; ++i) {            \
            T_ v = random<T_> () ;                          \
            REQUIRE (std::numeric_limits<T_>::min () <= v) ;    \
            REQUIRE (v <= std::numeric_limits<T_>::max ()) ;    \
        }               \
    }


TEST_CASE ("Type annotated PRNG", "[typed-prng]") {
    size_t  NUM_TRY = 10000 ;
    TEST_PRNG (int8_t) ;
    TEST_PRNG (uint8_t) ;
    TEST_PRNG (int16_t) ;
    TEST_PRNG (uint16_t) ;
    TEST_PRNG (int32_t) ;
    TEST_PRNG (uint32_t) ;
    TEST_PRNG (int64_t) ;
    TEST_PRNG (uint64_t) ;

    SECTION ("Testing: float") {
        for (size_t i = 0 ; i < NUM_TRY ; ++i) {
            float v = random<float> () ;
            REQUIRE (0.0f <= v) ;
            REQUIRE (v < 1.0f) ;
        }
    }

    SECTION ("Testing: double") {
        for (size_t i = 0 ; i < NUM_TRY ; ++i) {
            double v = random<double> () ;
            CAPTURE (v) ;
            REQUIRE (0.0 <= v) ;
            REQUIRE (v < 1.0) ;
        }
    }
#if 0
    /* Should cause a compilation error  */
    SECTION ("Testing: long double") {
        for (size_t i = 0 ; i < NUM_TRY ; ++i) {
            long double v = random<long double> () ;
            REQUIRE (0.0 <= v) ;
            REQUIRE (v < 1.0) ;
        }
    }
#endif
}
