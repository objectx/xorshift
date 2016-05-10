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

#define XORSHIFT_LOCKFREE   1

#define XORSHIFT_ALIGNMENT  alignas (16)

#if defined (_WIN32) || defined (_WIN64)
#   include <intrin.h>
#endif

/**
 * XorShift128+ RNG.
 */
class XORSHIFT_ALIGNMENT XorShift128 {
private:
    std::array<uint64_t, 2> v_ ;
public:
    XorShift128 (uint64_t v0, uint64_t v1) : v_ {v0, v1} {
        /* NO-OP */
    }

    XorShift128 () : XorShift128 (0, 0) {
        /* NO-OP */
    }

    XorShift128 (const XorShift128 &src) : v_ (src.v_) {
        /* NO-OP */
    }
#if XORSHIFT_LOCKFREE
#if defined (_WIN32) || defined (_WIN64)
    uint64_t next () {
        while (true) {
            XORSHIFT_ALIGNMENT auto S = v_ ;
            const uint_fast64_t ax = S [0] ;
            const uint_fast64_t dx = S [1] ;
            auto bx = dx ;
            auto cx = ax ;
            cx ^= (ax << 23) ;
            cx ^= (cx >> 18) ;
            cx ^= dx ;
            cx ^= (dx >> 5) ;
            if (_InterlockedCompareExchange128 ((volatile long long *)v_.data (), cx, bx, (long long *)S.data ()) != 0) {
                return bx + cx ;
            }
        }
    }
#else   /* NOT (_WIN32 OR _WIN64) */
    uint64_t next () __attribute__ ((noinline)){
        uint64_t result ;
        uint64_t *  s = v_.data () ;
        __asm__ ("movq (%1), %%rax  \n"
                 "movq 8(%1), %%rdx \n"
                 "1:"
                 "movq %%rdx, %%rbx \n"
                 "movq %%rax, %%rcx \n"
                 "movq %%rax, %%rsi \n"
                 "shlq $23, %%rsi   \n"
                 "xorq %%rsi, %%rcx \n"
                 "movq %%rcx, %%rsi \n"
                 "shrq $18, %%rsi   \n"
                 "xorq %%rsi, %%rcx \n"
                 "xorq %%rdx, %%rcx \n"
                 "movq %%rdx, %%rsi \n"
                 "shrq $5, %%rsi    \n"
                 "xorq %%rsi, %%rcx \n"
                 "lock; cmpxchg16b (%1)   \n"
                 "jnz 1b    \n"
                 "addq %%rcx, %%rbx \n"
                 "movq %%rbx, %0    \n"
                : "=r" (result)
                : "r" (s)
                : "%rax", "%rbx", "%rcx", "%rdx", "%rsi") ;
        return result ;
    }
#endif  /* NOT (_WIN32 OR _WIN64) */

#else   /* ! XORSHIFT_LOCKFREE */
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

#endif  /* ! XORSHIFT_LOCKFREE */
} ;

using xorshift128_state_t = std::array<uint64_t, 2> ;

#if XORSHIFT_LOCKFREE

#if defined (_WIN32) || defined (_WIN64)
inline uint64_t next (xorshift128_state_t &state) {
    while (true) {
        XORSHIFT_ALIGNMENT auto S = state ;
        const uint_fast64_t ax = S [0] ;
        const uint_fast64_t dx = S [1] ;
        auto bx = dx ;
        auto cx = ax ;
        cx ^= (ax << 23) ;
        cx ^= (cx >> 18) ;
        cx ^= dx ;
        cx ^= (dx >> 5) ;
        if (_InterlockedCompareExchange128 ((volatile long long *)state.data (), cx, bx, (long long *)S.data ()) != 0) {
            return bx + cx ;
        }
    }
}
#else   /* NOT (_WIN32 OR _WIN64) */
inline uint64_t next (xorshift128_state_t &state) {
    uint64_t result ;
    uint64_t *  s = state.data () ;
    __asm__ ("movq (%1), %%rax  \n"
             "movq 8(%1), %%rdx \n"
             "1:"
             "movq %%rdx, %%rbx \n"
             "movq %%rax, %%rcx \n"
             "movq %%rax, %%rsi \n"
             "shlq $23, %%rsi   \n"
             "xorq %%rsi, %%rcx \n"
             "movq %%rcx, %%rsi \n"
             "shrq $18, %%rsi   \n"
             "xorq %%rsi, %%rcx \n"
             "xorq %%rdx, %%rcx \n"
             "movq %%rdx, %%rsi \n"
             "shrq $5, %%rsi    \n"
             "xorq %%rsi, %%rcx \n"
             "lock; cmpxchg16b (%1)   \n"
             "jnz 1b    \n"
             "addq %%rcx, %%rbx \n"
             "movq %%rbx, %0    \n"
            : "=r" (result)
            : "r" (s)
            : "%rax", "%rbx", "%rcx", "%rdx", "%rsi") ;
    return result ;
}
#endif  /* NOT (_WIN32 OR _WIN64) */

#else   /* ! XORSHIFT_LOCKFRE */
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
#endif  /* XORSHIFT_LOCKFREE */

#endif /* end of include guard: xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f */
