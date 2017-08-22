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
#include "spikesrc.h"

using namespace std;

TInt SpikeSrc::SS_cnt = 0;

//
// default constructor
//
SpikeSrc::SpikeSrc(const string& xname, const TInt& idx) :
   _src_idx(idx),
   _src_name(xname) 
{
   ++SS_cnt;
}

//
// copy constructor
SpikeSrc::SpikeSrc(const SpikeSrc& p)
{
   *this = p;
   ++SS_cnt;
}

//
// default destructor
SpikeSrc::~SpikeSrc(void)
{
   --SS_cnt;
}

//
// assignment operator
//SpikeSrc& SpikeSrc::operator= (const SpikeSrc& p)
//{
//   if (this != &p) {
//
//      _src_idx = p._src_idx;
//      _src_name = p._src_name;
//      _synpLst = p._synpLst;
//   }
//
//   return *this;
//}

//
// set the synapse number of a synaptic connection
// add a new synaptic connection if cannot find one 
// macthing the postsynaptic neuron and layer
void SpikeSrc::set_synp_conn(const TReal& val, const TInt& post, const TInt& ilayer)
{
   for (vector<SynpConn>::iterator it = _synpLst.begin(); it != _synpLst.end(); ++it) {
      if (it->postsynp() == post && it->layer() == ilayer) {
         if (val > SYNP_EPS) {
            it->set_synp(val);
         }
         else {
            _synpLst.erase(it);
         }
         return;
      }
   }

   //the connection is not in the list
   //create a new one
   if (val > SYNP_EPS) {
      _synpLst.push_back(SynpConn(val, index(), post, ilayer));
   }
}

//----------------------------------------------------------------------
// update a synaptic connection
// if a connection matching the postsynaptic neuron and layer is found, 
// it updates the synaptic connection, otherwise, it adds a new connection
//----------------------------------------------------------------------
void SpikeSrc::set_synp_conn(const SynpConn& p)
{
   for (vector<SynpConn>::iterator it = _synpLst.begin(); it != _synpLst.end(); ++it) {
      if (it->postsynp() == p.postsynp() && it->layer() == p.layer()) {
         if (p.synp() > SYNP_EPS) {
            it->set_synp(p.synp());
         }
         else {
            _synpLst.erase(it);
         }
         return;
      }
   }

   if (p.synp() > SYNP_EPS) {
      _synpLst.push_back(p);
   }
}

//------------------------------------------------------------------
// delete a synaptic connection
// return true if a matching synaptic connection is found and erased
// return false otherwise
//------------------------------------------------------------------
bool SpikeSrc::del_synp_conn(const TInt& post, const TInt& ilayer)
{
   for (vector<SynpConn>::iterator it = _synpLst.begin(); it != _synpLst.end(); ++it) {
      if (it->postsynp() == post && it->layer() == ilayer) {
         _synpLst.erase(it);
         return true;
      }
   }
   return false;
}

//
//set the const in equation M for a connection
//eqM_const = 1 / (V_rev,pre - V_0,post)
bool SpikeSrc::set_eqM_const(const TReal& val, const TInt& post, const TInt& ilayer)
{
   for (vector<SynpConn>::iterator it = _synpLst.begin(); it != _synpLst.end(); ++it) {
      if (it->postsynp() == post && it->layer() == ilayer) {
         it->set_eqM_const(val);
         return true;
      }
   }
   return false;
}

//
//set the time delay caused by spike propagating across cortical layers 
//from presynapticneuron to the synapses
//time delay caused by horizontal propagation across columns is calculated on the fly
bool SpikeSrc::set_spk_delay(const TInt& val, const TInt& post, const TInt& ilayer)
{
   for (vector<SynpConn>::iterator it = _synpLst.begin(); it != _synpLst.end(); ++it) {
      if (it->postsynp() == post && it->layer() == ilayer) {
         it->set_spk_delay(val);
         return true;
      }
   }
   return false;
}

//
//set the time delay caused by PSP propagating across cortical layers 
//from the synapse to the postsynaptic neuron
//time delay caused by horizontal propagation across columns is calculated on the fly
bool SpikeSrc::set_psp_delay(const TInt& val, const TInt& post, const TInt& ilayer)
{
   for (vector<SynpConn>::iterator it = _synpLst.begin(); it != _synpLst.end(); ++it) {
      if (it->postsynp() == post && it->layer() == ilayer) {
         it->set_psp_delay(val);
         return true;
      }
   }
   return false;
}

//
//initialise spike source
//erase any dummy synaptic connections (whose synapse number is too small)
void SpikeSrc::init()
{
   for (vector<SynpConn>::iterator it = _synpLst.begin(); it != _synpLst.end(); ++it) {
      if (it->is_dummy()) {
         _synpLst.erase(it);
         it = _synpLst.begin();
      }
   }
}

//
//set the ratio of PSP decay during the propagation 
//from synapse to postsybaptic neuron
bool SpikeSrc::set_psp_decay(const TReal& val, const TInt& post, const TInt& ilayer)
{
   for (vector<SynpConn>::iterator it = _synpLst.begin(); it != _synpLst.end(); ++it) {
      if (it->postsynp() == post && it->layer() == ilayer) {
         return it->set_psp_decay(val);
      }
   }
   return false;
}
