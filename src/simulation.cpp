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
#include "simulation.h"
#include <algorithm>
using namespace std;

//--------------------------------------------------
// function Simulation::Simulation(void)
//   default constructor
//--------------------------------------------------
Simulation::Simulation(void) :
   LCM(), gPSP(NULL), gVolt(NULL), tCheck_pnt(0),
   tEvlt_step(0), gRand_seed(0), gThread_num(0), 
   cfg_file("UNKNOWN"), tOut_flg(false), simu_state(false)
{  }

//--------------------------------------------------
// function Simulation::~Simulation(void)
//   default destructor
//--------------------------------------------------
Simulation::~Simulation(void)
{
   //delete gPSP
   if (gPSP != NULL) {
      for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
         for (TInt ineur = 0; ineur < gNG_num; ++ineur) {
            delete[] gPSP[ielmt][ineur];
         }
         delete[] gPSP[ielmt];
      }
      delete[] gPSP;
      gPSP = NULL;
   }

   //delete gVolt
   if (gVolt != NULL) {
      for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
         delete[] gVolt[ielmt];
      }
      delete[] gVolt;
      gVolt = NULL;
   }
}

void Simulation::load_from_file(const string& fname)
{
   std::ifstream fp(fname.c_str());
   if (!fp.good() || fp.eof()) {
      cerr << "ERROR! Simulation::load_from_file: cannot open the file '" << fname
         << "', or the file is empty! " << _FILE_LINE_ << endl;
      exit(-1);
   }

   cfg_file = fname;

   std::string str;

   fp.seekg(0, std::ios::end);  //get to the end of the file 
   str.reserve(static_cast<long int>(fp.tellg())); //allocate space for str
   fp.seekg(0, std::ios::beg); //return to the beginning of the file

   str.assign((std::istreambuf_iterator<char>(fp)), \
      std::istreambuf_iterator<char>());

   fp.close();

   map<string, string> paramList;

   if (!read_param(str, paramList)) {
      cerr << "Simulation::load_from_file: fail to read parameters from text." << _FILE_LINE_ << endl;
      exit(-1);
   }

   load(paramList);
}


