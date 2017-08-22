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

#ifndef SIMULATION_H
#define SIMULATION_H

#include "lcm.h"
#include "array.h"
#include "omp.h" 

#ifndef VOLT_EPS
#define VOLT_EPS 1e-6
#endif

class TTimeWin {
public:
    TTimeWin() { pnt_num = 0; };

    TReal bgn_time;
    TReal end_time;
    TReal inc_time;
    TInt  bgn_step;
    TInt  end_step;
    TInt  inc_step;
    TInt  pnt_num;
};

class Simulation : public LCM
{
private:

    DynamicArray      ***gPSP; //array for PSP
    DynamicArray      **gVolt; //array for membrane potential

    std::vector<TInt> gElmtX;
    std::vector<TInt> gElmtY;

    //TInt              gVolt_arry_size;
    //TInt              gPSP_arry_size;

    TInt              tCheck_pnt;
    TInt              tEvlt_step;

    TInt              gRand_seed;
    TInt              gThread_num;

    std::vector<TTimeWin> output_time;

    std::string       cfg_file;
    std::string       cfg_str;

    bool              tOut_flg;

    bool              simu_state;

public:
    //constructor
    Simulation(void);

    //destructor
    ~Simulation(void);

    //load parameter settings from a string
    void load(std::map<std::string, std::string> &paramList);

    //load parameter settings from a file 
    void load_from_file(const std::string& fname);

    //initialise the simulation for running
    //this function is automatically called after the parameter is loaded
    //this function must be called after LCM paramter is changed
    bool init(void);

    //advance the simulation one step forward
    //time evolution: t <- t+dt
    void advance(void);

    //return the step and time in simulation evolution 
    inline TInt  evlt_step(void) const { return tEvlt_step; };
    inline TReal evlt_time(void) const { return tEvlt_step*gStep_size; };

    //return the voltage of a neuron group
    inline TReal Volt(const TInt& ielmt, const TInt& ineur) {
        return gVolt[ielmt][ineur].rear();
    }

    //get the thread number specified by the user
    inline TInt thread_num() { return gThread_num; };

    //get the header of the data file
    void get_data_header(std::vector<char> &);

    //get a data block contain the following info:
    // curr_time + voltage for all neuronal groups in elements + a '\0'
    // the time and voltage info are with type of 'TFloat'
    // all data are re-formated to a array of unsigned char
    void get_data_block(std::vector<char> &);

    //get the parameter configuration
    inline std::string get_cfg() {
        if (cfg_str.empty()) cfg_str = Simulation::print();
        return cfg_str;
    };

    //return whether the data in current step need to be ouput
    inline bool is_out() const { return tOut_flg; };

    //The same as above
    std::string print(void) const;

};
//
//gPSP-postsynaptic potential
//  rear  ==>>>  ==>>>  ==>>>  ==>>>  front
//  t-N*dt                              t
//
//gVolt-membrane potential 
//  rear  ==>>>  ==>>>  ==>>>  ==>>>  front
//   t                               t+N*dt
//
#endif /* end of #ifndef SIMULATION_H */
