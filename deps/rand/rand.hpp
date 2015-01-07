#pragma once

// uniform_real_distribution example
#include <iostream>
#include <chrono>
#include <random>

struct MTRand
{
    unsigned seed;
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution;

    MTRand() {
        seed = std::chrono::system_clock::now().time_since_epoch().count();
        generator = std::default_random_engine( seed );
        distribution = std::uniform_real_distribution<double>( 0.0, 1.0 );
    }

    double operator()() {
        // construct a trivial random generator engine from a time-based seed:
        return distribution(generator);
    }
};

static inline double rand01() { // [0..+1]
	static MTRand r;
	return r();
}

static inline double rand11() { // [-1..1]
	static MTRand r;
	return r() - r();
}

static inline int randsgn() { // {+1,-1}
	return rand01() >= 0.5 ? +1 : -1;
}

template<typename T>
static inline T rand2( T N, T M ) { // [N,M]
	double ret = N + rand01() * ( M + 1 - N );
	return T( ret >= M ? M : ret );
}

template<typename T>
static inline T rand1( T N ) { // [0,N]
	return rand2( 0, N );
}