//--------------------------------------------------
// function void Simulation::load(char *fname)
//   this function will load the parameters from a file
//   fname is the path to the parameter file
//  
//   this function also initialises the meodel
//--------------------------------------------------
void Simulation::load(map<string, string> &paramList)
{
   string paramName, paramVal;
   vector<string> parts;

   //output time window setting
   map<string, string>::iterator it = paramList.find("SIMU.OUTPUT_TIME");
   if (it == paramList.end()) {
      cerr << __FUNCTION__ << msg_param_not_set("SIMU.OUTPUT_TIME") << endl;
      exit(-1);
   }
   else {
      if ((it->second)[0] == '{' && (it->second)[(it->second).size() - 1] == '}') {
         (it->second) = (it->second).substr(1, (it->second).size() - 2); // remove leading '{' and ending '}'
      }
      else {
         cerr << __FUNCTION__ << msg_invalid_param_value(it->first, it->second) << endl;
         cerr << "   the value of parameter '" << it->first << "' should be embraced by a pair of '{}'" << endl;
         exit(-1);
      }
      strsplit(it->second, ",", parts); //it->second should be be like "1:0.001:2, 3:0.001:4"

      vector<string> tmpVec;
      TReal tmpVal;
      bool tmpFlg;
      TTimeWin tmpWin;
      for (vector<string>::iterator it2 = parts.begin(); it2 != parts.end(); ++it2) {
         *it2 = strtrim(*it2);
         strsplit(*it2, ":", tmpVec);
         tmpFlg = true;
         if (tmpVec.size() != 3) {
            tmpFlg = false;
         }
         else {

            if (str2float(strtrim(tmpVec[0]), tmpVal)) {
               tmpWin.bgn_time = tmpVal;
            }
            else {
               tmpFlg = false;
            }

            if (str2float(strtrim(tmpVec[1]), tmpVal)) {
               tmpWin.inc_time = tmpVal;
            }
            else {
               tmpFlg = false;
            }

            if (str2float(strtrim(tmpVec[2]), tmpVal)) {
               tmpWin.end_time = tmpVal;
            }
            else {
               tmpFlg = false;
            }

         }

         if (!tmpFlg || tmpWin.bgn_time + tmpWin.inc_time > tmpWin.end_time) {
            cerr << "Simulation::load: the specified output time is invalid! " << _FILE_LINE_ << endl;
            cerr << "** " << it->first << " = " << it->second << ";" << endl;
            exit(-1);
         }

         output_time.push_back(tmpWin);
      }
      for (vector<TTimeWin>::iterator it3 = output_time.begin(); it3 != output_time.end(); ++it3) {
         for (vector<TTimeWin>::iterator it4 = it3 + 1; it4 != output_time.end(); ++it4) {
            if (!((it3->end_time < it4->bgn_time) || (it3->bgn_time > it4->bgn_time))) { //PS. it3->bgn_pnt < it3->end_pnt and it4->bgn_pnt < it4->end_pnt
               cerr << "Simulation::load: the output time is overlapped! " << _FILE_LINE_ << endl;
               cerr << "** " << it->first << " = " << it->second << ";" << endl;
               exit(-1);
            }
            if (it3->bgn_time > it4->end_time) {
               tmpWin = *it3;
               *it3 = *it4;
               *it4 = tmpWin;
               --it3; //re-check *it3
               break;
            }
         }
      }
      paramList.erase(it);
   }

   it = paramList.find("SIMU.RAND_SEED");
   if (it != paramList.end()) {
      TInt int_val;
      if ((!str2int(it->second, int_val)) || (int_val < 0)) {
         cerr << msg_invalid_param_value(it->first, it->second) << endl;
         exit(-1);
      }

      gRand_seed = int_val;

      paramList.erase(it);
   }

   it = paramList.find("SIMU.THREAD_NUM");
   if (it != paramList.end()) {
      TInt int_val;
      if ((!str2int(it->second, int_val)) || int_val < 0) {
         cerr << msg_invalid_param_value(it->first, it->second) << endl;
         exit(-1);
      }
      gThread_num = int_val;

      paramList.erase(it);
   }
   else {
      gThread_num = 0;
   }

   rand_init(gRand_seed, gThread_num);

   //processing the rest of the list 
   if (!LCM::set_param(paramList)) {
      cerr << __FUNCTION__ << ": set parameters failed! " << _FILE_LINE_ << endl;
      exit(-1);
   }

   if (!Simulation::init()) {
      cerr << "ERROR! intialising the simulation failed! " << _FILE_LINE_ << endl;
      exit(-1);
   }

   if (!is_ready()) {
      cerr << "ERROR! The model is not ready! please check the configuration file. " << _FILE_LINE_ << endl;
      exit(-1);
   }

   //cout<<print()<<endl;
}

