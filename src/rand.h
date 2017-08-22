//-------------------------------------------------
//
//          Laminar cortex model
//
// Developed by Jiaxin Du under the supervision of
//    Prof. David Reutens and Dr. Viktor Vegh
//
//       Centre for Advanced Imaging (CAI),
//   The University of Queensland (UQ), Australia
//
//        jiaxin.du@uqconnect.edu.au
//
// Reference:
//  Du J, Vegh V, & Reutens DC,
//                PLOS Compt Biol 8(10): e1002733.
//              & NeuroImage 94: 1-11.
//
// See README for software copyright statements.
//-------------------------------------------------
#pragma once

#ifndef RAND_H
#define RAND_H
//---------------------------------------------------
// RandStream is a class for saving the state of 
// random number generator, and Rand is a static class 
// for generating random numbers
//
// The random number generation method was adopted from 
// https://root.cern.ch/root/html/TRandom2.html
//
// The period of the generator is 2**88 (about 10**26) and 
// it uses only 3 words for the state.
//
// For more information see:
//     P. L'Ecuyer, Mathematics of Computation, 65, 213 (1996)
//     P. L'Ecuyer, Mathematics of Computation, 68, 225 (1999)
//
// Example:
//   RandStream stream(SEED);
// where SEED is the seed for the generator (unsigned integer), and
//   num = Rand::rndm(stream);
// would generate a random number evenly distributed between [0, 1)
//   num = Rand::gauss(stream, mean, std);
// would generate a random number with a gaussian PDF
//---------------------------------------------------

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#ifdef __INTEL_COMPILER
#include "mathimf.h"
#else
#include <cmath>
#endif

#ifdef _OPENMP
#include "omp.h"
#endif

//---------------------------------------------------
// Class RandSteam defines the state of 
//   the random number generator
//
//---------------------------------------------------
class RandStream
{
private:
    unsigned int _fSeed;
    unsigned int _fSeed1;
    unsigned int _fSeed2;
    unsigned int _fSeed3;

public:
    RandStream(const unsigned int& seed = 0);

    //the class contains no pointer member, member-wise copy is sufficient
    RandStream(const RandStream& p) {
        *this = p;
    };

    RandStream& operator= (const RandStream& p) {
        if (this != &p) {
            _fSeed = p._fSeed;
            _fSeed1 = p._fSeed1;
            _fSeed2 = p._fSeed2;
            _fSeed3 = p._fSeed3;
        }
        return *this;
    };

    ~RandStream() { };

    void set_seed(const unsigned int& seed);

    unsigned int get_seed(const int &idx = 0) const {
        switch (idx)
        {
        case 0:
            return _fSeed;
            break;
        case 1:
            return _fSeed1;
            break;
        case 2:
            return _fSeed2;
            break;
        default:
            return _fSeed3;
            break;
        }
    };

    friend class Rand;

};

//---------------------------------------------------
// Class Rand is static random number generator
//
//---------------------------------------------------
class Rand {
    friend void rand_init(const unsigned int&, const unsigned int&);

private:
    static bool Rand_state;

public:
    static double rndm(RandStream& stream);
    static double rndm(RandStream* stream) {
        return rndm(*stream);
    };

    static double rndm_int(RandStream& stream){
        return static_cast<int>(rndm(stream) * 2147483647.); // == rndm * power(2, 31)
    };
    static double rndm_int(RandStream* stream) {
        return rndm_int(*stream);
    };

    static void rndm_array(RandStream& stream, const unsigned int& n, double* arry);
    static void rndm_array(RandStream* stream, const unsigned int& n, double* arry) {
        rndm_array(*stream, n, arry);
    };

    static double gauss(RandStream& stream, const double& mean, const double& sigma);
    static double gauss(RandStream* stream, const double& mean, const double& sigma) {
        return gauss(*stream, mean, sigma);
    };

    static void gauss_array(RandStream& stream, const double& mean, const double& sigma, const unsigned int& n, double* arry);
    static void gauss_array(RandStream* stream, const double& mean, const double& sigma, const unsigned int& n, double* arry) {
        gauss_array(*stream, mean, sigma, n, arry);
    };

    static bool is_ready() { return Rand_state; };
};

void rand_init(const unsigned int& _fseed, const unsigned int &max_thread = 0);

#ifdef _OPENMP

extern std::vector<RandStream> gRStreamArry;

inline unsigned int rand_seed(const int &thread = 0)
{
    if (thread < omp_get_thread_num()) {
        return gRStreamArry[thread].get_seed();
    }
    else {
        return gRStreamArry[0].get_seed();
    }
};

inline double rand_rndm(void)
{
    return Rand::rndm(gRStreamArry[(omp_get_thread_num())]);
};

inline double rand_gauss(const double &mean, const double &devn)
{
    return Rand::gauss(gRStreamArry[(omp_get_thread_num())], mean, devn);
};

#else

extern RandStream gRStream;

inline unsigned int rand_seed(void)
{
    return gRStream.get_seed();
};

inline double rand_rndm(void)
{
    return Rand::rndm(gRStream);
};

inline double rand_gauss(const double &mean, const double &devn)
{
    return Rand::gauss(gRStream, mean, devn);
};
#endif /* end of #ifdef _OPENMP*/

#endif /* end of #ifndef RAND_H */
