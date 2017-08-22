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
#include "layer.h"

using namespace std;

//parameter for layer
const char *Layer::LY_paramName[] = { "UPPER_BOUND", "LOWER_BOUND" };

//initialize the counter
TInt Layer::LY_COUNT = 0;

//-------------------------------------------------
// function: Layer::Layer(const string &xname, const TInt &idx)
//   default constructor
//
//-------------------------------------------------
Layer::Layer(const string& xname, const TInt& idx) :
    _ly_name(xname), _ly_idx(idx), _ly_depth(0), _ly_thickness(0),
    _ly_state(false)
{
    for (TInt ii = 0; ii < LY_PARA_NUM; ++ii) {
        _ly_paramVal[ii] = 0;
        _ly_paramFlg[ii] = false;
    }
    ++LY_COUNT;
}

//--------------------------------------------------
// function: Layer::Layer(const Layer& p)
//   copy constructor
//--------------------------------------------------
Layer::Layer(const Layer &p)
{
    *this = p;

    ++LY_COUNT;
}

//--------------------------------------------------
// function: Layer& Layer::operator=(const Layer& p)
//   assignment function
//--------------------------------------------------
Layer& Layer::operator= (const Layer &p)
{
    if (this != &p) {
        _ly_name = p._ly_name;
        _ly_idx = p._ly_idx;
        _ly_depth = p._ly_depth;
        _ly_thickness = p._ly_thickness;
        _ly_state = p._ly_state;

        std::copy(p._ly_paramVal, p._ly_paramVal + LY_PARA_NUM, _ly_paramVal);
        std::copy(p._ly_paramFlg, p._ly_paramFlg + LY_PARA_NUM, _ly_paramFlg);
    }

    return *this;
}

//--------------------------------------------------
// function: void Layer::set_boundary(const TReal& xlower, const TReal& xupper)
//   Set the boundary of the cortical layer
//
//--------------------------------------------------
void Layer::set_boundary(const TReal &xlower, const TReal &xupper)
{

    assert(xlower > 0 && xupper > 0 && xlower > xupper);

    _ly_paramVal[LY_IDX_LOWER] = xlower;
    _ly_paramFlg[LY_IDX_LOWER] = true;
    _ly_paramVal[LY_IDX_UPPER] = xupper;
    _ly_paramFlg[LY_IDX_UPPER] = true;
    _ly_state = false;
    init();
}

//-------------------------------------------------
// function: bool Layer::set_param(const string& paramName, const TReal& val)
//   Set parameters of the layer
//
//-------------------------------------------------
bool Layer::set_param(const string &paramName, const TReal &val)
{
    if (val < 0) {
        cerr << name() << ": " << msg_invalid_param_value(paramName, val) << endl;
        return false;
    }
    if (paramName == LY_paramName[LY_IDX_LOWER]) {
        _ly_paramVal[LY_IDX_LOWER] = val;
        _ly_paramFlg[LY_IDX_LOWER] = true;
    }
    else if (paramName == LY_paramName[LY_IDX_UPPER]) {
        _ly_paramVal[LY_IDX_UPPER] = val;
        _ly_paramFlg[LY_IDX_UPPER] = true;
    }
    else {
        cerr << name() << ": " << msg_invalid_param_name(paramName) << endl;
        return false;
    }

    _ly_state = false;
    init();
    return true;
}

//------------------------------------------------
// function: bool Layer::is_ready() const
//   Examine whether the layer is ready for running
//
//------------------------------------------------
bool Layer::is_ready() const
{
    if (_ly_state) return true;

    if (!_ly_paramFlg[LY_IDX_LOWER]) {
        cerr << name() << ": " << msg_param_not_set(LY_paramName[LY_IDX_LOWER]) << endl;
        return false;
    }
    if (!_ly_paramFlg[LY_IDX_UPPER]) {
        cerr << name() << ": " << msg_param_not_set(LY_paramName[LY_IDX_UPPER]) << endl;
        return false;
    }
    if (_ly_paramVal[LY_IDX_UPPER] >= _ly_paramVal[LY_IDX_LOWER]) {
        cerr << name() << ": upper boundary is bigger than (or equal to) the lower!" << endl;
        return false;
    }
    if (_ly_idx == MAX_INT_NUM) {
        cerr << name() << ": index has not been set!\n";
        return false;
    }
    cerr << name() << ": the layer has not been initialized!\n";
    return false;
}