//--------------------------------------------------
// function void Simulation::init(void)
//   Initialise the simulation environment
//   This function is automatically called in function load(str)
//--------------------------------------------------
bool Simulation::init(void)
{
   if (!LCM::init())
      return false;

   //allocate space for gElmtX and gElmtY
   gElmtX.resize(gElmt_num);
   gElmtY.resize(gElmt_num);

   for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
      gElmtY[ielmt] = ielmt / gGrid_row;
      gElmtX[ielmt] = ielmt - gElmtY[ielmt] * gGrid_row;
   }

   //delete gPSP
   if (gPSP != NULL) {
      for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
         for (TInt ineur = 0; ineur < gNG_num; ++ineur) {
            delete[] gPSP[ielmt][ineur];
         }
         delete[] gPSP[ielmt];
      }
      delete[] gPSP;
      gPSP = NULL;
   }

   //delete gVolt
   if (gVolt != NULL) {
      for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
         delete[] gVolt[ielmt];
      }
      delete[] gVolt;
      gVolt = NULL;
   }

   TInt max_Nrcpt = gRcpt_excit.size();
   if (max_Nrcpt < gRcpt_inhib.size())
      max_Nrcpt = gRcpt_inhib.size();

   //Nneur_x_Nrcpt=gNG_num * max_Nrcpt;
   try {
      gPSP = new DynamicArray**[gElmt_num];
      for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
         gPSP[ielmt] = new DynamicArray*[gNG_num];
         for (TInt ineur = 0; ineur < gNG_num; ++ineur) {
            gPSP[ielmt][ineur] = new DynamicArray[max_Nrcpt];
         }
      }

      gVolt = new DynamicArray*[gElmt_num];
      for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
         gVolt[ielmt] = new DynamicArray[gNG_num];
      }

   }
   catch (bad_alloc& e) {
      cerr << msg_allocation_error(e) << endl;
      exit(-1);
   }

   //determin the length for voltage array
   TInt  max_psp_size = 0;
   for (vector<Receptor>::iterator rc_it = gRcpt_excit.begin(); rc_it != gRcpt_excit.end(); ++rc_it) {
      max_psp_size = std::max(max_psp_size, rc_it->psp_size());
   }

   for (vector<Receptor>::iterator rc_it = gRcpt_inhib.begin(); rc_it != gRcpt_inhib.end(); ++rc_it) {
      max_psp_size = std::max(max_psp_size, rc_it->psp_size());
   }

   TInt max_psp_delay = 0;
   TInt max_elmt_delay = 0;
   TInt max_spk_delay = 0;
   for (vector<NeurGrp>::iterator ng_it = gNeur.begin(); ng_it != gNeur.end(); ++ng_it) {
      max_elmt_delay = std::max(max_elmt_delay, gSpk_delay[ng_it->index()][SPK_DELAY_IDX(gGrid_row, gGrid_row, 0)]);
      for (vector<SynpConn>::const_iterator sy_it = ng_it->synp_conn().begin(); sy_it != ng_it->synp_conn().end(); ++sy_it) {
         max_psp_delay = std::max(max_psp_delay, sy_it->psp_delay());
         max_spk_delay = std::max(max_spk_delay, sy_it->spk_delay());
      }
   }

   TInt volt_arry_size = nextpow2(max_psp_size + max_psp_delay + 1);
   TInt psp_arry_size = nextpow2(max_elmt_delay + max_spk_delay + 1);

   //cout<<"Voltage buff size="<<gVolt_arry_size<<endl;
   //cout<<"PSP buff size="<<gpsp_arry_size<<endl;

   //
   // reserve space for the voltage of each element
   // there is no advantage to parallelise  the section in 
   // memory allocation in operating system is running in a single thread 

   for (TInt ielmt = 0; ielmt != gElmt_num; ++ielmt) {
      for (TInt ineur = 0; ineur != gNG_num; ++ineur) {
         //idx=ielmt*gNG_num+ineur;
         gVolt[ielmt][ineur].resize(volt_arry_size);
         gVolt[ielmt][ineur].set_default(gNeur[ineur].V_0());
         gVolt[ielmt][ineur].fill(gNeur[ineur].V_0());

         gVolt[ielmt][ineur].step_backward();

         if (gNeur[ineur].type() == cEXCIT) {
            //for excitatory neuron only gRcpt[ielmt][ineur][0-1] is valid
            for (std::size_t ircpt = 0; ircpt != gRcpt_excit.size(); ++ircpt) {
               //idx=ielmt*Nneur_x_Nrcpt+ineur*max_Nrcpt+ircpt;
               gPSP[ielmt][ineur][ircpt].resize(psp_arry_size);
               gPSP[ielmt][ineur][ircpt].set_default(0);
               gPSP[ielmt][ineur][ircpt].fill(0);
            }
         }
         else {
            //for inhibitory neuron only gRcpt[ielmt][ineur][0] is valid 
            for (std::size_t ircpt = 0; ircpt != gRcpt_inhib.size(); ++ircpt) {
               //idx=ielmt*Nneur_x_Nrcpt+ineur*max_Nrcpt+ircpt;
               gPSP[ielmt][ineur][ircpt].resize(psp_arry_size);
               gPSP[ielmt][ineur][ircpt].set_default(0);
               gPSP[ielmt][ineur][ircpt].fill(0);
            }
         }

      }
   }

   //check the output time window
   for (vector<TTimeWin>::iterator it = output_time.begin(); it != output_time.end(); ++it) {
      it->bgn_step = static_cast<TInt>(it->bgn_time / LCM::time_step());
      it->end_step = static_cast<TInt>(it->end_time / LCM::time_step());
      it->inc_step = static_cast<TInt>(it->inc_time / LCM::time_step());
      if (it->inc_step <= 0) it->inc_time = 1;
      it->pnt_num = (it->end_step - it->bgn_step) / it->inc_step + 1;
   }

   //check point
   tCheck_pnt = MAX_INT_NUM;
   for (vector<ExSource>::iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
      it->init();
      if (tCheck_pnt > it->check_point())
         tCheck_pnt = it->check_point();
   }

   tEvlt_step = 0;

   simu_state = true;

   cfg_str.clear();

   cfg_str = Simulation::print();

   cout << "INFO: next check point is " << tCheck_pnt*gStep_size << " msec.\n";

   return true;
}

