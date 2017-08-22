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
#include "stimulator.h"
#include <algorithm>
#include <iomanip>
using namespace std;

const char *Stimulator::ST_ParaName[] = { "AMPLITUDE", "PERIOD", "SOURCE", "MODE", "UPDATE_INTERVAL", "START", "STOP" };

TInt Stimulator::ST_COUNT = 0;

//default constructor
//default parameter: xname = "UNNAMED_STIMULATOR", md = ST_NOISE
Stimulator::Stimulator(const string& xname, const TInt& md) :
    _st_phi_in(),
    _st_phi_out(),
    _st_coeff_in(),
    _st_coeff_out(),
    _st_rand(),
    _st_ampl(0),
    _st_pos(0),
    _st_mode(ST_NOISE),
    _st_elmts(),
    _st_spksrc_id(MAX_UINT_NUM),
    _st_period_win(0),
    _st_start(0),
    _st_stop(0),
    _st_state(false),
    _st_active(false),
    _st_update_intvl(1),
    _st_paramFlg(ST_PARA_NUM, false), /*initial all of them to false*/
    _st_name(xname)
{
    set_mode(md);

    ++ST_COUNT;
}

//copy constructor
Stimulator::Stimulator(const Stimulator& p)
{
    *this = p;

    ++ST_COUNT;
}

//destructor
Stimulator::~Stimulator()
{
    --ST_COUNT;
}

//
//no deep copy needed, use default assignment operator
//
//assignment operator
//Stimulator& Stimulator::operator= (const Stimulator& p)
//{
//   if (this != &p) {
//
//      _st_phi_in = p._st_phi_in;
//      _st_phi_out = p._st_phi_out;
//
//      _st_coeff_in = p._st_coeff_in;
//      _st_coeff_out = p._st_coeff_out;
//
//      _st_ampl = p._st_ampl;
//      _st_mode = p._st_mode;
//
//      _st_elmts = p._st_elmts;
//
//      _st_period_win = p._st_period_win;
//      _st_spksrc_id = p._st_spksrc_id;
//
//      _st_start = p._st_start;
//      _st_stop = p._st_stop;
//
//      _st_state = p._st_state;
//      _st_active = p._st_active;
//
//      _st_name = p._st_name;
//
//      _st_paramFlg = p._st_paramFlg;
//
//      //      st_stream=p.st_stream;
//   }
//
//   return *this;
//}

//swap the content of a stimulator with another one
void Stimulator::swap(Stimulator& p)
{
    if (this != &p) {

        Stimulator tmp = p;
        p = (*this);
        (*this) = tmp;
    }
}

//
void Stimulator::set_mode(const TInt& md)
{
    //ST_NOISE: 0; ST_GAUSS: 1; ST_SYNC_NOISE: 2;
    if ((md == ST_NOISE) || (md == ST_GAUSS) || (md == ST_SYNC_NOISE)) {
        _st_mode = static_cast<StimMode>(md);
    }
    else {
        _st_mode = ST_NOISE;
        cerr << name() << ":  only three modes are supported:" << endl;
        cerr << ST_NOISE << "-asynced white noise; " << ST_GAUSS << "-Gaussian peaks; and " \
            << ST_SYNC_NOISE << "-synced white noise." << endl;
        cerr << "**WARNING: cannot recognize the stimulator mode (mode=" << md \
            << "), it have been changed to " << ST_NOISE << "!" << endl;
    }

    _st_paramFlg[ST_IDX_MODE] = true;
    _st_state = false;
}

//
bool Stimulator::set_source(const TInt& src)
{
    _st_spksrc_id = src;
    _st_paramFlg[ST_IDX_SOURCE] = true;
    return true;
}

