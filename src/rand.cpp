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
#include "rand.h"

bool Rand::Rand_state = false;

//---------------------------------------------------
// function: RandStream::RandStream(const int &seed)
//   default constructor
//---------------------------------------------------
RandStream::RandStream(const unsigned int& seed)
{
    set_seed(seed);
};

//---------------------------------------------------
// function: void RandStream::set_seed(int seed)
//   Set the seed of the random number generator
//   
//   if the seed is greater than zero, the seed will 
//     be used to set the state of the stream
//   otherwise the seed will be generated from time
//
//   Use a positive seed if you want the same 
//     result from every run
//   Use a negative seed if you want to check 
//     statistical effect of the simulation
//---------------------------------------------------
void RandStream::set_seed(const unsigned int& seed)
{
    // Set the seed.
    if (seed <= 0) {
        _fSeed = static_cast<unsigned int>(time(NULL) & 0x7fffffff);
    }
    else {
        _fSeed = seed;
    }

    _fSeed  = _fSeed | 0x00000001UL;

    _fSeed1 = (1103515245 * _fSeed  + 99991) & 0x7fffffffUL;
    _fSeed2 = (1103515245 * _fSeed1 + 99991) & 0x7fffffffUL;
    _fSeed3 = (1103515245 * _fSeed2 + 99991) & 0x7fffffffUL;

    if (_fSeed1 < 2)  _fSeed1 += 2UL;
    if (_fSeed2 < 8)  _fSeed2 += 8UL;
    if (_fSeed3 < 16) _fSeed3 += 16UL;

    for (int idx = 0; idx < 6; ++idx)
        Rand::rndm(*this);
}

//----------------------------------------------------
// function: double Rand::rndm(RandStream& stream)
//   Random number generator, this function returns
//   a random number in the range of [0, 1)
//
//   The period of the generator is 2**88 (about 10**26)
//
//   This function will update the state of parameter "stream"
//----------------------------------------------------
double Rand::rndm(RandStream& stream)
{
    const double kScale = 2.3283064365386963e-10; // range in 32 bit ( 1/(2**32)

    //do not change the following micro unless you know what you are doing
#define TAUSWORTHE(s,a,b,c,d) (((s &c) <<d) & 0xffffffffUL ) ^ ((((s <<a) & 0xffffffffUL )^s) >>b)

    stream._fSeed1 = TAUSWORTHE(stream._fSeed1, 13, 19, 4294967294UL, 12);
    stream._fSeed2 = TAUSWORTHE(stream._fSeed2, 2, 25, 4294967288UL, 4);
    stream._fSeed3 = TAUSWORTHE(stream._fSeed3, 3, 11, 4294967280UL, 17);

    unsigned int iy = stream._fSeed1 ^ stream._fSeed2 ^ stream._fSeed3;
    if (iy) return  kScale*static_cast<double>(iy);
    return rndm(stream);
}

//------------------------------------------------------
// function: void Rand::rndm_array(RandStream &stream, const int &n, double *arry)
//   Similar to the above function, this function generates 
//   a array of random numbers and saves them in "array"
//
//   The period of the generator is 2**88 (about 10**26)
//  
//   This function will update the state of parameter "stream"
//-------------------------------------------------------
void Rand::rndm_array(RandStream& stream, const unsigned int& n, double* arry)
{
    const double kScale = 2.3283064365386963e-10; // range in 32 bit ( 1/(2**32)

    unsigned int iy;

    for (unsigned int ii = 0; ii < n; ++ii) {
        stream._fSeed1 = TAUSWORTHE(stream._fSeed1, 13, 19, 4294967294UL, 12);
        stream._fSeed2 = TAUSWORTHE(stream._fSeed2, 2, 25, 4294967288UL, 4);
        stream._fSeed3 = TAUSWORTHE(stream._fSeed3, 3, 11, 4294967280UL, 17);

        iy = stream._fSeed1 ^ stream._fSeed2 ^ stream._fSeed3;
        if (iy) arry[ii] = kScale*static_cast<double>(iy);
        else --ii;
    }
}

