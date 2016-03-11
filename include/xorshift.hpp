/**
 * xorshift.hpp: XorShift pseudo random number generator.
 *
 * Copyright (c) 2016 Polyphony Digital Inc.
 */
#pragma once
#ifndef xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f
#define xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f  1

#include <stddef.h>
#include <stdint.h>
#include <array>
#include <iterator>

/**
 * XorShift128+ RNG.
 */
class XorShift128 {
private:
    std::array<uint64_t, 2> v_ ;
public:
    XorShift128 (uint64_t v0, uint64_t v1) : v_ {v0, v1} {
        /* NO-OP */
    }

    XorShift128 () : XorShift128 (0, 0) {
        /* NO-OP */
    }

    XorShift128 (const XorShift128 &src) : v_ { src.v_ } {
        /* NO-OP */
    }

    /**
     * Generates a random value.
     */
    uint64_t next () {
        uint_fast64_t s1 = v_ [0] ;
        const uint_fast64_t s0 = v_ [1] ;
        v_ [0] = s0 ;
        s1 ^= s1 << 23 ;
        v_ [1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5) ;
        return v_ [1] + s0 ;
    }

    /**
     * Equivalent to call next () 2^64 times.
     */
    XorShift128 & jump () {
        static const std::array<uint64_t, 2> JUMP { 0x8a5cd789635d2dff, 0x121fd2155c472f96 } ;

        uint_fast64_t s0 = 0 ;
        uint_fast64_t s1 = 0 ;
        for (auto jmp : JUMP) {
            for (int_fast32_t b = 0 ; b < 64 ; ++b) {
                if ((jmp & (1ull << b)) != 0) {
                    s0 ^= v_ [0] ;
                    s1 ^= v_ [1] ;
                }
                next () ;
            }
        }
        v_ [0] = s0 ;
        v_ [1] = s1 ;
        return *this ;
    }
} ;

#endif /* end of include guard: xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f */
