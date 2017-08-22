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

#ifndef LCM_H
#define LCM_H

//--------------------------------------------------
// This is a major class that defines a LCM model
// 
// The tasks of the class include:
//    Create and set up a layer list "gLayer"
//    Create and set up a receptor list "gRcpt"
//    Create and set up a neuron group list "gNeur"
//    Create and set up a external source list "gExSrc"
//    Create and set up a stimulator list "gStim"
//    Set the parameters of all the objects
//    Set up and maintain the connection between the objects
//--------------------------------------------------

#include "misc.h"
#include "rand.h"
#include "layer.h"
#include "neurgrp.h"
#include "receptor.h"
#include "synpconn.h"
#include "exsource.h"
#include "stimulator.h"
#include <map>
#include <set>

#ifndef LCM_PARA_NUM
#define LCM_PARA_NUM       4
#define LCM_IDX_GRID_SIZE  0
#define LCM_IDX_GRID_ROW   1
#define LCM_IDX_TIME_STEP  2
#define LCM_IDX_SIMU_TIME  3
#endif

class LCM {

protected:

    //spike propagation delay (in simulation steps) over two elements 
    TInt       **gSpk_delay; //[n1][n2], n1==gNG_num, n2==gGrid_row * gGrid_row * SPK_PATH_NUM

#ifndef SPK_DELAY_IDX
#define SPK_DELAY_IDX(dx, dy, ipath) (ipath + SPK_PATH_NUM*(dy + gGrid_row*dx))
   //(dx,dy) is the displacement between two elements, and ipath is the pathway number
   //Periodic boundary conditions is adopted in this function
   //Fow two elements at (x1,y1) and (x2,y2), there is four possible pathways
   //ipath==0: dx = |x1-x2|, and dy = |y1-y2|
   //ipath==1: dx = gGrid_row - |x1 - x2|, and dy = |y1-y2|; //go over the horizontal boundary
   //ipath==2: dx = |x1-x2|, and dy = gGrid_row - |y1-y2|; //go over the vertical boundary
   //ipath==3: dx = gGrid_row - |x1 - x2| and dy = gGrid_row - |y1-y2|; //go over both boundary
#endif

   //the ratio of synapses formed between two groups at a distance 
    TReal     **gSynp_pct;  //[n1][n2], n1==gNG_num, n2==gElmt_num * gElmt_num * SPK_PATH_NUM
#ifndef SYNP_PCT_IDX
#define SYNP_PCT_IDX(ix, iy, ipath) (ipath + SPK_PATH_NUM*(iy + gElmt_num*ix))
   //the same as SPK_DELAY_IDX, except gSynp_pct is dependent on the number of the elements
   //not just the displacement betweent them
#endif

   //LCM structure memebers
    std::vector<Layer>       gLayer;
    std::vector<Receptor>    gRcpt;
    std::vector<Receptor>    gRcpt_excit;
    std::vector<Receptor>    gRcpt_inhib;
    std::vector<NeurGrp>     gNeur;
    std::vector<ExSource>    gExSrc;
    std::vector<Stimulator>  gStim;

    TInt                     gElmt_num;  // == gGrid_row * gGrid_row
    TInt                     gNG_num; // how many neuron group type
    TInt                     gGrid_row;  // the row/column number of the grid

    std::vector<TReal>       gLy_dist;

#ifndef LY_DIST_IDX
#define LY_DIST_IDX(ilayer, jlayer) (jlayer + ilayer*gLayer_num)
#endif

    std::set<std::string>    gObj_name_lst;

    TReal    gROI_size; // the size of the grid

    TReal    gTotal_time; // The duration of the whole simulation
    TReal    gStep_size; // the step size of simulation evolution

    bool     _lcm_paramFlg[LCM_PARA_NUM];
    TReal    gElmt_size; // the size of element
    TReal    gInv_step;  // equal to 1/step_size

    TInt     gLayer_num; // number of layers

