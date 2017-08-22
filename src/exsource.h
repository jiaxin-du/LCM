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

#ifndef EXSOURCE_H
#define EXSOURCE_H

#include "misc.h"
#include "spikesrc.h"
#include "stimulator.h"

//--------------------------------------------------
//  This class defines a external spike source 
//  The external source only defines the synaptic 
//    connections from external region to LCM. 
//  The form of afferent spikes is defined by 
//    stimulators attached to the external source
//  
//  See class Stimulator defined in 
//                  stimulator.cpp and stimulator.h
//--------------------------------------------------

class ExSource : public SpikeSrc
{
public:
    //constructor
    ExSource(const std::string &xname = "UNNAMED_SOURCE", const TInt &idx = SpikeSrc::SS_cnt);

    //copy constructor
    ExSource(const ExSource &p);

    //destrutor
    ~ExSource();

    //assignment operator
    ExSource & operator= (const ExSource &);

    //set parameter. this is a dummy function, external source has no parameters
    //this function is reserved for future use
    virtual bool set_param(std::string const &paramName, std::string const &paramVal) { return true; };

    //attach a stimulators to the external source
    inline void add_stim(const Stimulator &st) { _es_stim.push_back(st); };

    //check whether the external source is ready or not
    inline bool is_ready(void) const { return _es_state; };

    //return the number of active stimulators
    inline TInt act_stim_num(void) const { return _Nact_stim; };

    //return the number of total stimulators attached to the source
    inline TInt stim_num(void) const { return _es_stim.size(); };

    //return the next check point
    //A check point is a time point where the form of afferent spikes 
    // need to be changed, for example, changes of stimulation magnitude
    inline TInt check_point(void) const { return _chk_pnt; };

    //return how many elements the source is projected to
    inline TInt elmt_num(void) const { return _es_elmt.size(); };

    //return the number of the Nth element 
    inline TInt get_elmt(const TInt& idx) const { return _es_elmt[idx]; };

    //return the name of a stimulator
    std::string stim_name(const TInt& idx) const { return _es_stim[idx].name(); };

    //initialize the external source
    // this function will set the status flag: _es_state
    // always call this function before checking is_ready()
    void init();

    //this function will calculate the next check point
    TInt check(const TInt &c_step);

    //advance the stimulators
    // the afferent spikes of stimulators are time-dependent
    // this function allows the stimulator to advance in time
    void advance();

    //this function will generate spike afferent
    TReal generate(const TInt &idx);

    //print out the function
    std::string print(const TReal &step_size) const;

    //external source count
    static TInt es_count(void) { return ES_cnt; };

    //index base
    static TInt idx_base(void) { return ES_idx_base; };

private:
    //data member
    std::vector<Stimulator>  _es_stim;
    std::vector<TInt>        _es_elmt;

    //TReal **es_phi;
    //this matrix connects elements in exsource to that in stimulators
    TInt                  **_elmt_stim;
    //_elmt_stim[ielmt][ist] is the index of element 'ielmt' in stimulator 'ist'
    //that is _es_stim[ist].elmt_list.at(_elmt_stim[ielmt][ist]) will give the same 
    //value as _es_elmt[ielmt]

    TInt                   _Nact_stim; //how many active stimulator the source have
    TInt                   _chk_pnt;   //next check point
    bool                    _es_state;

    static TInt            ES_cnt;
    static TInt            ES_idx_base;
};


bool es_check_idx(const std::vector<ExSource> &EsArry);

#endif /* end of #ifndef EXSOURCE_H */
