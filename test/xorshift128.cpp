
#include "catch.hpp"
#include <stdint.h>

#include "xorshift.hpp"

namespace {
    /*  Written in 2014-2015 by Sebastiano Vigna (vigna@acm.org)

    To the extent possible under law, the author has dedicated all copyright
    and related and neighboring rights to this software to the public domain
    worldwide. This software is distributed without any warranty.

    See <http://creativecommons.org/publicdomain/zero/1.0/>. */

    /* This is the fastest generator passing BigCrush without
       systematic failures, but due to the relatively short period it is
       acceptable only for applications with a mild amount of parallelism;
       otherwise, use a xorshift1024* generator.

       The state must be seeded so that it is not everywhere zero. If you have
       a 64-bit seed, we suggest to seed a splitmix64 generator and use its
       output to fill s. */

    uint64_t s[2];

    uint64_t next(void) {
    	uint64_t s1 = s[0];
    	const uint64_t s0 = s[1];
    	s[0] = s0;
    	s1 ^= s1 << 23; // a
    	s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5); // b, c
    	return s[1] + s0;
    }


    /* This is the jump function for the generator. It is equivalent
       to 2^64 calls to next(); it can be used to generate 2^64
       non-overlapping subsequences for parallel computations. */

    void jump() {
    	static const uint64_t JUMP[] = { 0x8a5cd789635d2dff, 0x121fd2155c472f96 };

    	uint64_t s0 = 0;
    	uint64_t s1 = 0;
    	for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
    		for(int b = 0; b < 64; b++) {
    			if (JUMP[i] & 1ULL << b) {
    				s0 ^= s[0];
    				s1 ^= s[1];
    			}
    			next();
    		}

    	s[0] = s0;
    	s[1] = s1;
    }
}

TEST_CASE ("Test lockfree-xorshift128", "[xorshift]") {
    SECTION ("Value should be equal to the reference implementation") {
        s [0] = 0 ;
        s [1] = 1 ;
        alignas (16) XorShift::state_t state { 0, 1 } ;

        for (int_fast32_t i = 0 ; i < 10000 ; ++i) {
            auto expected = next () ;
            auto actual = XorShift::next (state) ;
            CAPTURE (i) ;
            REQUIRE (expected == actual) ;
        }
    }

    SECTION ("Value should be equal after jump was called") {
        s [0] = 0 ;
        s [1] = 1 ;

        jump () ;
        alignas (16) XorShift::state_t state { 0, 1 } ;

        XorShift::jump (state) ;
        for (int_fast32_t i = 0 ; i < 10000 ; ++i) {
            auto expected = next () ;
            auto actual = XorShift::next (state) ;
            REQUIRE (expected == actual) ;
        }
    }
}

TEST_CASE ("Test lock agnositic xorshift128", "[xorshift]") {
    SECTION ("Value should be equal to the reference implementation") {
        s [0] = 0 ;
        s [1] = 1 ;
        alignas (16) XorShift::state_t state { 0, 1 } ;

        for (int_fast32_t i = 0 ; i < 10000 ; ++i) {
            auto expected = next () ;
            auto actual = XorShift::unsafe_next (state) ;
            CAPTURE (i) ;
            REQUIRE (expected == actual) ;
        }
    }

    SECTION ("Value should be equal after jump was called") {
        s [0] = 0 ;
        s [1] = 1 ;

        jump () ;
        alignas (16) XorShift::state_t state { 0, 1 } ;

        XorShift::unsafe_jump (state) ;
        for (int_fast32_t i = 0 ; i < 10000 ; ++i) {
            auto expected = next () ;
            auto actual = XorShift::unsafe_next (state) ;
            REQUIRE (expected == actual) ;
        }
    }
}