//--------------------------------------------------
// function void Simulation::advance(void)
// The function advances the stimulation for a time step 
//--------------------------------------------------
void Simulation::advance(void)
{

   assert(simu_state); // Simulation::advance: the model is not ready!

   ++tEvlt_step;

   tOut_flg = false;

   if (tEvlt_step > gTotal_step) return;

   for (vector<TTimeWin>::iterator it = output_time.begin(); it != output_time.end(); ++it) {
      if (tEvlt_step > it->end_step) {
         it = output_time.erase(it); //it will point to the next data
         --it; //even if it==output_time.begin(), operation --it will not throw any exception
         continue;
      }
      else if (tEvlt_step >= (it->bgn_step)) {
         if ((tEvlt_step - (it->bgn_step)) % (it->inc_step) == 0) {
            tOut_flg = true;
         }
         break;
      }
   }

   //calculate next check point
   if (tEvlt_step == tCheck_pnt) {
      TInt tmp;
      tCheck_pnt = MAX_INT_NUM;
      for (vector<ExSource>::iterator es_it = gExSrc.begin(); es_it != gExSrc.end(); ++es_it) {
         tmp = es_it->check(tEvlt_step); //update the state of stimulators
         if (tCheck_pnt > tmp) tCheck_pnt = tmp;
      }
      if (tCheck_pnt != MAX_INT_NUM) {
         cout << "INFO: current simulation time=" << evlt_time() << " msec, " \
            "next check point=" << tCheck_pnt*gStep_size << " msec." << endl;
      }
   }


   for (vector<ExSource>::iterator es_it = gExSrc.begin(); es_it != gExSrc.end(); ++es_it) {
      if (es_it->act_stim_num() == 0) continue;

#ifdef _OPENMP //OpenMP options
#pragma omp parallel for
      for (TInt idx = 0; idx < es_it->elmt_num(); ++idx) {
#else
      for (TInt idx = 0; idx < es_it->elmt_num(); ++idx) {
#endif
         TReal phi = es_it->generate(idx);
         if (phi == 0) continue;

         TReal tmp_NM;
         DynamicArray *t_volt;
         for (vector<SynpConn>::const_iterator sy_it = es_it->synp_conn().begin(); sy_it != es_it->synp_conn().end(); ++sy_it) {
            t_volt = gVolt[es_it->get_elmt(idx)] + (sy_it->postsynp());
            tmp_NM = sy_it->weight() * (gV_rev_max - t_volt->rear());
            for (vector<Receptor>::const_iterator rc_it = gRcpt_excit.begin(); rc_it != gRcpt_excit.end(); ++rc_it) {
               t_volt->add2rear(rc_it->psp(), rc_it->psp_size(), sy_it->psp_delay(), tmp_NM * rc_it->eqn_J(phi));
            }
         }
      }//end of OpenMP parallel section
   }

#ifdef _OPENMP //OpenMP options
#pragma omp parallel for
   for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
#else
   for (TInt ielmt = 0; ielmt != gElmt_num; ++ielmt) {
#endif
      for (TInt ineur = 0; ineur < gNG_num; ++ineur) {

         TReal phi = gNeur[ineur].eqn_firing(gVolt[ielmt][ineur].rear());

         if (gNeur[ineur].type() == cEXCIT) {

            for (TInt ircpt = 0; ircpt < gRcpt_excit.size(); ++ircpt) {
               gPSP[ielmt][ineur][ircpt].step_backward();
               gPSP[ielmt][ineur][ircpt].set_front(gRcpt_excit[ircpt].eqn_J(phi));
            }

         }
         else {

            for (TInt ircpt = 0; ircpt < gRcpt_inhib.size(); ++ircpt) {
               gPSP[ielmt][ineur][ircpt].step_backward();
               gPSP[ielmt][ineur][ircpt].set_front(gRcpt_inhib[ircpt].eqn_J(phi));
            }
         }
      }
   }

#ifdef _OPENMP
#pragma omp parallel for
   for (TInt t_elmt = 0; t_elmt < gElmt_num; ++t_elmt) { //loop over the target elements
#else
   for (TInt t_elmt = 0; t_elmt != gElmt_num; ++t_elmt) { //loop over the target elements
#endif

      TInt d_x, d_y, s_neur, ircpt;
      TInt delay;
      TReal tmp_NM, mag;

      TInt *spk_delay;
      TReal *synp_pct;
      DynamicArray *t_volt, *s_psp;

      vector<Receptor>::const_iterator rcpt_begin, rcpt_end;

      for (vector<NeurGrp>::const_iterator sn_it = gNeur.begin(); sn_it != gNeur.end(); ++sn_it) {
         //if(sn_it->synp.empty()) continue;

         s_neur = sn_it->index();

         //determine the target receptor
         if (sn_it->type() == cEXCIT) {
            rcpt_begin = gRcpt_excit.begin();
            rcpt_end = gRcpt_excit.end();
         }
         else {
            rcpt_begin = gRcpt_inhib.begin();
            rcpt_end = gRcpt_inhib.end();
         }

         //loop over the synaptic connection, sn_it->synps() gives all the synaptic connection the neuron group projecting to
         for (vector<SynpConn>::const_iterator sy_it = sn_it->synp_conn().begin(); sy_it != sn_it->synp_conn().end(); ++sy_it) {

            t_volt = &(gVolt[t_elmt][sy_it->postsynp()]); // pointer to target neuron group

            tmp_NM = sy_it->weight() * (sn_it->V_rev() - t_volt->rear());

            ircpt = 0;
            for (vector<Receptor>::const_iterator rc_it = rcpt_begin; rc_it != rcpt_end; ++rc_it) { //loop over the target receptor

               mag = 0.;
               for (TInt s_elmt = 0; s_elmt < gElmt_num; ++s_elmt) { //loop over the source elements

                  d_x = abs(gElmtX[t_elmt] - gElmtX[s_elmt]);
                  d_y = abs(gElmtY[t_elmt] - gElmtY[s_elmt]);
                  s_psp = &(gPSP[s_elmt][s_neur][ircpt]);

                  //path 0, see LCM::init()
                  spk_delay = gSpk_delay[s_neur] + SPK_DELAY_IDX(d_x, d_y, 0);
                  synp_pct = gSynp_pct[s_neur] + SYNP_PCT_IDX(s_elmt, t_elmt, 0);
                  if (*synp_pct > SYNP_RATIO_EPS) {
                     delay = sy_it->spk_delay() + (*spk_delay);
                     mag += tmp_NM * (*synp_pct) * s_psp->get_front(delay);
                  }

                  //path 1, see LCM::init()
                  ++spk_delay;
                  ++synp_pct;
                  if (*synp_pct > SYNP_RATIO_EPS) {
                     delay = sy_it->spk_delay() + (*spk_delay);
                     mag += tmp_NM * (*synp_pct) * s_psp->get_front(delay);
                  }

                  //path 2, see LCM::init()
                  ++spk_delay;
                  ++synp_pct;
                  if (*synp_pct > SYNP_RATIO_EPS) {
                     delay = sy_it->spk_delay() + (*spk_delay);
                     mag += tmp_NM * (*synp_pct) * s_psp->get_front(delay);
                  }

                  //path 3, see LCM::init()
                  ++spk_delay;
                  ++synp_pct;
                  if (*synp_pct > SYNP_RATIO_EPS) {
                     delay = sy_it->spk_delay() + (*spk_delay);
                     mag += tmp_NM * (*synp_pct) * s_psp->get_front(delay);
                  }

               } //end of loop for source element

               if (mag > VOLT_EPS) {
                  t_volt->add2rear(rc_it->psp(), rc_it->psp_size(), sy_it->psp_delay(), mag);
               }

               ++ircpt;
            } //end of loop for receptor
         } //end of loop for synaptic connections
      } //end of loop for neuron groups
   } //end of loop for target element      

   //calculate the membrane potentials for neuron groups
#ifdef _OPENMP
#pragma omp parallel for
   for (TInt s_elmt = 0; s_elmt < gElmt_num; ++s_elmt) {
#else
   for (TInt s_elmt = 0; s_elmt != gElmt_num; ++s_elmt) {
#endif
      DynamicArray *t_volt;
      TReal pre_volt, curr_volt;
      for (TInt s_neur = 0; s_neur < gNG_num; ++s_neur) {

         t_volt = &(gVolt[s_elmt][s_neur]);

         pre_volt = t_volt->rear(); //previous step value

         t_volt->step_backward(); 

         //current step value is t_volt->rear();

         //(V_(n-1) - V_0) * decay_factor + (V_n -V_0)
         curr_volt = (pre_volt - gNeur[s_neur].V_0()) * gNeur[s_neur].mp_decay_step() + \
            t_volt->rear();

         if (curr_volt< gV_rev_min) {
            curr_volt = gV_rev_min;
         }
         else if (curr_volt > gV_rev_max) {
            curr_volt = gV_rev_max;
         }

         t_volt->set_rear(curr_volt);
      }
   }

   //move the stimulator a step forward
   for (vector<ExSource>::iterator es_it = gExSrc.begin(); es_it != gExSrc.end(); ++es_it) {
      if (es_it->act_stim_num() == 0) continue;
      es_it->advance(); //prepare the stimulators
   }

}


string Simulation::print(void) const
{
   if (!simu_state) {
      cerr << __FUNCTION__ << ": the model is not ready!" << _FILE_LINE_ << endl;
      exit(-1);
   }

   //if(!cfg_str.empty()) return cfg_str;

   ostringstream  oss;
   time_t raw_time;

   time(&raw_time);
   string str(ctime(&raw_time));
   oss << "//Parameter for LCM simulation." << endl;
   oss << "//runing time: " << str << endl;

   oss << "//Simulation parameters" << endl;
   oss << "SIMU {" << endl;
   oss << "\tOUTPUT_TIME = {";
   for (vector<TTimeWin>::const_iterator it = output_time.begin(); it != output_time.end(); ++it) {
      if (it != output_time.begin()) oss << ", ";
      oss << it->bgn_time << ":" << it->inc_time << ":" << it->end_time;
   }
   oss << "}; // {";
   for (vector<TTimeWin>::const_iterator it = output_time.begin(); it != output_time.end(); ++it) {
      if (it != output_time.begin()) oss << ", ";
      oss << it->bgn_step << ":" << it->inc_step << ":" << it->end_step << " (" << it->pnt_num << ")";
   }
   oss << "}" << endl;
   oss << "\tRAND_SEED = " << rand_seed() << "; //input value = " << gRand_seed << endl;
   oss << "\tTHREAD_NUM = " << gThread_num << ";" << endl;
   oss << "};" << endl << endl;

   oss << LCM::print() << endl;

   return oss.str();
}

void Simulation::get_data_header(vector<char> &buff)
{
   if (!is_ready()) {
      cerr << __FUNCTION__ << ": the model is not ready!" << _FILE_LINE_ << endl;
      exit(-1);
   }

   ostringstream  oss;

   time_t raw_time;

   time(&raw_time);

   char time_stamp[32];

   //get the time stamp
   strftime(time_stamp, 31, "%Y/%b/%d %H:%M:%S", localtime(&raw_time));

   TInt block_num = 0;

   for (vector<TTimeWin>::iterator it = output_time.begin(); it != output_time.end(); ++it) {
      block_num += it->pnt_num;
   }

   TInt block_size = (elmt_num() * ng_num() + 1) * sizeof(TFloat) + 1;

   oss << "//Laminar cortex model by Jiaxin Du (jiaxin.du@uqconnect.edu.au)" << endl;
   oss << "DATE = " << time_stamp << "; //creation date" << endl;
   oss << "CFG_FILE = " << cfg_file << "; //parameter configure file" << endl;
   oss << "ELMT_NUM = " << elmt_num() << "; //number of element" << endl;
   oss << "NG_NUM = " << ng_num() << "; //number of neuron group" << endl;
   oss << "NUM_SIZE = " << sizeof(TFloat) << "; //data type size" << endl;
   oss << "HEADER_POS = 0; //header section position" << endl;
   oss << "HEADER_LEN = 1024; //header section length" << endl;
   oss << "CFG_POS = 1024; //configure section position" << endl;
   oss << "CFG_LEN = " << (cfg_str.size() + 1) << "; //configure section length" << endl;
   oss << "DATA_POS = " << (cfg_str.size() + 1025) << "; //data section position" << endl;
   oss << "DATA_LEN = " << block_num * block_size << "; //data section length" << endl;
   oss << "BLOCK_SIZE = " << block_size << "; //a data block size" << endl;
   oss << "BLOCK_NUM = " << block_num << "; //number of block in data section" << endl;
   oss << "DIM1 = NEURON; //voltage array idx=ineur+ielmt*neur_num" << endl;
   oss << "OUTPUT_TIME = {";
   for (vector<TTimeWin>::iterator it = output_time.begin(); it != output_time.end(); ++it) {
      if (it != output_time.begin()) oss << ", ";
      oss << it->bgn_time << ":" << it->inc_time << ":" << it->end_time;
   }
   oss << "} ; //the time period of the voltage data " << endl;
   oss << "SECTION_NUM = " << output_time.size() << "; //number of sections for the voltage data" << endl;

   string str = oss.str();

   buff.clear();

   buff.reserve(1025 + cfg_str.size()); //the size of the header = 1024(file info) + cfg_str.size() + 1 (a ending 0)

   str.resize(1023, '\0');
   str.push_back('\0');

   buff.insert(buff.end(), str.begin(), str.end()); // copy file info

   while (buff.size() < 1024) {
      buff.push_back('\0');//padding the vector to 1024 byte
   }

   buff.insert(buff.end(), cfg_str.begin(), cfg_str.end()); //copy model configuration

   buff.push_back(0);   //add a ending zero
}

void Simulation::get_data_block(vector<char> &buff)
{
   TInt block_size = (elmt_num() * ng_num() + 1) * sizeof(TFloat) + 1;

   buff.clear();

   buff.reserve(block_size);

   TFloat tmp = static_cast<TFloat>(evlt_time());
   char *pos = (char *)(&tmp);

   buff.insert(buff.end(), pos, pos + sizeof(TFloat));

   for (TInt ielmt = 0; ielmt < elmt_num(); ++ielmt) {
      for (TInt ineur = 0; ineur < ng_num(); ++ineur) {
         tmp = static_cast<TFloat>(gVolt[ielmt][ineur].rear());
         buff.insert(buff.end(), pos, pos + sizeof(TFloat));
      }
   }

   buff.push_back(0);

   if (buff.size() != block_size) {
      cerr << "Simulation::get_data_block: the size of the data block is not right!" << _FILE_LINE_ << endl;
      cerr << "**buff.size() = " << buff.size() << ", but block_size = " << block_size << endl;
      exit(-1);
   }

}
