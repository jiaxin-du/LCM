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
#include "exsource.h"

#include <algorithm>

using namespace std;

TInt ExSource::ES_cnt = 0;
TInt ExSource::ES_idx_base = 0;

ExSource::ExSource(const string &xname, const TInt &idx) :
    SpikeSrc(xname, idx), _elmt_stim(NULL), _Nact_stim(0), _chk_pnt(0), _es_state(false)
{
    if (ES_cnt == 0)
        ES_idx_base = SS_cnt;

    ++ES_cnt;
}

ExSource::ExSource(const ExSource &p) :
    _elmt_stim(NULL)
{
    *this = p;

    ++ES_cnt;
}

ExSource::~ExSource()
{
    if (_elmt_stim != NULL) {
        for (TInt ielmt = 0; ielmt < _es_elmt.size(); ++ielmt) {
            delete[] _elmt_stim[ielmt];
        }
        delete[] _elmt_stim;
    }
    --ES_cnt;
}

ExSource& ExSource::operator= (const ExSource&p)
{
    if (this != &p) {

        //base class assignment
        SpikeSrc::operator=(p);

        //vector member 
        _es_stim = p._es_stim;
        _es_elmt = p._es_elmt;

        //simple data member
        _Nact_stim = p._Nact_stim;
        _chk_pnt = p._chk_pnt;
        _es_state = p._es_state;

        if (_elmt_stim != NULL) {
            for (TInt ielmt = 0; ielmt < _es_elmt.size(); ++ielmt) {
                delete[] _elmt_stim[ielmt];
            }
            delete[] _elmt_stim;
            _elmt_stim = NULL;
        }

        if (p._elmt_stim != NULL) {
            try {
                _elmt_stim = new TInt*[_es_elmt.size()];
            }
            catch (bad_alloc& e) {
                cerr << MEMORY_ERROR << endl << e.what() << endl;
                fflush(stderr);
                exit(-1);
            }
            for (TInt ielmt = 0; ielmt < _es_elmt.size(); ++ielmt) {
                try {
                    _elmt_stim[ielmt] = new TInt[_es_stim.size()];
                }
                catch (bad_alloc& e) {
                    cerr << MEMORY_ERROR << endl << e.what() << endl;
                    fflush(stderr);
                    exit(-1);
                }
                std::copy(p._elmt_stim[ielmt], p._elmt_stim[ielmt] + _es_stim.size(), _elmt_stim[ielmt]);
            }
        }
    }

    return *this;
}

//
//bool ExSource::set_param(const string &paramName, const string &paramVal)
//{
//   //this function is reserved for further use
//   cerr << "There is no parameter need to be set in the object." << endl;
//   cerr << "paramName=" << paramName << ", paramVal" << paramVal << "." << endl;
//   return true;
//}

void ExSource::init()
{
    SpikeSrc::init();

    if (_es_stim.empty()) {
        cerr << "ExSource::init: no stimulator attached to source " << name()
            << ". In '" << __FILE__ << "' line " << __LINE__ << "." << endl;
        exit(-1);
    }

    //TInt max_elmt=0; //the largest element number

    for (vector<Stimulator>::iterator it = _es_stim.begin(); it != _es_stim.end(); ++it) {
        it->init();
        if (!it->is_ready()) {
            cerr << msg_object_not_ready(it->name()) << endl;
            exit(-1);
        }
    }

    //build a list of the elements projected to by the external source
    _es_elmt.clear();

    for (vector<Stimulator>::const_iterator st_it = _es_stim.begin(); st_it != _es_stim.end(); ++st_it) {
        for (vector<TInt>::const_iterator el_it = st_it->elmt_list().begin(); el_it != st_it->elmt_list().end(); ++el_it) {
            if (find(_es_elmt.begin(), _es_elmt.end(), *el_it) == _es_elmt.end())
                _es_elmt.push_back(*el_it);
        }
    }

    std::sort(_es_elmt.begin(), _es_elmt.end());

    if (_elmt_stim != NULL) {
        for (size_t ielmt = 0; ielmt < _es_elmt.size(); ++ielmt) {
            delete[] _elmt_stim[ielmt];
        }
        delete[] _elmt_stim;
    }

    //allocate memory
    try {
        _elmt_stim = new TInt*[_es_elmt.size()];
    }
    catch (bad_alloc& e) {
        cerr << MEMORY_ERROR << endl << e.what() << endl;
        fflush(stderr);
        exit(-1);
    }

    for (size_t ielmt = 0; ielmt < _es_elmt.size(); ++ielmt) {
        try {
            _elmt_stim[ielmt] = new TInt[_es_stim.size()];
        }
        catch (bad_alloc& e) {
            cerr << MEMORY_ERROR << endl << e.what() << endl;
            fflush(stderr);
            exit(-1);
        }

        //_elmt_stim[ielmt][ist]=-1 indicates stimulator ist does not apply to element ielmt
        for (size_t ist = 0; ist < _es_stim.size(); ++ist) {
            _elmt_stim[ielmt][ist] = -1;
        }
    }

    //connect element in exsource to that in stimulators
    for (size_t ist = 0; ist < _es_stim.size(); ++ist) {
        for (size_t ielmt = 0; ielmt < _es_stim[ist].elmt_list().size(); ++ielmt) {
            for (size_t jelmt = 0; jelmt < _es_elmt.size(); ++jelmt) {
                if (_es_stim[ist].elmt_list().at(ielmt) == _es_elmt[jelmt]) {
                    _elmt_stim[jelmt][ist] = ielmt;
                    break;
                }
            }
        }
    }

    check(0);
    _es_state = true;
}

