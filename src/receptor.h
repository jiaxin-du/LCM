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

#ifndef RECEPTOR_H
#define RECEPTOR_H

//---------------------------------------------------
//
// Initialising the receptor requires calls to both 
// init(void) and precalc(step_size)
//
//---------------------------------------------------
#include "misc.h"

#ifndef RCPT_PARA_NUM
 //#define RCPT_PARA_NUM 	  6
#define RCPT_PARA_NUM          5
#define RCPT_IDX_G0            0
#define RCPT_IDX_LAMBDA        1
#define RCPT_IDX_DELAY         2
#define RCPT_IDX_TAU_RISE      3
#define RCPT_IDX_TAU_FALL      4
//#define RCPT_IDX_K      5
#endif

//---------------------------------------------
// PSP time course will terminate at the point 
// where the PSP is less PSP_EPS
//---------------------------------------------
#ifndef PSP_EPS
#define PSP_EPS                1e-3  /* in percentage */
//A very small PSP_EPS (< 1E-4) will slow down the 
//program significantly
#endif

//using namespace std;

//-------------------------------------------
// Class Receptor defines the receptor between
// neuron groups
//
//-------------------------------------------
class Receptor {

private:
    std::vector<TReal>   _rcpt_psp;
    //TReal                _t_step;

    TReal                _Jconst;
    std::vector<TReal>   _rcpt_paramVal;
    std::vector<bool>    _rcpt_paramFlg;

    std::string          _rcpt_name;
    TNeur                _rcpt_type;
    bool                 _rcpt_state;

    TInt                 _rcpt_idx;

    static TInt          RCPT_count;

public:

    Receptor(const std::string& xname = "UNNAMED_RECEPTOR");

    Receptor(const Receptor& p);

    ~Receptor(void) { --RCPT_count; };

    //Receptor& operator= (const Receptor &);

    bool set_param(const std::string& paramName, const TReal& val);

    bool set_type(const TNeur& type);

    bool is_ready(void) const;

    void swap(Receptor& b);

    inline std::string name(void) const { return _rcpt_name; };

    inline TInt   index(void) const { return _rcpt_idx; };

    inline TNeur  type(void) const { return _rcpt_type; };

    inline TReal  gain(void) const { return _rcpt_paramVal[RCPT_IDX_G0]; };

    inline TReal  lambda(void) const { return _rcpt_paramVal[RCPT_IDX_LAMBDA]; };

    inline TReal  delay(void) const { return _rcpt_paramVal[RCPT_IDX_DELAY]; };

    inline TReal  psp_rise(void) const { return _rcpt_paramVal[RCPT_IDX_TAU_RISE]; };

    inline TReal  psp_fall(void) const { return _rcpt_paramVal[RCPT_IDX_TAU_FALL]; };

    inline const  TReal* psp(void) const { return _rcpt_psp.data(); };

    inline TInt   psp_size(void) const { return _rcpt_psp.size(); };

    void  init(void);

    void  precalc(const TReal& step_size);

    TReal eqn_J(const TReal& phi) const;

    TReal eqn_R(const TReal& tau) const;

    std::string print(void) const;

    static TInt count() { return RCPT_count; };

    static const char  *RCPT_ParamName[];
    static const TReal  RCPT_ParamMin[];
    static const TReal  RCPT_ParamMax[];

};

//----------------------------------------------------
// function: inline void rcpt_swap(Receptor &a, Receptor &b)
//   swap the content of two Receptor objects
//
//----------------------------------------------------
inline void rcpt_swap(Receptor& a, Receptor& b)
{
    a.swap(b);
}

#endif /* end of #ifndef RECEPTOR_H */