    TInt     gRcpt_type; // how many rceptor type
    TInt     gExSrc_num; // how many external sources
    TInt     gStim_num;  // how many stimulators
    TInt     gTotal_step;  // total simulation steps

    TReal    gV_rev_min;
    TReal    gV_rev_max;
    bool     _l_state;       // the state of LCM
    bool     _l_neur_state;  // the state of gNeur
    bool     _l_layer_state; // the state of gLayer
    bool     _l_exsrc_state; // the state of gExSrc

public:

    static const char *LCM_paramName[];
    static const TReal LCM_paramMin[];
    static const TReal LCM_paramMax[];


    //constructor
    LCM(void);
    //destrutor
    ~LCM(void);

    //return true, if a string is a valid object name and having not been used
    //return false otherwise
    bool check_name(const std::string&) const;

    //add a neuron group with a name
    bool add_neur(const std::string&);

    //add a layer with a name
    bool add_layer(const std::string&);

    //add a receptor with a name
    bool add_rcpt(const std::string&);

    //add a stimulator with a name
    bool add_stim(const std::string&);

    //add a external source with a name
    bool add_exsrc(const std::string&);

    //set the values of a set of parameters, the parameter name must be in full formate, 
    //e.g., NEURON.E1.PARA_NAME
    bool set_param(std::map<std::string, std::string>&);

    //set the value of a single parameter
    bool set_param(std::string paramName, std::string paramVal);

    //set parameter value for a neuron group, the parameter name must be in short formate
    //e.g., FIRE_MEAN
    bool set_neur_param(NeurGrp& obj, std::string paramName, std::string paramVal);

    //set parameter value for a layer object
    bool set_layer_param(Layer& obj, std::string paramName, std::string paramVal);

    //set parameter value for a receptor
    bool set_rcpt_param(Receptor& obj, std::string paramName, std::string paramVal);

    //set parameter value for a stimulator
    bool set_stim_param(Stimulator& obj, std::string paramName, std::string paramVal);

    //set parameter value for a synaptic connection
    bool set_synp_conn(std::string pre, std::string post, std::string paramVal);
    bool set_synp_conn(std::string pre, std::string post, std::string ly, std::string paramVal);

    //set the value of a global parameter 
    bool set_simu_param(std::string paramName, std::string paramVal);

    //return the index of a neuron group
    std::vector<NeurGrp>::size_type idx_neuron(const std::string&);

    //return the index of a layer
    std::vector<Layer>::size_type idx_layer(const std::string&);

    //return the index of a external source
    std::vector<ExSource>::size_type idx_exsrc(const std::string&);

    //initialise the LCM
    bool init(void);

    //return true, if the model is ready for running
    //return false, otherwise
    //init() must be called before this check the state 
    inline bool is_ready(void) { return _l_state; };

    //return the distance of two layers
    inline TReal ly_dist(const TInt& ilayer, const TInt& jlayer)
    {
        return gLy_dist[ilayer + jlayer*gLayer_num];
    }

    //the same as above except using element index as argument
    inline TInt idx4rcpt(const TInt& ielmt, const TInt& ineur, const TInt& ircpt) const {
        return ircpt + gRcpt_type * (ineur + gNG_num * ielmt);
    };

    //return the value of data members

    //return the number of rows
    inline TInt  grid_row(void) const { return gGrid_row; };

    //return total simulation time 
    inline TReal total_time(void) const { return gTotal_time; };

    //return simulation time step size
    inline TReal time_step(void) const { return gStep_size; };

    //return simulation time step size
    inline TReal step_size(void) const { return gStep_size; };

    //return the the size of a element
    inline TReal elmt_size(void) const { return gElmt_size; };

    //return the area of a element
    inline TReal elmt_area(void) const { return gElmt_size*gElmt_size; };

    //return the size of the simulated cortical area
    inline TReal ROI_size(void) const { return gROI_size; };