//--------------------------------------------------
// function: void Layer::init(void)
//   Initialising the cortical layer for running
//
//--------------------------------------------------
void Layer::init(void)
{
    if (!_ly_paramFlg[LY_IDX_LOWER]) return;
    if (!_ly_paramFlg[LY_IDX_UPPER]) return;
    if (_ly_paramVal[LY_IDX_UPPER] >= _ly_paramVal[LY_IDX_LOWER]) return;

    _ly_depth = (_ly_paramVal[LY_IDX_LOWER] + _ly_paramVal[LY_IDX_UPPER]) / 2;
    _ly_thickness = fabs(_ly_paramVal[LY_IDX_LOWER] - _ly_paramVal[LY_IDX_UPPER]);

    _ly_state = true;
}

//------------------------------------------------
// function: void Layer::swap(Layer& p)
//   Swap the content of two cortical layer objects
//
//------------------------------------------------
void Layer::swap(Layer &p)
{
    if (this == &p) return;

    Layer tmp = p;
    p = (*this);
    (*this) = tmp;
}

//------------------------------------------------
// function: string Layer::print()
//   Return a string of the parameter setting of the layer
//
//------------------------------------------------
string Layer::print(void) const
{
    ostringstream oss;
    oss << "LAYER." << _ly_name << "{" << endl;
    oss << "\t//INDEX = " << _ly_idx << ";" << endl;
    for (TInt ii = 0; ii < LY_PARA_NUM; ++ii) {
        oss << "\t" << LY_paramName[ii] << " = " << _ly_paramVal[ii] << ";" << endl;
    }
    oss << "\t//DEPTH = " << _ly_depth << ";" << endl;
    oss << "\t//THICKNESS = " << _ly_thickness << ";" << endl;
    oss << "};" << endl;

    return oss.str();
}

string Layer::idx2name(const TInt& idx, const vector<Layer>& lyArry)
{
    for (vector<Layer>::const_iterator it = lyArry.begin(); it != lyArry.end(); ++it) {
        if (idx == it->index()) return it->name();
    }
    return LY_DEFAULT_NAME;
}
//------------------------------------------------
// function: void ly_sort(vector<Layer> &LyArry)
//   Sort the cortical layers according to the depth 
//   of the cortical layers in "LyArry"
//
//------------------------------------------------
void ly_sort(vector<Layer>& LyArry)
{
    //sort the layers
    if (!ly_chk_boundary(LyArry)) return;
    if (LyArry.empty()) return;
    for (vector<Layer>::iterator it = LyArry.begin(); it != LyArry.end(); ++it) {
        for (vector<Layer>::iterator it2 = it + 1; it2 != LyArry.end(); ++it2) {
            if (it2->upper() < it->upper()) ly_swap(*it, *it2);
        }
    }
}

//------------------------------------------------
// function: bool ly_chk_boundary(const vector<Layer> &LyArry)
//   Examine the bounday of the cortical layers
//
//   Return true if the cortical layers are not overlapped, 
//   return false otherwise 
//
//------------------------------------------------
bool ly_chk_boundary(const vector<Layer>& LyArry)
{
    if (LyArry.empty() || LyArry.size() == 1) return true;
    for (vector<Layer>::const_iterator it = LyArry.begin(); it != LyArry.end(); ++it) {
        for (vector<Layer>::const_iterator it2 = it + 1; it2 != LyArry.end(); ++it2) {
            if ((it2->lower() > it->lower() && it2->lower() < it->upper()) ||
                (it2->upper() > it->lower() && it2->upper() < it->upper()) ||
                (it2->upper() > it->upper() && it2->lower() < it->lower())) {
                cerr << "The boundary of " << it->name() << " and " << it2->name() << " are overlaped!" << endl;
                return false;
            }
        }
    }
    return true;
}

//------------------------------------------------
// function: bool ly_chk_idx(const vector<Layer> &LyArry)
//   Examine whether the index of cortical layers is consistent
//   with its position in the array
//
//------------------------------------------------
bool ly_chk_idx(const vector<Layer>& LyArry)
{
    if (LyArry.empty()) return true;

    for (TInt idx = 0; idx < LyArry.size(); ++idx) {
        if (LyArry[idx].index() != idx) {
            cerr << LyArry[idx].name() << ": the index value is not consistent with its array index!" << endl;
            cerr << "**array index=" << idx << "; object index=" << LyArry[idx].index() << endl;
            return false;
        }
    }
    return true;
}