//--------------------------------------------------------
// function: double Rand::gauss(RandStream &stream, const double &mean, const double &sigma)
//
// Generate a random number with Gaussian PDF with given mean and sigma 
//
//--------------------------------------------------------
double Rand::gauss(RandStream& stream, const double& mean, const double& sigma)
{
    //--------------------------------------------------------------------------
    //Returns a normal (Gaussian) distributed random number.
    //NOTE: use s>0.0
    //
    //Uses a very accurate approximation of the normal idf due to 
    //   Odeh & Evans, J. Applied Statistics, 1974, vol 23, pp 96-97.
    //const double Norm_P0 = 0.322232431088;
    //const double Norm_P1 = 1.0;
    //const double Norm_P2 = 0.342242088547;
    //const double Norm_P3 = 0.204231210245e-1;
    //const double Norm_P4 = 0.453642210148e-4;

    //const double Norm_Q0 = 0.099348462606;
    //const double Norm_Q1 = 0.588581570495;
    //const double Norm_Q2 = 0.531103462366;
    //const double Norm_Q3 = 0.103537752850;
    //const double Norm_Q4 = 0.385607006340e-2;
    //double u, t, p, q, z;
    //
    //u = rndm(stream);
    //
    //if (u<0.5){
    //   t = sqrt(-2.*log(u));
    //   p = Norm_P0 + t*(Norm_P1 + t*(Norm_P2 + t*(Norm_P3 + t*Norm_P4)));
    //   q = Norm_Q0 + t*(Norm_Q1 + t*(Norm_Q2 + t*(Norm_Q3 + t*Norm_Q4)));
    //   z = (p/q) - t;
    //}else{
    //   t = sqrt(-2.*log(1.-u));
    //   p = Norm_P0 + t*(Norm_P1 + t*(Norm_P2 + t*(Norm_P3 + t*Norm_P4)));
    //   q = Norm_Q0 + t*(Norm_Q1 + t*(Norm_Q2 + t*(Norm_Q3 + t*Norm_Q4)));
    //   z = t - (p/q);
    //}
    //return (mean + sigma*z);
    //
    //--------------------------------------------------------------------------
    // The following method is adopted from 
    //     https://root.cern.ch/root/html518/src/TRandom.cxx.html             
    // The function samples a random number from the standard Normal 
    // (Gaussian) distribution with the given mean and sigma.
    //                                                                             
    // REFERENCE:  - W. Hoermann and G. Derflinger (1990):                       
    //             The ACR Method for generating normal random variables,       
    //             OR Spektrum 12 (1990), 181-185. 
    //--------------------------------------------------------------------------

    const double kC1 = 1.448242853;
    const double kC2 = 3.307147487;
    const double kC3 = 1.46754004;
    const double kD1 = 1.036467755;
    const double kD2 = 5.295844968;
    const double kD3 = 3.631288474;
    const double kHm = 0.483941449;
    const double kZm = 0.107981933;
    const double kHp = 4.132731354;
    const double kZp = 18.52161694;
    const double kPhln = 0.4515827053;
    const double kHm1 = 0.516058551;
    const double kHp1 = 3.132731354;
    const double kHzm = 0.375959516;
    const double kHzmp = 0.591923442;
    /*zhm 0.967882898*/

    const double kAs = 0.8853395638;
    const double kBs = 0.2452635696;
    const double kCs = 0.2770276848;
    const double kB = 0.5029324303;
    const double kX0 = 0.4571828819;
    const double kYm = 0.187308492;
    const double kS = 0.7270572718;
    const double kT = 0.03895759111;

    double result;
    double rn, x, y, z;

    do {
        y = Rand::rndm(stream);

        if (y > kHm1) {
            result = kHp*y - kHp1;
            break;
        }
        else if (y < kZm) {
            rn = kZp*y - 1.;
            result = (rn > 0.) ? (1. + rn) : (-1. + rn);
            break;
        }
        else if (y < kHm) {
            rn = Rand::rndm(stream);
            rn = rn - 1. + rn;
            z = (rn > 0.) ? 2. - rn : -2. - rn;
            if ((kC1 - y)*(kC3 + fabs(z)) < kC2) {
                result = z;
                break;
            }
            else {
                x = rn*rn;
                if ((y + kD1)*(kD3 + x) < kD2) {
                    result = rn;
                    break;
                }
                else if (kHzmp - y < exp(-(z*z + kPhln) / 2.)) {
                    result = z;
                    break;
                }
                else if (y + kHzm < exp(-(x + kPhln) / 2.)) {
                    result = rn;
                    break;
                }
            }
        }

        while (1) {

            x = Rand::rndm(stream);
            y = kYm * Rand::rndm(stream);
            z = kX0 - kS*x - y;

            if (z > 0.) {
                rn = 2. + y / x;
            }
            else {
                x = 1. - x;
                y = kYm - y;
                rn = -(2. + y / x);
            }

            if ((y - kAs + x)*(kCs + x) + kBs < 0.) {
                result = rn;
                break;
            }
            else if (y < x + kT) {
                if (rn*rn < 4. * (kB - log(x))) {
                    result = rn;
                    break;
                }
            }
        }
    } while (0); //allow jump using "break"

    return mean + sigma*result;
}

//--------------------------------------------------------
// function: void Rand::gauss_array(RandStream &stream,const double &mean, 
//                            const double &sigma, const int &n, double *arry)
//   Similar to the above function, this function will generate 
//   a array of gaussian random number and saved in array
//
//   This function will update the state of parameter "stream"
//--------------------------------------------------------
void Rand::gauss_array(RandStream& stream, const double& mean, const double& sigma, const unsigned int& n, double* arry)
{
    for (unsigned int idx = 0; idx < n; ++idx) {
        arry[idx] = gauss(stream, mean, sigma);
    }
}

#ifdef _OPENMP 
//allocate more stream just in case hyper-threading programs 
std::vector<RandStream> gRStreamArry(omp_get_max_threads());
#else
RandStream gRStream;
#endif

//this function initialise the random number generator
//it automatically allocate randstream for each thread
void rand_init(const unsigned int &_fSeed, const unsigned int &max_threads)
{

#ifdef _OPENMP
    int num_threads = max_threads;

    if (num_threads == 0) {
        num_threads = omp_get_max_threads();
    }

    if (num_threads > static_cast<int>(gRStreamArry.size())) {
        //maximum thread number changed!
        gRStreamArry.resize(num_threads);
    }
    else {
        num_threads = gRStreamArry.size();
    }

    unsigned int seed = _fSeed;

    if (seed <= 0) {
        seed = static_cast<unsigned int>(time(NULL) & 0x7fffffff);
    }

    for (int num = 0; num < num_threads; ++num) {
        gRStreamArry[num].set_seed(seed);
        seed += 99991;
    }

#else

    gRStream.set_seed(_fSeed);

#endif /* end of #ifdef _OPENMP */

    Rand::Rand_state = true;
}