//
bool Stimulator::set_param(const string& paramName, const TReal& val)
{
    if (val < 0) {
        cerr << name() << ": " << msg_invalid_param_value(paramName, val) << endl;
    }

    if (paramName == ST_ParaName[ST_IDX_AMPL]) {
        _st_ampl = val;
        _st_paramFlg[ST_IDX_AMPL] = true;
        return true;
    }

    if (paramName == ST_ParaName[ST_IDX_PERIOD]) {
        _st_period_win = static_cast<TInt>(round(val));
        _st_paramFlg[ST_IDX_PERIOD] = true;
        return true;
    }

    if (paramName == ST_ParaName[ST_IDX_SOURCE]) {
        _st_spksrc_id = static_cast<TInt>(round(val));
        _st_paramFlg[ST_IDX_SOURCE] = true;
        return true;
    }

    if (paramName == ST_ParaName[ST_IDX_MODE]) {
        TInt md = static_cast<TInt>(round(val));
        if (md == ST_NOISE || md == ST_GAUSS || md == ST_SYNC_NOISE) {
            set_mode(md);
            return true;
        }
        else {
            cerr << name() << ": " << msg_invalid_param_value(paramName, val) << endl;
            return false;
        }
    }

    if (paramName == ST_ParaName[ST_IDX_INTVL]) {
        _st_update_intvl = static_cast<TInt>(round(val));
        _st_paramFlg[ST_IDX_INTVL] = true;
        return true;
    }

    if (paramName == ST_ParaName[ST_IDX_START]) {
        _st_start = static_cast<TInt>(round(val));
        _st_paramFlg[ST_IDX_START] = true;
        return true;
    }

    if (paramName == ST_ParaName[ST_IDX_STOP]) {
        _st_stop = (static_cast<TInt>(round(val)));
        _st_paramFlg[ST_IDX_STOP] = true;
        return true;
    }

    cerr << name() << ": " << msg_invalid_param_name(paramName) << endl;
    return false;
}

void Stimulator::add_elmt(const TInt& ielmt)
{
    assert(ielmt >= 0);
    //check whether the element is already added
    for (vector<TInt>::iterator it = _st_elmts.begin(); it != _st_elmts.end(); ++it) {
        //if yes, do nothing
        if (*it == ielmt) return;
    }

    //otherwise add the element to the list
    _st_elmts.push_back(ielmt);
}

bool Stimulator::is_ready() const
{
    if (_st_state) return true;

    for (TInt idx = 0; idx < ST_PARA_NUM; ++idx) {
        if (!_st_paramFlg[idx]) {
            cerr << name() << ": " << msg_param_not_set(ST_ParaName[idx]) << endl;
        }
    }

    if (_st_elmts.empty()) {
        cout << name() << ": no element attached!" << endl;
        return false;
    }

    cerr << "Stimulator " << name() << " has not been initialized!" << endl;
    return false;
}

