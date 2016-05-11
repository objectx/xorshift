/**
 * xorshift.hpp: XorShift pseudo random number generator.
 *
 * Copyright (c) 2016 Masashi Fujita
 */
#pragma once
#ifndef xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f
#define xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f  1

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <array>

/**
 * Enables lock-free version of xorshift128 PRNG.
 *
 * Note: Disables `jump` feature.
 */
#define XORSHIFT_LOCKFREE   1

#define XORSHIFT_ALIGNMENT  alignas (16)

#if defined (_WIN32) || defined (_WIN64)
#   include <intrin.h>
#endif

namespace XorShift {
    using state_t = std::array<uint64_t, 2> ;

#if XORSHIFT_LOCKFREE

#if defined (_WIN32) || defined (_WIN64)
    inline uint64_t next (state_t &state) {
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

    inline uint64_t next (state_t &state) {
        uint64_t result;
        //uint64_t *s = state.data ();
        // CMPXCHG16B requires destination was aligned to 16byte boundary.
        assert ((reinterpret_cast<uintptr_t> (state.data ()) & 0xF) == 0) ;
        __asm__ ("leaq %1, %%rsi\n"
                 "movq (%%rsi), %%rax  \n"
                 "movq 8(%%rsi), %%rdx \n"
                 "xorshift_retry_EFF4AF5C_2F3E_4D8F_9DAE_9D1CFD6444B9_%=:   \n"
                 "movq %%rdx, %%rbx \n"
                 "movq %%rax, %%rcx \n"
                 "movq %%rax, %%rsi \n"
                 "shlq   $23, %%rsi \n"
                 "xorq %%rsi, %%rcx \n"
                 "movq %%rcx, %%rsi \n"
                 "shrq   $18, %%rsi \n"
                 "xorq %%rsi, %%rcx \n"
                 "xorq %%rdx, %%rcx \n"
                 "movq %%rdx, %%rsi \n"
                 "shrq    $5, %%rsi \n"
                 "xorq %%rsi, %%rcx \n"
                 "lock; cmpxchg16b %1   \n"
                 "jnz xorshift_retry_EFF4AF5C_2F3E_4D8F_9DAE_9D1CFD6444B9_%= \n"
                 "leaq (%%rcx, %%rbx), %0\n"
                : "=r" (result), "+m" (state [0]), "+m" (state [1])
                : /* empty */
                : "%rax", "%rbx", "%rcx", "%rdx", "%rsi");
        return result;
    }

#endif  /* NOT (_WIN32 OR _WIN64) */

#else   /* ! XORSHIFT_LOCKFRE */

    inline uint64_t next (state_t &state) {
        uint_fast64_t s1 = state [0] ;
        const uint_fast64_t s0 = state [1] ;
        uint64_t v0 = s0 ;
        s1 ^= s1 << 23 ;
        uint64_t v1 = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5) ;
        state [0] = v0 ;
        state [1] = v1 ;
        return v1 + s0 ;
    }

    inline state_t &    jump (state_t &state) {
        uint_fast64_t s0 = 0 ;
        uint_fast64_t s1 = 0 ;

        auto update = [&s0, &s1](state_t &S, uint64_t mask) {
            for (int_fast32_t b = 0 ; b < 64 ; ++b) {
                auto v0 = S [0] ;
                auto v1 = S [1] ;
                if ((mask & (1ull << b)) != 0) {
                    s0 ^= v0 ;
                    s1 ^= v1 ;
                }
                next (S) ;
            }
        } ;
        update (state, 0x8a5cd789635d2dffull) ;
        update (state, 0x121fd2155c472f96ull) ;
        state [0] = s0 ;
        state [1] = s1 ;
        return state ;
    }
#endif  /* ! XORSHIFT_LOCKFREE */

}

#endif /* end of include guard: xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f */
