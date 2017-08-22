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
#include "receptor.h"

using namespace std;

const char* Receptor::RCPT_ParamName[RCPT_PARA_NUM] =
   { "G0", "LAMBDA", "DELAY", "TAU_RISE", "TAU_FALL" };
const TReal Receptor::RCPT_ParamMin[RCPT_PARA_NUM] =
   { -1,        0,       0,          0,          0 };
const TReal Receptor::RCPT_ParamMax[RCPT_PARA_NUM] =
   { 1,        1,     1e4,        1e4,        1e4 };

TInt Receptor::RCPT_count = 0;
//---------------------------------------------------
// function: Receptor::Receptor(const string &xname)
//   default constructor
//
//---------------------------------------------------
Receptor::Receptor(const string &xname) :
    _rcpt_psp(),
    _Jconst(0),
    _rcpt_paramVal(RCPT_PARA_NUM, 0),
    _rcpt_paramFlg(RCPT_PARA_NUM, false),
    _rcpt_name(xname),
    _rcpt_type(cNaN),
    _rcpt_state(false),
    _rcpt_idx(RCPT_count)

{
    ++RCPT_count;
};

//----------------------------------------------------
// function: Receptor::Receptor(const Receptor &p)
//   deep copy constructor
//
//----------------------------------------------------
Receptor::Receptor(const Receptor &p) {
    *this = p;

    ++RCPT_count;
};

//Receptor& Receptor::operator= (const Receptor &p)
//{
//   if (this != &p) {
//      _rcpt_name = p._rcpt_name;
//      _rcpt_idx = p._rcpt_idx;
//      _rcpt_type = p._rcpt_type;
//      _rcpt_state = p._rcpt_state;
//      _Jconst = p._Jconst;
//
//      _rcpt_paramFlg = p._rcpt_paramFlg;
//      _rcpt_paramVal = p._rcpt_paramVal;
//
//      //deal with PSP time course
//      _rcpt_psp = p._rcpt_psp;
//
//   }
//   return *this;
//}

bool Receptor::set_type(const TNeur &type)
{
    if (type == cNaN) return false;

    _rcpt_type = type;
    _rcpt_state = false;

    init();
    return true;
}

//-------------------------------------------------
// function: bool Receptor::set_param(const string &paramName, const TReal &val)
//   Set a parameter of the receptor
//
//-------------------------------------------------
bool Receptor::set_param(const string &paramName, const TReal &val)
{
    if (paramName.compare("G0") == 0) {

        if (type() == cEXCIT && val <= 0) {
            cerr << name() << ": " << msg_invalid_param_value(paramName, val) << endl;
            cerr << "  value should be positive!" << endl;
            return false;
        }

        if (type() == cINHIB && val >= 0) {
            cerr << name() << ": " << msg_invalid_param_value(paramName, val) << endl;
            cerr << "  value should be negative!" << endl;
            return false;
        }
    }

    TInt idx;
    for (idx = 0; idx < RCPT_PARA_NUM; ++idx) {
        if (paramName == RCPT_ParamName[idx]) {
            if (val < RCPT_ParamMin[idx] || val > RCPT_ParamMax[idx]) {
                cerr << name() << ": " << msg_invalid_param_value(paramName, val) << endl;
                cerr << "   the allowed range is [" << RCPT_ParamMin[idx] << ", " << RCPT_ParamMax[idx] << "]" << endl;
                return false;
            }
            _rcpt_paramVal[idx] = val;
            _rcpt_paramFlg[idx] = true;
            break;
        }
    }

    if (idx == RCPT_PARA_NUM) {
        cerr << name() << ": " << msg_invalid_param_name(paramName) << endl;
        return false;
    }

    _rcpt_state = false;

    init();

    return true;
}

//------------------------------------------------
// function: void Receptor::swap(Receptor &p)
//   Swap the content of two Receptor objectors
//
//------------------------------------------------
void Receptor::swap(Receptor &p)
{
    if (this != &p) {
        Receptor tmp = *this;
        *this = p;
        p = tmp;
    }
}

//------------------------------------------------
// function: bool Receptor::is_ready() const
//   Examine whether the Receptor is ready for running
//
//------------------------------------------------
bool Receptor::is_ready() const
{
    if (_rcpt_state && (!_rcpt_psp.empty())) return true;

    if (_rcpt_psp.empty()) {
        cerr << name() << ": the psp has not been pre-calculated!\n";
        return false;
    }

    for (TInt ii = 0; ii < RCPT_PARA_NUM; ++ii) {
        if (!_rcpt_paramFlg[ii]) {
            cerr << name() << ": " << msg_param_not_set(RCPT_ParamName[ii]) << endl;
            return false;
        }
    }
    if (_rcpt_type == cNaN) {
        cerr << name() << ": type is not valid!\n";
        return false;
    }

    if (type() == cEXCIT && gain() <= 0) {
        cerr << name() << ": the gain for excitatory receptor must be a positive value!\n";
        return false;
    }

    if (type() == cINHIB && gain() >= 0) {
        cerr << name() << ": the gain for inhibitory receptor must be a negative value!\n";
        return false;
    }
    cerr << name() << ": the receptor has not been initialized!" << endl;
    return false;
}