//initialisation
void Stimulator::init()
{
    //check parameter settings
    for (TInt idx = 0; idx < ST_PARA_NUM; idx++) {
        if (!_st_paramFlg[idx]) return;
    }

    //check attched elements
    if (_st_elmts.empty()) {
        cout << "stimulator " << name() << ": no element attached!\n";
        return;
    }

    if (!Rand::is_ready()) {
        rand_init(0);
    }
    //sort elements
    std::sort(_st_elmts.begin(), _st_elmts.end());

    if (mode() == ST_NOISE || mode() == ST_SYNC_NOISE) {
        //calculate the coefficient of the filter
        //first parameter is the f_cut/f_sample
        calc_3rd_butter_coeff(TReal(_st_update_intvl)/TReal(_st_period_win), _st_coeff_in, _st_coeff_out);

        if ((_st_coeff_in.size() != BUTTER_COEFF_NUM) ||
            (_st_coeff_out.size() != BUTTER_COEFF_NUM)) {
            cerr << name() << ": filter coefficient number is not right!" << endl;
            return;
        }
    }


    TReal devn, peak;
    //initialise phi sequence
    switch (mode()) {

        //asynced noise
    case (ST_NOISE):
        //index calculation macro for _st_phi_in and _st_phi_out
        //icoeff = 0 .. BUTTER_COEFF_NUM-1;
        //ielmt = 1 .. _st_elmts.size()
#define ST_PHI_IDX(ielmt, icoeff) (ielmt * BUTTER_COEFF_NUM + icoeff )

        _st_phi_in.set_default(0.);
        _st_phi_out.set_default(0.);

        _st_phi_in.resize(_st_elmts.size() * BUTTER_COEFF_NUM);
        _st_phi_out.resize(_st_elmts.size() * BUTTER_COEFF_NUM);

        _st_rand.resize(_st_elmts.size());

        for (TInt inum=0; inum < _st_elmts.size(); ++inum){
            _st_rand[inum].set_seed(static_cast<unsigned int>(rand_rndm() * 4294967295.));
        }

        //run this section in parallel if not already in a parallel section
        for (TInt inum = 0; inum != _st_phi_in.size(); ++inum) {
            //fill _st_phi_in with random number 
            // and _st_phi_out with zero
            _st_phi_in[inum] = Rand::gauss(_st_rand[inum/BUTTER_COEFF_NUM], 0., _st_ampl);
            _st_phi_out[inum] = 0;
        }

        for (TInt idx = 0; idx < 5; ++idx) {
            advance();
        }

        _st_pos = _st_update_intvl;

        _st_state = true;

        break;

    case (ST_GAUSS):
        _st_phi_in.clear();
        _st_phi_out.resize(_st_period_win);

        devn = _st_period_win / 8.;

        peak = 0.5 * static_cast<TReal>(_st_period_win);

        for (TInt idx = 0; idx < _st_period_win; idx++) {
            _st_phi_out[idx] = _st_ampl * exp((idx - peak)*(idx - peak) / (-2.*devn*devn));
        }

        _st_pos = 0;

        _st_state = true;

        break;

    case (ST_SYNC_NOISE):
        _st_phi_in.set_default(0.);
        _st_phi_out.set_default(0.);

        _st_phi_in.resize(BUTTER_COEFF_NUM);
        _st_phi_out.resize(BUTTER_COEFF_NUM);

        _st_rand.resize(1);

        _st_rand[0].set_seed(static_cast<unsigned int>(rand_rndm() * 4294967295.));

        for (TInt inum = 0; inum != _st_phi_in.size(); ++inum) {
            _st_phi_in[inum] = Rand::gauss(_st_rand[0], 0., _st_ampl);
            _st_phi_out[inum] = 0;
        }

        for (TInt idx = 0; idx < 10; ++idx) {
            advance();
        }

        _st_pos = _st_update_intvl;

        _st_state = true;

        break;
    }
}

TReal Stimulator::generate(const TInt &ielmt)
{
    assert(_st_state);
    assert(ielmt < _st_elmts.size());

    if (!_st_active) return 0.;

    if (mode() == ST_GAUSS) {
        return _st_phi_out[_st_pos];
    }

    if (mode() == ST_SYNC_NOISE) {
        return _st_phi_out[0];
    }

    return _st_phi_out[ST_PHI_IDX(ielmt, 0)];
}