    //return the reciprocal of the time_step == 1 / time_step
    inline TReal inv_step(void) const { return gInv_step; };

    //return the number of layers in the model
    inline TInt layer_num(void) const { return gLayer_num; };

    //return the number of elements == grid_row() * grid_row()
    inline TInt elmt_num(void) const { return gElmt_num; };

    //return the types of receptors
    inline TInt rcpt_num(void) const { return gRcpt_type; };

    //return the types of the neuron 
    inline TInt ng_num(void) const { return gNG_num; };

    //return the number of external sources in the model
    inline TInt exsrc_num(void) const { return gExSrc_num; };

    //return the number of external stimulators in the model
    inline TInt stim_num(void) const { return gStim_num; };

    //return the total number of steps  == TInt(total_time()/time_step () + 0.5)
    inline TInt total_step(void) const { return gTotal_step; };

    //return the maximum voltage neurons can achieve == maximum V_rev of among neuron groups
    inline TReal max_volt(void) const { return gV_rev_max; };

    //return the minimum voltage neurons can achieve == minimum V_rev of among neuron groups
    inline TReal min_volt(void) const { return gV_rev_min; };

    //return the name of objects
    //returnt the name of a neuron group
    inline std::string neur_name(const TInt& idx) const { return gNeur[idx].name(); };

    //return the name of a receptor type
    inline std::string rcpt_name(const TInt& idx) const { return gRcpt[idx].name(); };

    //return the name of a external sources
    inline std::string exsrc_name(const TInt& idx) const { return gExSrc[idx].name(); };

    //return the name of a stimulator
    inline std::string stim_name(const TInt& idx) const { return gStim[idx].name(); };

    //return a reference of the object
    const Layer& layer(const TInt& idx) const {
        assert(idx < gLayer.size());
        return gLayer[idx];
    };

    Layer& layer(const TInt& idx) {
        assert(idx < gLayer.size());
        return gLayer[idx];
    };

    const Receptor& receptor(const TInt& idx) const {
        //assert(idx<gRcpt.size()); 
        //return gRcpt[idx]; 
        for (std::vector<Receptor>::const_iterator it = gRcpt_excit.begin(); it != gRcpt_excit.end(); ++it)
            if (it->index() == idx) return *it;
        for (std::vector<Receptor>::const_iterator it = gRcpt_inhib.begin(); it != gRcpt_inhib.end(); ++it)
            if (it->index() == idx) return *it;
        throw std::string("No matched receptor found!");
    };

    Receptor& receptor(const TInt& idx) {
        //assert(idx<gRcpt.size()); 
        //return gRcpt[idx]; 
        for (std::vector<Receptor>::iterator it = gRcpt_excit.begin(); it != gRcpt_excit.end(); ++it)
            if (it->index() == idx) return *it;
        for (std::vector<Receptor>::iterator it = gRcpt_inhib.begin(); it != gRcpt_inhib.end(); ++it)
            if (it->index() == idx) return *it;
        throw std::string("No matched receptor found!");
    };

    const NeurGrp& neur_group(const TInt& idx) const {
        assert(idx < gNeur.size());
        return gNeur[idx];
    };

    NeurGrp& neur_group(const TInt& idx) {
        assert(idx < gNeur.size());
        return gNeur[idx];
    };

    const ExSource& external_source(const TInt& idx) const {
        assert(idx < gExSrc.size());
        return gExSrc[idx];
    };

    ExSource& external_source(const TInt& idx) {
        assert(idx < gExSrc.size());
        return gExSrc[idx];
    };

    const Stimulator& stimulator(const TInt& idx) const {
        assert(idx < gStim.size());
        return gStim[idx];
    };

    Stimulator& stimulator(const TInt& idx) {
        assert(idx < gStim.size());
        return gStim[idx];
    };

    //convert the configuration of the current model to a string
    std::string print() const;

};

#endif /* end of #ifndef LCM_H */
