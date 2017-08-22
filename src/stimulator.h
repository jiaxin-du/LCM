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

#ifndef STIMULATOR_H
#define STIMULATOR_H

//--------------------------------------------------
//  The class defined a extrnal stimulator class.
//  A external stimulator is a spike source projected 
//  from external source to the model.
//
//  Currently, only three stimulator modes are implemented
//    
// 1. mode=0: a low-frequency unsynchronised white noise
//    This type of stimulator generates white-noise shape spike rates,
//    and the spike rates projecting to columns are independent, i.e 
//    each column has its own stimulator with the same parameter values
//    but different random number sequences.
//
//    paramters:
//      amplitude: stimulation amplitude, see below (spike/sec)
//      period: low-pass filter window size (msec)
//        The spike rates are filtered using a low-pass butterworth filter
//        the low-pass cutoff frequency is approximately inverse of the period. 
//        If the generated value is negative, it is reset to zero. 
//
// 2. mode=1: a recurent Gaussian peaks
//    This type of stimulator generates spike rates with recurrent
//    Gaussian shape. The spike rates projecting to all elements are 
//    synchronized, i.e, they are the same at a time point.
//    
//    parameter
//      amplitude: amplitude of spike rates (spike/sec)
//      period: period of the spike rates (msec)
//      
// 3. mode=2: a low-frequency synchronised white noise
//    Similar to the mode 0, but the spike rates projecting to all elements 
//    are synchronised.
//
//  NB: a synchronised stimulator generate the same spike rates for all columns 
//      at a time, while a unsynchronised stimulator genrate spike rates 
//      indepdently for each column using the same parameter values but 
//      different random number sequences. 
// 
//  other parameters:
//    st_stop and st_stop: the time points when the stimulator starts and stops
//    source: name of external source that the stimulator is attached to
//    elements: elements that the stimulator projects to
//--------------------------------------------------

#include "misc.h"
#include "rand.h"
#include "array.h"

#ifdef _OPENMP
#include "omp.h"
#endif

#ifndef   ST_PARA_NUM

#define   ST_IDX_AMPL     0
#define   ST_IDX_PERIOD   1
#define   ST_IDX_SOURCE   2
#define   ST_IDX_MODE     3
#define   ST_IDX_INTVL    4
#define   ST_IDX_START    5
#define   ST_IDX_STOP     6

#define   ST_PARA_NUM     7
#endif

// set butterworth filter order, 
// BUTTER_COEFF_NUM = BUTTER_ORDER + 1
#define   BUTTER_COEFF_NUM    4 

#ifndef ST_MODE
#define ST_MODE
enum StimMode {
    ST_NONE = -1,
    ST_NOISE = 0,
    ST_GAUSS = 1,
    ST_SYNC_NOISE = 2
};
#endif

class Stimulator {
    //give ExSource objects access to data member of this class
    friend class ExSource;

private:
    //data member

    DynamicArray         _st_phi_in;
    DynamicArray         _st_phi_out;

    std::vector<TReal>   _st_coeff_in;
    std::vector<TReal>   _st_coeff_out;

    std::vector<RandStream> _st_rand;

    TReal    _st_ampl;  // amplitude 

    //data index
    TInt     _st_pos;

    StimMode _st_mode; //stimulator mode

    std::vector<TInt>   _st_elmts; //elements this stimulator projecting to 

    TInt     _st_spksrc_id;
    TInt     _st_period_win;
    TInt     _st_start, _st_stop;

    TInt     _st_update_intvl;

    bool     _st_state, _st_active;

    std::vector<bool>   _st_paramFlg;

    std::string _st_name; // name of the stimulator

    //RandStream  st_stream;

    //TInt st_nthrd ;
    static TInt  ST_COUNT;

public:
    Stimulator(const std::string& xname = "UNNAMED_STIMULATOR", const TInt& md = ST_NOISE);
    Stimulator(const Stimulator& p);

    ~Stimulator();

    //Stimulator& operator= (const Stimulator &);

    bool set_param(const std::string& paramName, const TReal& val);
    bool set_source(const TInt& src);
    void add_elmt(const TInt& ielmt);
    void set_mode(const TInt& md);
    void swap(Stimulator& a);

    void init(void);
    bool is_ready(void) const;

    void advance();

    TReal generate(const TInt&);

    inline std::string name(void) const { return  _st_name; };
    inline TReal amplitude(void) const { return  _st_ampl; };
    inline TInt  source(void) const { return  _st_spksrc_id; };
    inline TInt  start_step(void) const { return  _st_start; };
    inline TInt  stop_step(void) const { return  _st_stop; };
    inline TInt  mode(void) const { return  _st_mode; };

    //when deactivated, the stimulators produce nothing
    inline void  activate(void) {
        _st_active = true;
        init(); //restart the stimulator 
    };

    inline void  deactivate(void) { _st_active = false; };
    inline bool  is_active(void) const { return _st_active; };

    //std::vector<TInt> & elmt_list(void)  { return _st_elmts;    };
    const std::vector<TInt>& elmt_list(void) const { return _st_elmts; }

    std::string print(const std::string& srcname = "", const TReal& step_size = 0) const;

    static const char   *ST_ParaName[];

    static TInt count(void) { return ST_COUNT; };

};

// data structure
// _st_phi_in and _st_phi_out
//             elmt 1              |             elmt 2
// x[n] | x[n-1] | x[n-2] | x[n-3] | x[n] | x[n-1] | x[n-2] | x[n-3]
//
// after step_forward 
//  *        elmt 1                |  *       elmt 2
// new  |  x[n]  | x[n-1] | x[n-2] | new  |  x[n]  | x[n-1] | x[n-2]  
//
// filtering
// in[0]*b[0] + in[1]*b[1] + in[2]*b[2] +in[3]*b[3]
//            - out[1]*a[1] - out[2]*a[2] - out[3]*a[3]
//
//

inline void st_swap(Stimulator& a, Stimulator& b)
{
    a.swap(b);
}

#endif /* end of #ifndef STIMULATOR_H */
