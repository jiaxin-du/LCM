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
#include "synpconn.h"

using namespace std;

TInt SynpConn::SY_count = 0;

SynpConn::SynpConn() :
   _sy_spk_delay(0),
   _sy_psp_delay(0),
   _sy_weight(0), 
   _sy_eqM_const(0), 
   _sy_psp_decay(1),
   _sy_pre(MAX_INT_NUM), 
   _sy_post(MAX_INT_NUM), 
   _sy_layer(MAX_INT_NUM),
   _sy_synp(0), 
   _sy_state(false)
{
   ++SY_count;
}

SynpConn::SynpConn(const TReal &val, const TInt &pre, const TInt &post, const TInt &ilayer) :
   _sy_spk_delay(0), 
   _sy_psp_delay(0),
   _sy_weight(0),
   _sy_eqM_const(0),
   _sy_psp_decay(1),
   _sy_pre(pre), 
   _sy_post(post), 
   _sy_layer(ilayer), 
   _sy_synp(val),
   _sy_state(false)
{
   ++SY_count;
}

SynpConn::SynpConn(const SynpConn &p)
{
   *this = p;

   ++SY_count;
}

SynpConn::~SynpConn(void)
{
   --SY_count;
}

//SynpConn& SynpConn::operator= (const SynpConn& p)
//{
//   if (this != &p) {
//      _sy_pre = p._sy_pre;
//      _sy_post = p._sy_post;
//      _sy_layer = p._sy_layer;
//      _sy_synp = p._sy_synp;
//      _sy_spk_delay = p._sy_spk_delay;
//      _sy_psp_delay = p._sy_psp_delay;
//      _sy_psp_decay = p._sy_psp_decay;
//      _sy_eqM_const = p._sy_eqM_const;
//      _sy_weight = p._sy_weight;
//      _sy_state = p._sy_state;
//   }
//
//   return *this;
//}

void SynpConn::set(const TReal& val, const TInt& pre, const TInt& post, const TInt& ilayer)
{
   _sy_pre = pre;
   _sy_post = post;
   _sy_layer = ilayer;
   _sy_synp = val;

   _sy_weight = _sy_synp * _sy_psp_decay * _sy_eqM_const;

   _sy_state = true;
}

bool SynpConn::is_ready(void) const
{
   if (_sy_pre == MAX_INT_NUM) {
      cerr << "SynpConn: " << msg_param_not_set ("presynaptic neuron") << endl;
      return false;
   }
   if (_sy_post == MAX_INT_NUM) {
      cerr << "SynpConn: " << msg_param_not_set("postsynaptic neuron") << endl;
      return false;
   }
   if (_sy_layer == MAX_INT_NUM) {
      cerr << "SynpConn: " << msg_param_not_set("synapse layer") << endl;
      return false;
   }
   if (_sy_synp < SYNP_EPS) {
      cerr << "SynpConn: " << msg_param_not_set("synapse number") << endl;
      return false;
   }
   if (_sy_spk_delay == MAX_INT_NUM) {
      cerr << "SynpConn: " << msg_param_not_set("spike delay") << endl;
      return false;
   }
   if (_sy_psp_delay == MAX_INT_NUM) {
      cerr << "SynpConn: " << msg_param_not_set("PSP delay") << endl;
      return false;
   }
   if (_sy_psp_decay < 0) {
      cerr << "SynpConn: " << msg_param_not_set("PSP decay factor") << endl;
      return false;
   }

   return true;
}

void SynpConn::swap(SynpConn &p)
{
   if (this != &p) {
      SynpConn tmp = p;
      p = (*this);
      (*this) = tmp;
   }
}
