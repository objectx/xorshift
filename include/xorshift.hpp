/**
 * xorshift.hpp: XorShift pseudo random number generator.
 *
 * Copyright (c) 2016-2018 Masashi Fujita
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
 */
#ifndef XORSHIFT_LOCKFREE
#   if __x86_64__
#       if 3 < __clang_major__
#           define XORSHIFT_LOCKFREE   1
#       else    /* __clang_major__ <=3 */
#           if 6 < __clang_minor__
#               define XORSHIFT_LOCKFREE   1
#           endif
#       endif   /* __clang_major__ <=3 */
#   elif defined (_WIN64)
#       define XORSHIFT_LOCKFREE   1
#   endif
#endif

#ifndef XORSHIFT_LOCKFREE
#   define XORSHIFT_LOCKFREE   0   /* Disables lock-free versions.  */
#endif

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

    inline state_t &    jump (state_t &state) {
        uint_fast64_t s0 ;
        uint_fast64_t s1 ;

        auto update = [&s0, &s1](state_t &S, uint64_t mask) {
            // Update only locally copied state, thus no atomic ops. neeed.
            auto do_next = [](state_t &state) {
                uint_fast64_t s1 = state [0] ;
                const uint_fast64_t s0 = state [1] ;
                uint64_t v0 = s0 ;
                s1 ^= s1 << 23 ;
                uint64_t v1 = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5) ;
                state [0] = v0 ;
                state [1] = v1 ;
                return v1 + s0 ;
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

        while (true) {
            auto saved_state XORSHIFT_ALIGNMENT = state;
            auto tmp_state XORSHIFT_ALIGNMENT = state;

            s0 = 0;
            s1 = 0;

            update (tmp_state, 0x8a5cd789635d2dffull);
            update (tmp_state, 0x121fd2155c472f96ull);

            if (_InterlockedCompareExchange128 ((volatile long long *)state.data (), s1, s0, (long long *)saved_state.data ()) != 0) {
                break ;
            }
        }
        return state ;
    }

#else   /* NOT (_WIN32 OR _WIN64) */

    inline uint64_t next (state_t &state) {
        uint64_t result;
        //uint64_t *s = state.data ();
        // CMPXCHG16B requires destination was aligned to 16byte boundary.
        assert ((reinterpret_cast<uintptr_t> (state.data ()) & 0xF) == 0) ;
        __asm__ __volatile__ ("leaq %1, %%rsi\n"
                              "movq (%%rsi), %%rax  \n"
                              "movq 8(%%rsi), %%rdx \n"
                              "xorshift_retry_EFF4AF5C_2F3E_4D8F_9DAE_9D1CFD6444B9_%=:   \n"
                              "movq %%rdx, %%rbx \n"
                              "movq %%rax, %%rcx \n"
                              "movq %%rax, %%r8  \n"
                              "shlq   $23, %%r8  \n"
                              "xorq  %%r8, %%rcx \n"
                              "movq %%rcx, %%r8  \n"
                              "shrq   $18, %%r8  \n"
                              "xorq  %%r8, %%rcx \n"
                              "xorq %%rdx, %%rcx \n"
                              "movq %%rdx, %%r8  \n"
                              "shrq    $5, %%r8  \n"
                              "xorq  %%r8, %%rcx \n"
                              "lock; cmpxchg16b (%%rsi)  \n"
                              "jnz xorshift_retry_EFF4AF5C_2F3E_4D8F_9DAE_9D1CFD6444B9_%= \n"
                              "leaq (%%rcx, %%rbx), %0\n"
                             : "=r" (result), "+m" (state [0]), "+m" (state [1])
                             : /* empty */
                             : "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%r8");
        return result;
    }

    inline state_t &    jump (state_t &state) {
        uint_fast64_t s0 ;
        uint_fast64_t s1 ;

        auto update = [&s0, &s1](state_t &S, uint64_t mask) {
            // Update only locally copied state, thus no atomic ops. neeed.
            auto do_next = [] (state_t &state) {
                uint_fast64_t s1 = state [0] ;
                const uint_fast64_t s0 = state [1] ;
                uint64_t v0 = s0 ;
                s1 ^= s1 << 23 ;
                uint64_t v1 = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5) ;
                state [0] = v0 ;
                state [1] = v1 ;
                return v1 + s0 ;
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
            auto saved_state XORSHIFT_ALIGNMENT = state;
            auto tmp_state XORSHIFT_ALIGNMENT = state;

            s0 = 0;
            s1 = 0;

            update (tmp_state, 0x8a5cd789635d2dffull);
            update (tmp_state, 0x121fd2155c472f96ull);
            __asm__ __volatile__ ("leaq  %0, %%r8    \n"
                                  "leaq  %1, %%rsi   \n"
                                  "movq   (%%rsi), %%rax \n"
                                  "movq  8(%%rsi), %%rdx \n"
                                  "movq  %5, %%rbx       \n"
                                  "movq  %6, %%rcx       \n"
                                  "lock; cmpxchg16b (%%r8)   \n"
                                  "setz  %2  \n"
                                 : "+m" (state[0])
                                 , "+m" (saved_state[0])
                                 , "=q" (done)
                                 , "+m" (state[1])
                                 , "+m" (saved_state[1])
                                 : "r" (s0), "r" (s1)
                                 : "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%r8");
        } while (! done) ;
        return state ;
    }

#endif  /* NOT (_WIN32 OR _WIN64) */

#endif  /* ! XORSHIFT_LOCKFREE */

    /// Thread agnostic version of `XorShift::next`.
    inline uint64_t unsafe_next (state_t &state) {
        uint_fast64_t s1 = state [0] ;
        const uint_fast64_t s0 = state [1] ;
        uint64_t v0 = s0 ;
        s1 ^= s1 << 23 ;
        uint64_t v1 = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5) ;
        state [0] = v0 ;
        state [1] = v1 ;
        return v1 + s0 ;
    }

    /// Thread agnositic version of `XorShift::jump`.
    inline state_t &    unsafe_jump (state_t &state) {
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
                unsafe_next (S) ;
            }
        } ;
        update (state, 0x8a5cd789635d2dffull) ;
        update (state, 0x121fd2155c472f96ull) ;
        state [0] = s0 ;
        state [1] = s1 ;
        return state ;
    }
}

#endif /* end of include guard: xorshift_hpp__b71b3a16_63c6_402e_881e_d6327a69180f */