void ExSource::advance()
{
    for (vector<Stimulator>::iterator it = _es_stim.begin(); it != _es_stim.end(); ++it) {
        if (it->is_active()) it->advance();
    }
}


TReal ExSource::generate(const TInt &idx)
{
    assert(idx < _es_elmt.size());

    TReal phi = 0.;
    TInt *pos = _elmt_stim[idx];
    for (vector<Stimulator>::iterator it = _es_stim.begin(); it != _es_stim.end(); (++it), (++pos)) {
        if (!it->is_active() || (*pos) == -1) continue;
        phi += it->generate(*pos);
    }

    //if(phi<0.) return 0.; //not neccessary

    //if(phi > 0) std::cout<<"elmt no="<<idx<<", phi="<<phi<<std::endl;

    return phi;
}

string ExSource::print(const TReal &step_size) const
{
    ostringstream ss;
    ss << "//SOURCE." << _src_name << "{" << endl;
    ss << "//\tINDEX = " << _src_idx << ";" << endl;
    if (!_es_stim.empty()) {
        ss << "//\tSTIMULATOR = ";
        for (vector<Stimulator>::const_iterator it = _es_stim.begin(); it != _es_stim.end(); ++it) {
            if (it != _es_stim.begin())
                ss << ", ";
            ss << it->name();
        }
        ss << ";" << endl;
        if (!_es_elmt.empty()) {
            ss << "//\tELEMENT = {" << nums2str(_es_elmt) << "};" << endl;
        }
        ss << "//};" << endl;
        for (vector<Stimulator>::const_iterator it = _es_stim.begin(); it != _es_stim.end(); ++it) {
            ss << endl;
            ss << it->print(_src_name, step_size) << endl;
        }
    }
    else {
        ss << "//There is no stimulator attached to this source!" << endl;
        ss << "//};" << endl;
    }

    return ss.str();
}

TInt ExSource::check(const TInt &c_step)
{
    _Nact_stim = 0;
    _chk_pnt = MAX_INT_NUM;

    for (vector<Stimulator>::iterator it = _es_stim.begin(); it != _es_stim.end(); ++it) {
        if (c_step < it->start_step()) {
            it->deactivate();
            if (it->start_step() < _chk_pnt) _chk_pnt = it->start_step();
        }
        else if (c_step < it->stop_step()) {
            it->activate();
            if (it->stop_step() < _chk_pnt) _chk_pnt = it->stop_step();
            ++_Nact_stim;
        }
        else {
            it->deactivate();
        }
        //cout<<it->name()<<" ("<< (it->is_active()? std::string("active") : std::string("inactive"))<<"): "<<it->start_step()<<"-"<<it->stop_step()<<", Check_point="<<_chk_pnt<<endl;
    }

    return _chk_pnt;
}

bool es_check_idx(const vector<ExSource> &EsArry)
{
    if (EsArry.empty()) return true;
    for (TInt idx = 0; idx < EsArry.size(); ++idx) {
        if (EsArry[idx].index() != idx + ExSource::idx_base()) {
            cerr << "Neuron group " << EsArry[idx].name()
                << ": the index value is not consistent with the array index!" << endl;
            cerr << "**array index=" << idx << "; object index=" << EsArry[idx].index()
                << "; index base=" << ExSource::idx_base() << endl;
            return false;
        }
    }
    return true;
}
