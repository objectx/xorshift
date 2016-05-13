/**
 * xoroshiro.hpp: Xor-Rotate-Shift-Rotate pseudo random number generator.
 *
 * Copyright (c) 2016 Masashi Fujita
 */
#pragma once
#ifndef xoroshiro_hpp__49459923_F87B_46C7_8993_77E9E9A88574
#define xoroshiro_hpp__49459923_F87B_46C7_8993_77E9E9A88574  1

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <array>

/**
 * Enables lock-free version of xoroshiro128 PRNG.
 *
 * Note: Disables `jump` feature.
 */
#define XOROSHIRO_LOCKFREE   1

#define XOROSHIRO_ALIGNMENT  alignas (16)

#if defined (_WIN32) || defined (_WIN64)
#   include <intrin.h>
#endif

namespace XoRoShiRo {
    using state_t = std::array<uint64_t, 2> ;

#if XOROSHIRO_LOCKFREE

#if defined (_WIN32) || defined (_WIN64)
    inline uint64_t next (state_t &state) {
        assert (false, "Not yet implemented") ;
        while (true) {
            XOROSHIRO_ALIGNMENT auto S = state ;
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
        // CMPXCHG16B requires destination was aligned to 16byte boundary.
        assert ((reinterpret_cast<uintptr_t> (state.data ()) & 0xF) == 0) ;

        uint64_t result ;

        __asm__ ("leaq  %1, %%rsi       \n"
                 "movq  (%%rsi), %%rax  \n"
                 "movq  8(%%rsi), %%rdx \n"
                 "xoroshiro_retry_A80A9D36_6F9F_47A5_81F0_AD2A6F86F6D5_%=:   \n"
                 "movq  %%rax, %%rbx    \n"
                 "movq  %%rdx, %%rcx    \n"
                 "xorq  %%rbx, %%rcx    \n"
                 "rorq     $9, %%rbx    \n"
                 "xorq  %%rcx, %%rbx    \n"
                 "movq  %%rcx, %%r8     \n"
                 "shlq    $14, %%r8     \n"
                 "xorq   %%r8, %%rbx    \n"
                 "rorq    $28, %%rcx    \n"
                 "lock; cmpxchg16b (%%rsi)  \n"
                 "jnz   xoroshiro_retry_A80A9D36_6F9F_47A5_81F0_AD2A6F86F6D5_%=  \n"
                 "leaq  (%%rax, %%rdx), %0  \n"
                : "=r" (result), "+m" (state [0]), "+m" (state [1])
                : /* empty */
                : "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%r8"
                ) ;
        return result;
    }

    inline state_t &    jump (state_t &state) {
        uint_fast64_t s0 ;
        uint_fast64_t s1 ;

        auto update = [&s0, &s1](state_t &S, uint64_t mask) {
            // Only considers locally copied state, thus no need to `atomic` ops.
            auto do_next = [] (state_t &state) -> uint64_t {
                const uint64_t s0 = state [0];
                uint64_t s1 = state [1];
                const uint64_t result = s0 + s1;

                auto rotl = [](uint64_t v, int cnt) -> uint64_t {
                    return (v << cnt) | (v >> (64 - cnt)) ;
                } ;

                s1 ^= s0;
                state [0] = rotl (s0, 55) ^ s1 ^ (s1 << 14) ;
                state [1] = rotl (s1, 36) ;

                return result;
            } ;

            for (int_fast32_t b = 0 ; b < 64 ; ++b) {
                auto v0 = S [0] ;
                auto v1 = S [1] ;
                if ((mask & (1ull << b)) != 0) {
                    s0 ^= v0 ;
                    s1 ^= v1 ;
                }
                do_next (S) ;
            }
        } ;

        uint8_t done = 0 ;
        do {
            auto saved_state XOROSHIRO_ALIGNMENT = state;
            auto tmp_state XOROSHIRO_ALIGNMENT = state;

            s0 = 0;
            s1 = 0;

            update (tmp_state, 0xBEAC0467EBA5FACBull) ;
            update (tmp_state, 0xD86B048B86AA9922ull) ;
            __asm__ ("leaq %1, %%rsi    \n"
                     "movq (%%rsi), %%rax   \n"
                     "movq 8(%%rsi), %%rdx  \n"
                     "movq %5, %%rbx    \n"
                     "movq %6, %%rcx    \n"
                     "lock; cmpxchg16b %0   \n"
                     "setz %2   \n"
                    : "+m" (state[0])
                    , "+m" (saved_state[0])
                    , "=q" (done)
                    , "+m" (state[1])
                    , "+m" (saved_state[1])
                    : "r" (s0), "r" (s1)
                    : "%rax", "%rbx", "%rcx", "%rdx", "%rsi");
        } while (! done) ;
        return state ;
    }

#endif  /* NOT (_WIN32 OR _WIN64) */

#else   /* ! XOROSHIRO_LOCKFRE */

    inline uint64_t next (state_t &state) {
        const uint64_t s0 = state [0];
        uint64_t s1 = state [1];
        const uint64_t result = s0 + s1;

        auto rotl = [](uint64_t v, int cnt) -> uint64_t {
            return (v << cnt) | (v >> (64 - cnt)) ;
        } ;

        s1 ^= s0;
        state [0] = rotl (s0, 55) ^ s1 ^ (s1 << 14) ;
        state [1] = rotl (s1, 36) ;

        return result;
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
        update (state, 0xBEAC0467EBA5FACBull) ;
        update (state, 0xD86B048B86AA9922ull) ;
        state [0] = s0 ;
        state [1] = s1 ;
        return state ;
    }
#endif  /* ! XOROSHIRO_LOCKFREE */

}

#endif /* xoroshiro_hpp__49459923_F87B_46C7_8993_77E9E9A88574 */
