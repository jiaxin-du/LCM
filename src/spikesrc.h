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

#ifndef SPIKESRC_H
#define SPIKESRC_H

#include "misc.h"
#include "synpconn.h"

class SpikeSrc
{
protected:
    //data member 
    //synp is used to store the synaptic connection projecting from this source
    std::vector<SynpConn> _synpLst;

    //the index
    //NB: external sources and neuron groups share the index
    TInt           _src_idx;

    //the name
    std::string    _src_name;

    // static member, counting how many spike source, i.e. external sources neuron groups, 
    // in the program
    static TInt    SS_cnt;

public:
    // default constructor
    SpikeSrc(const std::string& xname = "UNNAMED_SOURCE", const TInt& idx = SS_cnt);

    // copy constructor, notice that the index is also copied to the new object
    SpikeSrc(const SpikeSrc&);

    // destructor
    ~SpikeSrc(void);

    // use default copy operator
    // assignment operator
    //SpikeSrc& operator= (const SpikeSrc&);

    // change the index of the spike source
    inline void set_idx(const TInt& idx) { _src_idx = idx; };

    // return the index of the spike source
    inline TInt index(void) const { return _src_idx; };

    // return the name of the spike source
    inline std::string name(void) const { return _src_name; };

    // set the synapse number of a synaptic connection
    // if a matching connection is already,it will update the synaptic number
    // otherwise it will add a new connection
    void set_synp_conn(const TReal& val, const TInt& post, const TInt& ilayer);

    // set the synapse number of a synaptic connection
    // if a matching connection is found, it will update the connection
    // otherwise it will add a new connection
    void set_synp_conn(const SynpConn& p);

    // delete a synaptic connection
    // return true if a matching synaptic connection is found and erased
    // return false otherwise
    bool del_synp_conn(const TInt& post, const TInt& ilayer);

    // delete all synaptic connections attached to spike source
    void del_synp_conn(void) { _synpLst.clear(); };

    // set the property of a synaptic connection
    // corresponding value will be updated for a connection with 
    // matching postsynaptic neuron group and layer
    bool set_eqM_const(const TReal& val, const TInt& post, const TInt& ilayer);

    // set spike propagation delay for a synaptic connection
    bool set_spk_delay(const TInt& val, const TInt& post, const TInt& ilayer);

    // set psp propagation delay for a synaptic connection
    bool set_psp_delay(const TInt& val, const TInt& post, const TInt& ilayer);

    // set psp decay ratio for a synaptic connection
    bool set_psp_decay(const TReal& val, const TInt& post, const TInt& ilayer);

    // initialise the spike source 
    void init(void);

    // return how many synaptic connection attached the spike source
    inline TInt synp_conn_num() const { return _synpLst.size(); };

    //return a read-only reference of synpLst
    const std::vector<SynpConn>& synp_conn() const { return _synpLst; };

    //return a normal reference of synpLst
    std::vector<SynpConn>& synp_conn() { return _synpLst; };

    //return how many SpikeSrc object (including inherited class) the whole program have
    static TInt src_count() { return SS_cnt; };

};
#endif /* end of #ifndef SPIKESRC_H */
