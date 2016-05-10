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
        const uint_fast64_t ax = v_ [0] ;
        const uint_fast64_t dx = v_ [1] ;
        auto bx = dx ;
        auto cx = ax ;
        cx ^= (ax << 23) ;
        cx ^= (cx >> 18) ;
        cx ^= dx ;
        cx ^= (dx >> 5) ;
        v_ [0] = bx ;
        v_ [1] = cx ;
        return bx + cx ;
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

using xorshift128_state_t = std::array<uint64_t, 2> ;

inline uint64_t next (xorshift128_state_t &state) {
    uint_fast64_t s1 = state [0] ;
    const uint_fast64_t s0 = state [1] ;
    uint64_t v0 = s0 ;
    s1 ^= s1 << 23 ;
    uint64_t v1 = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5) ;
    state [0] = v0 ;
    state [1] = v1 ;
    return v1 + s0 ;
}

inline xorshift128_state_t &    jump (xorshift128_state_t &state) {
    static const std::array<uint64_t, 2> JUMP { 0x8a5cd789635d2dff, 0x121fd2155c472f96 } ;

    uint_fast64_t s0 = 0 ;
    uint_fast64_t s1 = 0 ;

    for (auto jmp : JUMP) {
        for (int_fast32_t b = 0 ; b < 64 ; ++b) {
            uint64_t v0 = state [0] ;
            uint64_t v1 = state [1] ;
            if ((jmp & (1ull << b)) != 0) {
                s0 ^= v0 ;
                s1 ^= v1 ;
            }
            next (state) ;
        }
    }
    state [0] = s0 ;
    state [1] = s1 ;
    return state ;
}

#endif /* end of include guard: xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f */