void Stimulator::advance()
{
    /*
    //this check-up is performed in the class ExSource
    if(istep < _st_start || istep > _st_stop) {
        deactivate();
        return;
    }
    */
    if (mode() == ST_GAUSS) {
        ++_st_pos;
        if (_st_pos == _st_period_win) {
            _st_pos = 0;
        }
        return;
    }

    if (_st_pos != 0) {
        --_st_pos;
        return;
    }

    TInt pos;
    TReal phi;
    if (mode() == ST_NOISE) {
        //move numbers forward
        _st_phi_in.step_forward();
        _st_phi_out.step_forward();

#ifdef _OPENMP   
#pragma omp parallel for if(omp_in_parallel() == 0) private(phi, pos)
        for (TInt ielmt = 0; ielmt < _st_elmts.size(); ++ielmt) {
#else
        for (TInt ielmt = 0; ielmt < _st_elmts.size(); ++ielmt) {
#endif
            pos = ST_PHI_IDX(ielmt, 0);

            //add a new number
            _st_phi_in[pos] = Rand::gauss(_st_rand[ielmt], 0., _st_ampl);
            _st_phi_out[pos] = 0.;

            phi = _st_phi_in[pos] * _st_coeff_in[0];

            for (TInt idx = 1; idx < BUTTER_COEFF_NUM; ++idx) {
                phi += _st_phi_in[pos + idx] * _st_coeff_in[idx];
                phi -= _st_phi_out[pos + idx] * _st_coeff_out[idx];
            }
            _st_phi_out[pos] = (phi >= 0.) ? phi : 0.;
        }

        _st_pos = _st_update_intvl;
        return;
    }
    else {
        //move everything forward
        _st_phi_in.step_forward();
        _st_phi_out.step_forward();

        //generate a random number
        _st_phi_in[0] = Rand::gauss(_st_rand[0], 0., _st_ampl);

        //calculate the new phi value
        phi = _st_phi_in[0] * _st_coeff_in[0];

        for (int idx = 1; idx != BUTTER_COEFF_NUM; ++idx) {
            phi += _st_phi_in[idx] * _st_coeff_in[idx];
            phi -= _st_phi_out[idx] * _st_coeff_out[idx];
        }

        _st_phi_out[0] = (phi >= 0.) ? phi : 0.;
        _st_pos = _st_update_intvl;
    }
}

string Stimulator::print(const string& srcname, const TReal& step_size) const
{
    ostringstream oss;
    oss << "STIM." << name() << "{" << endl;
    oss << "\tmode = " << mode() << "; //" << ST_NOISE << "-white noise; " << ST_GAUSS << "-GAUSSIAN peaks; " \
        << ST_SYNC_NOISE << "-synchronized white noise; " << endl;
    oss << "\t" << ST_ParaName[ST_IDX_AMPL] << " = " << _st_ampl << ";" << endl;

    oss << "\t" << ST_ParaName[ST_IDX_PERIOD] << " = " << _st_period_win*step_size
        << "; // == " << _st_period_win << " steps " << endl;

    oss << "\t" << ST_ParaName[ST_IDX_INTVL] << " = " << _st_update_intvl*step_size
        << "; // == " << _st_update_intvl << " steps " << endl;

    oss << "\t//butter low-pass filter coefficient " << endl;

    oss << "\t// ID: ";
    for(TInt idx = 0; idx < BUTTER_COEFF_NUM; ++idx){
      oss <<std::setw(8)<<idx<<" ";
    }
    oss<<endl;
    
    oss << "\t// IN: ";
    for(TInt idx = 0; idx < BUTTER_COEFF_NUM; ++idx){
      oss <<std::setw(8)<<_st_coeff_in[idx]<<" ";
    }
    oss<<endl;
    
    oss << "\t//OUT: ";
    for(TInt idx = 0; idx < BUTTER_COEFF_NUM; ++idx){
      oss <<std::setw(8)<<_st_coeff_out[idx]<<" ";
    }
    oss<<endl;

    if (srcname.empty()) {
        oss << "\t//" << ST_ParaName[ST_IDX_SOURCE] << " = " << _st_spksrc_id << ";" << endl;
    }
    else {
        oss << "\t" << ST_ParaName[ST_IDX_SOURCE] << " = " << srcname << ";" << endl;
    }

    if (step_size != 0) {
        oss << "\t" << ST_ParaName[ST_IDX_START] << " = " << _st_start*step_size
            << "; // == " << _st_start << "-th step" << endl;
        oss << "\t" << ST_ParaName[ST_IDX_STOP] << " = " << _st_stop*step_size
            << "; // == " << _st_stop << "-th step" << endl;
    }
    else {
        oss << "\t//WARNING, the step_size is not set," << endl
            << "\t//the values of 'start' and 'stop' are in steps instead of times." << endl;
        oss << "\t" << ST_ParaName[ST_IDX_START] << " = " << _st_start << ";" << endl;
        oss << "\t" << ST_ParaName[ST_IDX_STOP] << " = " << _st_stop << ";" << endl;
    }
    oss << "\tELEMENT = {" << nums2str(_st_elmts) << "};\n";
    oss << "\t//st_elmt number = " << _st_elmts.size() << ";" << endl;
    oss << "\t//st_phi_in size = " << _st_phi_in.size() << ";" << endl;
    oss << "\t//st_phi_out size = " << _st_phi_out.size() << ";" << endl;
    oss << "};";

    return oss.str();
}

