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

#ifndef SYNPCONN_H
#define SYNPCONN_H

#include "misc.h"

//minimum synapse number for a significant connection
//synaptic connections whose synapse number is smaller
//than this value are considered to be dummys
//and will be remove at initialisation stage
#ifndef   SYNP_EPS
#define   SYNP_EPS   0.01
#endif

class SynpConn
{

private:
    TInt    _sy_spk_delay; // delay steps from presynaptic neuron soma to target synapse
    TInt    _sy_psp_delay; // delay steps from synapse to postsynapse neuron soma
    TReal    _sy_weight;    // sy_weight=sy_synap * sy_psp_decay * sy_eqM_const;
    TReal    _sy_eqM_const; // the constant in equation M
    TReal    _sy_psp_decay; // decay of psp amplitude from synapse to postsynapse neuron soma
    TInt     _sy_pre;       //NB: _sy_pre, _sy_psot and _sy_layer are stored as indice
    TInt     _sy_post;      //    in a global neuron group and cortical layer arrays, 
    TInt     _sy_layer;     //    therefore it is crucial to initialise these arrays 
    TReal    _sy_synp;      //    before set these values of the synaptic connections 
    bool     _sy_state;

    static TInt SY_count;

public:
    //constructors
    SynpConn(void);
    SynpConn(const TReal& val, const TInt& pre, const TInt& post, const TInt& ilayer);
    SynpConn(const SynpConn &p);

    //destructor
    ~SynpConn(void);

    //use default copy operator
    //assignment operator
    //SynpConn& operator= (const SynpConn &);

    //data member interface
    //All the data members are open to the outside
    //const reference for const object
    inline TInt  presynp(void) const { return _sy_pre; };
    inline TInt  postsynp(void) const { return _sy_post; };
    inline TInt  layer(void) const { return _sy_layer; };
    inline TReal synp(void) const { return _sy_synp; };
    inline TInt  spk_delay(void) const { return _sy_spk_delay; };
    inline TInt  psp_delay(void) const { return _sy_psp_delay; };
    inline TReal psp_decay(void) const { return _sy_psp_decay; };
    inline TReal weight(void) const { return _sy_weight; };
    inline TReal eqM_const(void) const { return _sy_eqM_const; };

    //set pre-synaptic neuron group
    inline bool set_presynp(const TInt& pre) {
        if (pre < 0) {
            return false;
        }
        _sy_pre = pre;
        return true;
    };

    //set post-synaptic neuron group
    inline bool set_postsynp(const TInt& post) {
        if (post < 0) {
            return false;
        }
        _sy_post = post;
        return true;
    };

    //set layer
    inline bool set_layer(const TInt& ly) {
        if (ly < 0) {
            return false;
        }
        _sy_layer = ly;
        return true;
    };

    //set synapse number
    inline bool set_synp(const TReal& sy) {
        if (sy < 0) {
            return false;
        }

        _sy_synp = sy;

        _sy_weight = _sy_synp * _sy_psp_decay * _sy_eqM_const;
        return true;
    };

    //set spike propagation delay
    inline void set_spk_delay(const TInt& delay) {
        _sy_spk_delay = delay;
    };

    //set PSP propagation delay
    inline void set_psp_delay(const TInt& delay) {
        _sy_psp_delay = delay;
    };

    //set PSP decay ratio
    inline bool set_psp_decay(const TReal& decay) {
        if (decay < 0 || decay>1)
            return false;
        _sy_psp_decay = decay;
        _sy_weight = _sy_synp * _sy_psp_decay * _sy_eqM_const;
        return true;
    };

    //set the constant of equation M 
    inline void set_eqM_const(const TReal& val) {
        _sy_eqM_const = val;
        _sy_weight = _sy_synp * _sy_psp_decay * _sy_eqM_const;
    };

    //set the synaptic connections
    void set(const TReal& val, const TInt& pre, const TInt& post, const TInt& ilayer);

    //check whether the synaptic connection is a dummy
    //return true if the synapse number is significant
    //return fase otherwise
    inline bool is_dummy(void) {
        if (_sy_synp < SYNP_EPS)
            return true;
        else
            return false;
    };

    //check whether the synaptic connection is ready for running
    bool  is_ready(void) const;

    //return how many synaptic connections in the program
    static TInt count(void) { return SY_count; };

    //swap the conetent of the synaptic connection with another one
    void swap(SynpConn& p);
};

inline void sy_swap(SynpConn& a, SynpConn& b)
{
    a.swap(b);
};

#endif /* end of #ifndef SYNPCONN_H */