//-----------------------------------------------
// function: void Receptor::init()
//   Initialising the Receptor for running 
//
//-----------------------------------------------
void Receptor::init()
{
    for (TInt ii = 0; ii < RCPT_PARA_NUM; ++ii) {
        if (!_rcpt_paramFlg[ii]) {
            //cerr<<name()<<": "<<RCPT_ParamName[ii]<<" has not been set!\n";
            return;
        }
    }
    if (_rcpt_type == cNaN) {
        //cerr<<name()<<": type is not valid!\n";
        return;
    }
    if (type() == cEXCIT && gain() <= 0) {
        //cerr<<name()<<": the gain for excitatory receptor must be positive!\n";
        return;
    }

    if (type() == cINHIB && gain() >= 0) {
        //cerr<<name()<<": the gain for inhibitory receptor must be negative!\n";
        return;
    }

    _Jconst = -1. * _rcpt_paramVal[RCPT_IDX_LAMBDA];

    _rcpt_state = true;
}

//------------------------------------------------
// function: TReal Receptor::eqn_R(const TReal &tau) const
//   Equation R, the time course of postsysnaptic potential (PSP)
//
//------------------------------------------------
TReal Receptor::eqn_R(const TReal &tau) const
{
    assert(_rcpt_state);

    if (tau <= _rcpt_paramVal[RCPT_IDX_DELAY]) return 0;

    TReal t_m = _rcpt_paramVal[RCPT_IDX_DELAY] - tau; // = -(t-t_dalay)

    return gain() * (psp_rise() + psp_fall()) * exp(t_m / psp_fall()) \
        * (1.0 - exp(t_m / psp_rise())) / (psp_fall() * psp_fall());
}

//------------------------------------------------
// function: TReal Receptor::eqn_J(const TReal &phi) const
//   Equation J, the adaption of the postsynaptic potential
//
//------------------------------------------------
TReal Receptor::eqn_J(const TReal &phi) const
{
    //return phi * exp(_Jconst*phi);
    TReal x = _Jconst * phi; // x is always negative

    //if (x<-1. || x>0) {
    if (x < -1.0) {
        return phi * exp(x);

    }
    else {
        TReal y = x + 0.5;
        //taylor expansion for small x
        return phi * (6.06530659712633E-01 + y * (6.06530659712633E-01 + y * (3.03265329856317E-01 + \
            y * (1.01088443285439E-01 + y * (2.52721108213597E-02 + y * (5.05442216427195E-03 + \
                y * (8.42403694045324E-04 + y * (1.20343384863618E-04 + y * (1.50429231079522E-05 + \
                    y * 1.67143590088358E-06)))))))));

        //calculation error is smaller than 10ppb for -1<x<0
    }
}

//------------------------------------------------
// function: void Receptor::precalc(const TReal &step_size) 
//   Pre-calculation the time course of the postsynaptic potential (PSP)
//
//------------------------------------------------
void Receptor::precalc(const TReal &step_size)
{
    assert(_rcpt_state);

    //the duration of PSP at least to be 20 msec
    TInt Nstep = static_cast<TInt>((_rcpt_paramVal[RCPT_IDX_DELAY] + 20.) / step_size);

    //PSP time course will terminate at PSP < PSP_EPS
    while ((eqn_R(Nstep*step_size) / gain()) >= PSP_EPS) {
        ++Nstep;
    }

    ++Nstep;

    //cout<<"Receptor "<<name()<<endl;
    //cout<<"Cutoff tau ="<<Nstep*step_size<<", R="<<eqn_R(Nstep*step_size)<<'\n';

    _rcpt_psp.clear();
    _rcpt_psp.resize(Nstep);

    //TReal sum=0;
    for (TInt istep = 0; istep < Nstep; ++istep) {
        _rcpt_psp[istep] = eqn_R((istep + 0.5) * step_size) * step_size;
        //sum += rcpt_psp[istep]/rcpt_rcpt_paramVal[RCPT_IDX_G0];
    }

    //cout<<name()<<": sum="<<sum<<endl;
}

//-------------------------------------------------
// function: string Receptor::print() const
//   Generating a string contain the parameter setting
//
//-------------------------------------------------
string Receptor::print() const
{
    ostringstream oss;
    oss << "RECEPTOR." << _rcpt_name << "{" << endl;
    oss << "\t//INDEX = " << _rcpt_idx << ";" << endl;
    oss << "\tTYPE = " << neur2str(_rcpt_type) << ";" << endl;
    for (TInt ii = 0; ii < RCPT_PARA_NUM; ++ii) {
        oss << "\t" << RCPT_ParamName[ii] << " = " << _rcpt_paramVal[ii] << ";" << endl;
    }

    oss << "\t//Npsp = " << psp_size() << ";" << endl;
    oss << "};" << endl;

    return oss.str();
}
