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

#ifndef NEURGRP_H
#define NEURGRP_H

#include "misc.h"
#include "rand.h"
#include "layer.h"
#include "spikesrc.h"

#ifndef NG_PARA_NUM
#define NG_PARA_NUM        	11
#define NG_IDX_NEUR_DENSITY    	0
#define NG_IDX_PSP_SPD      	1
#define NG_IDX_SPK_SPD      	2
#define NG_IDX_PSP_DECAY   	3
#define NG_IDX_SYNP_SIGMA  	4
#define NG_IDX_V_REV       	5
#define NG_IDX_V_0       	6
#define NG_IDX_FIRE_GAIN    	7
#define NG_IDX_FIRE_VHMF    	8
#define NG_IDX_FIRE_MAX    	9
#define NG_IDX_TAU_MBN          10
#endif

#ifndef SPK_PATH_NUM
#define SPK_PATH_NUM        	4
#endif

#ifndef SYNP_RATIO_EPS
#define SYNP_RATIO_EPS      	1e-4 /* in percentage */
#endif

#ifndef UNNAMED_NEURON_GROUP
#define UNNAMED_NEURON_GROUP 	("UNNAMED_NEURON_GROUP")
#endif

class NeurGrp : public SpikeSrc
{
protected:
    std::vector<TReal>   _ng_paramVal;
    TReal   _ng_mp_decay_step;
    TInt    _ng_layer;
    TNeur   _ng_type;

    std::vector<bool>    _ng_paramFlg;
    bool    _ng_state;

    static TInt NG_cnt; // this is a static number for how many ng in the program

public:
    //default constructor
    NeurGrp(const std::string& xname = UNNAMED_NEURON_GROUP, const TInt& idx = SpikeSrc::SS_cnt);

    //copy constructor
    NeurGrp(const NeurGrp& p);

    //default destructor
    ~NeurGrp(void);

    //use default copy operator
    //operator
    //NeurGrp& operator= (const NeurGrp &);

    //set parameter value, use parameter name
    bool set_param(const std::string&, const TReal&);

    //set parameter value, use parameter idx
    bool set_param(const TInt&, const TReal&);

    //set neuron type
    bool set_type(const TNeur& xtype);

    //set the laminar location of the neuron group
    bool set_layer(const TInt& xlayer);

    //swap the content with another object
    void swap(NeurGrp& b);

    //check whether the object is ready or not
    bool is_ready(void) const;

    //return neuron type
    inline TNeur type(void) const { return _ng_type; };

    //return layer of the neuron group
    inline TInt  layer(void) const { return _ng_layer; };

    //return neuron density, neuron density is not used in this program
    //it is primarily used for calculate LFPs
    inline TReal density(void) const { return _ng_paramVal[NG_IDX_NEUR_DENSITY]; };

    //return propagation speed of PSP
    inline TReal psp_speed(void) const { return _ng_paramVal[NG_IDX_PSP_SPD]; };

    //return propagation speed of APs
    inline TReal spk_speed(void) const { return _ng_paramVal[NG_IDX_SPK_SPD]; };

    //return psp decay factor, unit: 1/m
    inline TReal psp_decay_factor(void) const { return _ng_paramVal[NG_IDX_PSP_DECAY]; };

    //return sigma of Gaussian shape synapse distribute function
    inline TReal synp_dist_sigma(void) const { return _ng_paramVal[NG_IDX_SYNP_SIGMA]; };

    //return reverse membrane potential
    inline TReal V_rev(void) const { return _ng_paramVal[NG_IDX_V_REV]; };

    //return resting membrane potential
    inline TReal V_0(void) const { return _ng_paramVal[NG_IDX_V_0]; };

    //return gain of neuronal firing function
    inline TReal fire_gain(void) const { return _ng_paramVal[NG_IDX_FIRE_GAIN]; };

    //return power of neuronal firing function
    inline TReal fire_VHMF(void) const { return _ng_paramVal[NG_IDX_FIRE_VHMF]; };

    //return the maximum firing rate
    inline TReal fire_max(void) const { return _ng_paramVal[NG_IDX_FIRE_MAX]; };

    //return membrane time constant
    inline TReal tau_mbn(void) const { return _ng_paramVal[NG_IDX_TAU_MBN]; };

    //returnt the membrane potential decay after a period
    inline TReal mp_decay(const TReal &tau) { return exp(-1 * tau / tau_mbn()); };

    //return membrane potential constant decay in a step, 
    //which is equal to exp(-1 * time_step /tau_mbn())
    inline TReal mp_decay_step(void) const { return _ng_mp_decay_step; };

    //calculate membrane potential
    inline TReal calc_volt(const TReal &pre_volt, const TReal &dV) {
        return (pre_volt - V_0()) * _ng_mp_decay_step + V_0() + dV;
    };

    //initialising the neuron group, 
    //the argument is the simulation time step
    void init(const TReal &);

    //neuronal firing function 
    TReal eqn_firing(const TReal &) const; // return spike generation function

    //psp delay when propagating over a distance of s
    inline TReal eqn_psp_decay(const TReal &s) const;

    //percentage of synapse bwteen two individual neuron groups, where (x,y) is 
    //the distance between the two, and w is the size of elements
    TReal eqn_synp_ratio(const TReal &x, const TReal &y, const TReal &w) const;

    //print out the neuron group settings
    std::string print(const std::vector<Layer> LyArry) const;
    std::string print() const;

    static const char *NG_ParamName[];
    static const TReal NG_ParamMax[];
    static const TReal NG_ParamMin[];

    //return how many neuron group in the program
    static TInt ng_count(void) { return NG_cnt; };

};

inline void ng_swap(NeurGrp &a, NeurGrp &b)
{
    a.swap(b);
}

inline TReal NeurGrp::eqn_psp_decay(const TReal &s) const
{
    assert(_ng_state);
    assert(s >= 0);
    //exponential decay
    return exp(-1.*s / _ng_paramVal[NG_IDX_PSP_DECAY]);
}

bool ng_check_idx(const std::vector<NeurGrp>& NeurArry);

#endif /* end of #ifndef NEURGRP_H */
