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
#include "lcm.h"

using namespace std;

//LCM global parameter
const char *LCM::LCM_paramName[] = { "SIZE", "SIDE_GRID", "TIME_STEP", "SIMU_TIME" };
const TReal LCM::LCM_paramMin[] = { 0,           2,           0,           0 };
const TReal LCM::LCM_paramMax[] = { 1000,         100,          10,     1000000 };

//--------------------------------------------------
// function: LCM::LCM
//   default constructor
//--------------------------------------------------
LCM::LCM() :
    gSpk_delay(NULL), gSynp_pct(NULL), gElmt_num(0), gNG_num(0), gGrid_row(0),
    gROI_size(0), gTotal_time(0), gStep_size(0), gElmt_size(0), gInv_step(0),
    gLayer_num(0), gRcpt_type(0), gStim_num(0), _l_state(false),
    _l_neur_state(false), _l_layer_state(false), _l_exsrc_state(false)
{
    for (TInt ii = 0; ii < LCM_PARA_NUM; ++ii) {
        _lcm_paramFlg[ii] = false;
    }

    //the following strings cannot be used as object name
    gObj_name_lst.insert("GLOBAL");
    gObj_name_lst.insert("LAYER");
    gObj_name_lst.insert("NEURON");
    gObj_name_lst.insert("RECEPTOR");
    gObj_name_lst.insert("SOURCE");

}

//--------------------------------------------------
// function LCM::~LCM(void)
//    default destructor
//
//--------------------------------------------------
LCM::~LCM(void)
{
    if (gSpk_delay != NULL) {
        for (TInt ineur = 0; ineur < gNG_num; ++ineur) {
            delete[] gSpk_delay[ineur];
        }
        delete[] gSpk_delay;
    }

    if (gSynp_pct != NULL) {
        for (TInt ineur = 0; ineur < gNG_num; ++ineur) {
            delete[] gSynp_pct[ineur];
        }
        delete[] gSynp_pct;
    }
}

//--------------------------------------------------
// function: bool LCM::check_name(const string& xname) const
//   Return true if xname is a legal object name, return false otherwise
//   A object name must be unique, and  it cannot contain ., ', \, and "
//--------------------------------------------------
bool LCM::check_name(const string& xname) const
{
    if (xname.empty()) {
        cerr << __FUNCTION__ << ": object name cannot be empty! " << _FILE_LINE_ << endl;
        return false;
    }
    if (xname.find_first_of(".'\\\"") != string::npos) {
        cerr << __FUNCTION__ << ": object name cannot contain '.', ''', '\\' and '\"' (name='" << xname << "')! " << _FILE_LINE_ << endl;
        return false;
    }
    if (gObj_name_lst.count(xname) != 0) {
        cerr << __FUNCTION__ << ": object " << xname << " already exist! " << _FILE_LINE_ << endl;
        return false;
    }
    return true;
}

//-------------------------------------------------
// function: bool LCM::add_neur(const string& xname)
//   Add a new neuron group to the model
//
//   Return true if a neuron group is successfully added
//   Return false if the name is illegal or a object withe the name already exist
//-------------------------------------------------
bool LCM::add_neur(const string& xname)
{
    //xname = strtrim(xname);
    if (!check_name(xname)) {
        return false;
    }

    if (_l_neur_state) {
        cerr << __FUNCTION__ << ": cannot change neuron groups because the object indices are already in use! " << _FILE_LINE_ << endl;
        return false;
    }

    //Check whether gExSrc is empty, 
    //neuron groups and external source use the name counting index, NeurGrp::src_count()
    //neuron groups should be added before external source to make sure their index is not mixed
    if (!gExSrc.empty()) {
        cerr << __FUNCTION__ << ": neuron group cannot be added while external source list is not empty! " << _FILE_LINE_ << endl;
        cerr << "  Neuron group should be added before external sources!" << endl;
        return false;
    }

    gNeur.push_back(NeurGrp(xname, SpikeSrc::src_count())); // do not use NeurGrp::ng_count() here
    gObj_name_lst.insert(xname);
    return true;
}

//-------------------------------------------------
// function: bool LCM::add_layer(const string& xname)
//   Add a new layer to the model
//
//   Return true if a layer is successfully added
//   Return false if the name is illegal or a object withe the name already exist
//-------------------------------------------------
bool LCM::add_layer(const string& xname)
{
    //xname = strtrim(xname);
    if (!check_name(xname)) {
        return false;
    }

    if (_l_layer_state) {
        cerr << __FUNCTION__ << ": cannot add layer since the layer list indices are already in use! " << _FILE_LINE_ << endl;
        return false;
    }

    gLayer.push_back(Layer(xname, Layer::count()));
    gObj_name_lst.insert(xname);
    return true;
}

//-------------------------------------------------
// function: bool LCM::add_rcpt(const string& xname)
//   Add a new receptor type to the model
//
//   Return true if a receptor is successfully added
//   Return false if the name is illegal or a object withe the name already exist
//-------------------------------------------------
bool LCM::add_rcpt(const string& xname)
{
    //xname = strtrim(xname);
    if (!check_name(xname)) {
        return false;
    }

    gRcpt.push_back(Receptor(xname));
    gObj_name_lst.insert(xname);
    return true;
}

//-------------------------------------------------
// function: bool LCM::add_stim(const string& xname)
//   Add a new stimulator to the model
//   PS: a stimulator must be manually attached to a external source
//-------------------------------------------------
bool LCM::add_stim(const string& xname)
{
    //xname= strtrim(xname);
    if (!check_name(xname)) {
        return false;
    }

    gStim.push_back(Stimulator(xname));
    gObj_name_lst.insert(xname);
    return true;
}

//------------------------------------------------
// function: bool LCM::add_exsrc(const string& xname)
//   Add a new external source to the model
//------------------------------------------------
bool LCM::add_exsrc(const string& xname)
{
    //xname = strtrim(xname);
    if (!check_name(xname)) {
        return false;
    }

    if (_l_exsrc_state) {
        cerr << __FUNCTION__ << ": cannot add external source since the object indices are already in use! " << _FILE_LINE_ << endl;
        return false;
    }

    gExSrc.push_back(ExSource(xname, ExSource::src_count()));
    gObj_name_lst.insert(xname);
    return true;
}

//------------------------------------------------
// function: vector<NeurGrp>::size_type LCM::idx_neuron(const string& xname)
//   Returnt the index of a neuron group in the list gNeur
//------------------------------------------------
vector<NeurGrp>::size_type LCM::idx_neuron(const string& xname)
{
    //xname=strtrim(xname);
    for (vector<NeurGrp>::iterator it = gNeur.begin(); it != gNeur.end(); ++it) {
        if (it->name() == xname) {
            return it->index();
        }
    }
    _l_neur_state = true;
    return MAX_INT_NUM;
}

//-------------------------------------------------
// function: vector<ExSource>::size_type LCM::idx_exsrc(const string& xname)
//   Return the index of a external source in the list gExSrc
//-------------------------------------------------
vector<ExSource>::size_type LCM::idx_exsrc(const string& xname)
{
    //xname=strtrim(xname);
    for (vector<ExSource>::iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
        if (it->name() == xname) {
            return it->index();
        }
    }
    _l_exsrc_state = true;
    return MAX_INT_NUM;
}

//-------------------------------------------------
// function: vector<Layer>::size_type LCM::idx_layer(const string& xname)
//   Return the index of a layer in the list gLayer
//-------------------------------------------------
vector<Layer>::size_type LCM::idx_layer(const string& xname)
{
    //xname=strtrim(xname);
    for (vector<Layer>::iterator it = gLayer.begin(); it != gLayer.end(); ++it) {
        if (it->name() == xname) {
            return it->index();
        }
    }
    _l_layer_state = true;
    return MAX_INT_NUM;
}

//-------------------------------------------------
// function: bool LCM::set_neur_param(NeurGrp &obj, string paramName, string paramVal)
//   Set the parameter of a neuron group with a value 
//
//   Return true if the parameter is successfully set
//   Return false if the parameter is not found or the value is of a wrong type
//-------------------------------------------------
bool LCM::set_neur_param(NeurGrp& obj, string paramName, string paramVal)
{
    if (paramName == "LAYER") {
        // laminar location
        TInt ly_pos = idx_layer(paramVal);
        if (ly_pos == MAX_INT_NUM) {
            cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
            return false;
        }
        return obj.set_layer(ly_pos);

    }
    else if (paramName == "TYPE") {
        // neuron type
        TNeur neur = str2neur(paramVal);
        if (neur == cNaN) {
            cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
            cerr << "   neuron type can only be EXCIT or INHIB !" << endl;
            return false;
        }
        return obj.set_type(neur);

    }
    else {
        TReal val;
        if (!str2float(paramVal, val)) {
            cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
            return false;
        }
        return obj.set_param(paramName, val);
    }
}

//--------------------------------------------------
// function: bool LCM::set_layer_param(Layer &obj, string paramName, string paramVal)
//   Set the parameter of a layer with a value 
//
//   Return true if the parameter is successfully set
//   Return false if the parameter is not found or the value is of a wrong type
//--------------------------------------------------
bool LCM::set_layer_param(Layer& obj, string paramName, string paramVal)
{
    TReal val;
    if (!str2float(paramVal, val)) {
        cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
        return false;
    }
    return obj.set_param(paramName, val);
}

//--------------------------------------------------
// function: bool LCM::set_simu_param(string paramName, string paramVal)
//   Set a global parameter with a value 
//
//   Return true if the parameter is successfully set
//   Return false if the parameter is not found or the value is of a wrong type
//--------------------------------------------------
bool LCM::set_simu_param(string paramName, string paramVal)
{
    TInt idx;

    for (idx = 0; idx != LCM_PARA_NUM; ++idx) {
        if (paramName == LCM_paramName[idx])
            break;
    }

    if (idx == LCM_PARA_NUM) {
        cerr << "SIMU: " << msg_invalid_param_name(paramName) << endl;
        return false;
    }

    TReal val;
    if (!str2float(paramVal, val)) {
        cerr << "SIMU: " << msg_invalid_param_value(paramName, paramVal) << endl;
        return false;
    }

    if (val<LCM_paramMin[idx] || val> LCM_paramMax[idx]) {
        cerr << "SIMU: " << msg_invalid_param_value(paramName, paramVal) << endl;
        cerr << "   the value should be in the range of [" << LCM_paramMin[idx]
            << ", " << LCM_paramMin[idx] << "]." << endl;
        return false;
    }

    switch (idx) {

    case (LCM_IDX_GRID_ROW):
        gGrid_row = static_cast<TInt>(val + 0.5);
        _lcm_paramFlg[LCM_IDX_GRID_ROW] = true;
        gElmt_num = gGrid_row * gGrid_row;
        if (_lcm_paramFlg[LCM_IDX_GRID_SIZE])
            gElmt_size = gROI_size / static_cast<TReal>(gGrid_row);

        return true;

    case (LCM_IDX_GRID_SIZE):
        gROI_size = val;
        _lcm_paramFlg[LCM_IDX_GRID_SIZE] = true;

        if (_lcm_paramFlg[LCM_IDX_GRID_ROW])
            gElmt_size = gROI_size / static_cast<TReal>(gGrid_row);
        return true;

    case (LCM_IDX_SIMU_TIME):
        gTotal_time = val;
        _lcm_paramFlg[LCM_IDX_SIMU_TIME] = true;
        if (_lcm_paramFlg[LCM_IDX_TIME_STEP])
            gTotal_step = static_cast<TInt>(gTotal_time / gStep_size);
        return true;

    case (LCM_IDX_TIME_STEP):
        gStep_size = val;
        _lcm_paramFlg[LCM_IDX_TIME_STEP] = true;
        gInv_step = 1.0 / gStep_size;
        if (_lcm_paramFlg[LCM_IDX_SIMU_TIME])
            gTotal_step = static_cast<TInt>(gTotal_time / gStep_size);
        return true;

    default:
        return false;
    }
}

//----------------------------------------------------------
// function: bool LCM::set_rcpt_param(Receptor &obj, string paramName, string paramVal)
//   Set a parameter of a receptor with a value 
//
//   Return true if the parameter is successfully set
//   Return false if the parameter is not found or the value is of a wrong type
//------------------------------------------------------------
bool LCM::set_rcpt_param(Receptor& obj, string paramName, string paramVal)
{

    if (paramName == "TYPE") {
        TNeur neur = str2neur(paramVal);
        if (neur == cNaN) {
            cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
            cerr << "   the receptor type can only be EXCIT or INHIB !" << endl;
            return false;
        }
        return obj.set_type(neur);
    }
    else {
        TReal val;
        if (!str2float(paramVal, val)) {
            cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
            return false;
        }
        return obj.set_param(paramName, val);
    }
}

//------------------------------------------------------
// function: bool LCM::set_stim_param(Stimulator &obj, string paramName, string paramVal)
//   Set a parameter of a stimulator with a value
//
//   Return true if the parameter is successfully set
//   Return false if the parameter is not found or the value is of a wrong type
//-------------------------------------------------------
bool LCM::set_stim_param(Stimulator& obj, string paramName, string paramVal)
{
    if (paramName == "ELEMENT") {
        vector<string> parts;
        //cout<<obj.name()<<"."<<paramName<<"="<<paramVal<<endl;
        if (paramVal[0] != '{' || paramVal[paramVal.size() - 1] != '}') {
            cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
            cerr << "   the value must be enclosed by paired {} !" << endl;
            return false;
        }
        //delete the paired '{}'
        paramVal = paramVal.substr(1, paramVal.size() - 2);
        //split it to parts
        strsplit(paramVal, ",", parts);
        for (vector<string>::iterator it = parts.begin(); it != parts.end(); ++it) {
            //cout<<*it<<endl;
            vector<TInt> nums;
            if (!str2nums(*it, nums)) {
                cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
                return false;
            }
            for (vector<TInt>::iterator it2 = nums.begin(); it2 != nums.end(); ++it2) {
                obj.add_elmt(*it2);
            }
        }
        return true;
    }

    //set the external source the stimulator is attached to
    if (paramName == Stimulator::ST_ParaName[ST_IDX_SOURCE]) {
        TInt src = idx_exsrc(paramVal);
        if (src == MAX_INT_NUM) {
            cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
            cerr << "   the source is not in the source list!" << endl;
            return false;
        }
        return obj.set_source(src);
    }

    //set mode
    if (paramName == Stimulator::ST_ParaName[ST_IDX_MODE]) {
        TInt  u_val;
        if (!str2uint(paramVal, u_val)) {
            cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
            return false;
        }
        obj.set_mode(u_val);
        return true;
    }

    //convert the value from string to real number
    TReal val;
    if (!str2float(paramVal, val)) {
        cerr << obj.name() << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
        return false;
    }

    //convert the value from time to step
    if (paramName == Stimulator::ST_ParaName[ST_IDX_PERIOD] || paramName == Stimulator::ST_ParaName[ST_IDX_INTVL] ||
        paramName == Stimulator::ST_ParaName[ST_IDX_START] || paramName == Stimulator::ST_ParaName[ST_IDX_STOP]) {
        if (!_lcm_paramFlg[LCM_IDX_TIME_STEP]) {
            cerr << obj.name() << ": " << paramName << " cannot be set when TIME_STEP is unset!" << endl;
            return false;
        }
        return obj.set_param(paramName, val / gStep_size);
    }
    /*
    if (paramName == Stimulator::ST_ParaName[ST_IDX_AMPL]){
       if (!_lcm_paramFlg[LCM_IDX_TIME_STEP]){
          cerr << "ERROR! STIM." << obj.name() << "." << paramName << ": " << " the parameter cannot be set when TIME_STEP is unset!"<<endl;
          return false;
       }
       return obj.set_param(paramName, val * gStep_size);
    }
    */
    return obj.set_param(paramName, val);
}

//------------------------------------------------------
// function: bool LCM::set_synp_param(string pre, string post, string paramVal)
//   Set the value of synaptic connection from neuron group "pre" to neuron group "post" 
//
//   pre: the name of pre-synaptic neuron group
//   post: the name of post-synaptic neuron group
//   paramVal: the array of synaptic connections, separated with "," 
//
//   Return true if the parameter is successfully set
//   Return false if the parameter is not found or the value is of a wrong type
//-------------------------------------------------------
bool LCM::set_synp_conn(string pre, string post, string paramVal)
{
    TInt postsynp;
    vector<string> parts;
    TReal val;

    postsynp = idx_neuron(post);
    if (postsynp == MAX_INT_NUM) {
        cerr << __FUNCTION__ << ": presynaptic neuron '" << post << "' is not a valid neuron name!" << endl;
        return false;
    }

    vector<TReal> synp;
    if (paramVal[0] != '{' || paramVal[paramVal.size() - 1] != '}') {
        cerr << __FUNCTION__ << ": The value for SYNAPSE must be enclosed by paired {}" << endl;
        return false;
    }

    paramVal = paramVal.substr(1, paramVal.size() - 2);
    strsplit(paramVal, ",", parts);
    for (std::size_t ilayer = 0; ilayer < gLayer.size(); ++ilayer) {
        if (!str2float(parts[ilayer], val)) {
            cerr << __FUNCTION__ << ": " << parts[ilayer] << " is not a valid number!" << endl;
            return false;
        }
        synp.push_back(val);
    }

    for (vector<NeurGrp>::iterator it = gNeur.begin(); it != gNeur.end(); ++it) {
        if (it->name() == pre) {
            for (vector<TReal>::size_type ilayer = 0; ilayer < synp.size(); ilayer++) {
                it->set_synp_conn(synp[ilayer], postsynp, ilayer);
            }
            return true;
        }
    }

    for (vector<ExSource>::iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
        if (it->name() == pre) {
            for (vector<TReal>::size_type ilayer = 0; ilayer < synp.size(); ilayer++) {
                it->set_synp_conn(synp[ilayer], postsynp, ilayer);
            }
            return true;
        }
    }

    cerr << __FUNCTION__ << ": presynaptic neuron '" << pre << "' is not a valid neuron name or external spike source name!" << endl;
    return false;
}

//------------------------------------------------------
// function: bool LCM::set_synp_param(string pre, string post, string ly, string paramVal)
//   Set the value of the synaptic connection from neuron group "pre" to neuron group "post" 
//
//   pre: the name of pre-synaptic neuron group
//   post: the name of post-synaptic neuron group
//   ly: the cortical layer of the synapse
//   paramVal: the array of synaptic connections, separated with "," 
//
//   Return true if the parameter is successfully set
//   Return false if the parameter is not found or the value is of a wrong type
//-------------------------------------------------------
bool LCM::set_synp_conn(string pre, string post, string ly, string paramVal)
{
    TInt postsynp, ilayer;
    TReal val;

    postsynp = idx_neuron(post);
    if (postsynp == MAX_INT_NUM) {
        cerr << __FUNCTION__ << ": synaptic conntion: presynaptic neuron '" << post << "' is not a valid neuron name!" << endl;
        return false;
    }

    ilayer = idx_layer(ly);
    if (ilayer == MAX_INT_NUM) {
        cerr << __FUNCTION__ << ": synaptic conntion: layer '" << ly << "' is not a valid layer name!" << endl;
        return false;
    }

    if (!str2float(paramVal, val)) {
        cerr << __FUNCTION__ << ": synaptic conntion: " << paramVal << " is not a valid number!" << endl;
        return false;
    }

    for (vector<NeurGrp>::iterator it = gNeur.begin(); it != gNeur.end(); ++it) {
        if (it->name() == pre) {
            it->set_synp_conn(val, postsynp, ilayer);
            return true;
        }
    }

    for (vector<ExSource>::iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
        if (it->name() == pre) {
            it->set_synp_conn(val, postsynp, ilayer);
            return true;
        }
    }

    cerr << __FUNCTION__ << ": synaptic conntion: presynaptic neuron '" << pre << "' is not a valid neuron name or external spike source name!" << endl;
    return false;
}

//-------------------------------------------------
// function: bool LCM::set_param(string paramName, string paramVal)
//   set a parameter
//   
//   paramName: the full name of the parameter, such as "NEURON.E1.THRESHOLD"
//   paramVal: the value of the parameter
//-------------------------------------------------
bool LCM::set_param(string paramName, string paramVal)
{
    vector<string> parts;
    vector<string>::iterator it;
    TInt num_part;
    string part_1, part_2, part_3, part_4;
    //TNeur neur=cNaN;

    //TReal val=0;

    paramName = strtrim(paramName);
    paramVal = strtrim(paramVal);

    strsplit(paramName, ".", parts);

    num_part = parts.size();

    part_1 = strtrim(parts[0]);
    if (num_part > 1) {
        part_2 = strtrim(parts[1]);
    }
    else {
        part_2 = "";
    }
    if (num_part > 2) {
        part_3 = strtrim(parts[2]);
    }
    else {
        part_3 = "";
    }
    if (num_part > 3) {
        part_4 = strtrim(parts[3]);
    }
    else {
        part_4 = "";
    }

    //-------------------------------------
    // set parameter for neuron groups
    //-------------------------------------
    if (part_1 == "NEURON") {
        if (num_part == 1) {
            if (paramVal[0] != '{' || paramVal[paramVal.size() - 1] != '}') {
                cerr << __FUNCTION__ << ": the value for '" << paramName << "' must be enclosed by paired {}" << endl;
                return false;
            }
            //delete the paired '{}'
            paramVal = paramVal.substr(1, paramVal.size() - 2);
            //split it to parts
            strsplit(paramVal, ",", parts);
            //reserve data space
            //gNeur.clear();
            //gNeur.reserve(parts.size());
            for (it = parts.begin(); it != parts.end(); ++it) {
                *it = strtrim(*it);
                if (!add_neur(*it)) return false;
            }
            return true;
        }

        if (num_part == 3) {
            if (gNeur.empty()) {
                cerr << __FUNCTION__ << ": set parameter '" << paramName << "' failed! Neuron group list is empty." << endl;
                return false;
            }
            if (part_2 == "GLOBAL") {
                for (vector<NeurGrp>::iterator it = gNeur.begin(); it != gNeur.end(); ++it) {
                    if (!set_neur_param((*it), part_3, paramVal)) return false;
                }
                return true;
            }

            for (vector<NeurGrp>::iterator it = gNeur.begin(); it != gNeur.end(); ++it) {
                if (it->name() == part_2) {
                    return set_neur_param((*it), part_3, paramVal);
                }
            }
            cerr << __FUNCTION__ << ": cannot find a neuron group named '" << part_2 << "'!" << endl;
            cerr << "  Existing neuron groups in the model: ";
            for (vector<NeurGrp>::iterator it = gNeur.begin(); it != gNeur.end(); ++it) {
                if (it == gNeur.begin()) cerr << it->name();
                else cerr << ", " << it->name();
            }
            cerr << endl;
            return false;
        }
        cerr << __FUNCTION__ << ": cannot find a parameter named '" << paramName << "' in the neuron groups " << endl;
        return false;
    }

    //------------------------------------
    // set parameter for layers
    //------------------------------------
    if (part_1 == "LAYER") {
        //set layers
        if (num_part == 1) {
            if (paramVal[0] != '{' || paramVal[paramVal.size() - 1] != '}') {
                cerr << __FUNCTION__ << ": the value for '" << paramName << "' must be enclosed by paired {}" << endl;
                return false;
            }
            //delete the paired '{}'
            paramVal = paramVal.substr(1, paramVal.size() - 2);
            //split it to parts
            strsplit(paramVal, ",", parts);
            //reserve space
            //gLayer.clear();
            //gLayer.reserve(parts.size());
            for (it = parts.begin(); it != parts.end(); ++it) {
                *it = strtrim(*it);
                if (!add_layer(*it)) return false;
            }
            return true;
        }
        //three parts
        if (num_part == 3) {
            if (gLayer.empty()) {
                cerr << __FUNCTION__ << ": set parameter '" << paramName << "' failed! Layer list is empty." << endl;
                return false;
            }
            if (part_2 == "GLOBAL") {
                for (vector<Layer>::iterator it = gLayer.begin(); it != gLayer.end(); ++it) {
                    if (!set_layer_param(*it, part_3, paramVal))  return false;
                }
                return true;
            }
            for (vector<Layer>::iterator it = gLayer.begin(); it != gLayer.end(); ++it) {
                if (it->name() == part_2) {
                    return set_layer_param(*it, part_3, paramVal);
                }
            }
            cerr << __FUNCTION__ << ": cannot find a layer named '" << part_2 << "'!" << endl;
            cerr << "  Existing layers in the model: ";
            for (vector<Layer>::iterator it = gLayer.begin(); it != gLayer.end(); ++it) {
                if (it == gLayer.begin()) cerr << it->name();
                else cerr << ", " << it->name();
            }
            cerr << endl;
            return false;
        }
        //all other case
        cerr << __FUNCTION__ << ": " << msg_invalid_param_name(paramName) << endl;
        return false;
    }


    //---------------------------------------
    // set parameter for receptor
    //---------------------------------------
    if (part_1 == "RECEPTOR") {
        if (num_part == 1) {
            if (paramVal[0] != '{' || paramVal[paramVal.size() - 1] != '}') {
                cerr << __FUNCTION__ << ": the value for '" << paramName << "' must be enclosed by paired {}" << endl;
                return false;
            }
            //delete the paired '{}'
            paramVal = paramVal.substr(1, paramVal.size() - 2);
            //split it to parts
            strsplit(paramVal, ",", parts);
            //gRcpt.clear();
            //gRcpt.reserve(parts.size());
            for (it = parts.begin(); it != parts.end(); ++it) {
                *it = strtrim(*it);
                if (!add_rcpt(*it)) return false;
            }
            return true;
        }
        if (num_part == 3) {
            if (gRcpt.empty()) {
                cerr << __FUNCTION__ << ": set parameter '" << paramName << "' failed! Receptor list is empty." << endl;
                return false;
            }
            //parameter for all receptor
            if (part_2 == "GLOBAL") {
                for (vector<Receptor>::iterator it = gRcpt.begin(); it != gRcpt.end(); ++it) {
                    if (!set_rcpt_param(*it, part_3, paramVal)) return false;
                }
                return true;
            }
            //parameter for specific receptor
            for (vector<Receptor>::iterator it = gRcpt.begin(); it != gRcpt.end(); ++it) {
                if (it->name() == part_2) {
                    return set_rcpt_param(*it, part_3, paramVal);
                }
            }
            cerr << __FUNCTION__ << ": cannot find a receptor named " << part_2 << "." << endl;
            cerr << "  Existing receptors in the model: ";
            for (vector<Receptor>::iterator it = gRcpt.begin(); it != gRcpt.end(); ++it) {
                if (it == gRcpt.begin()) cerr << it->name();
                else cerr << ", " << it->name();
            }
            cerr << endl;
            return false;
        }
        cerr << __FUNCTION__ << ": " << msg_invalid_param_name(paramName) << endl;
        return false;
    }

    //--------------------------------------------
    // set parameter for SYNAPSE
    //--------------------------------------------
    if (part_1 == "SYNAPSE") {
        if (num_part == 3) {
            return set_synp_conn(part_2, part_3, paramVal);
        }
        else if (num_part == 4) {
            return set_synp_conn(part_2, part_3, part_4, paramVal);
        }
        cerr << __FUNCTION__ << ": " << msg_invalid_param_name(paramName) << endl;
        return false;
    }

    //----------------------------------------------
    // set simulation parameters
    //----------------------------------------------
    if (part_1 == "LCM") {
        if (num_part != 2) {
            cerr << __FUNCTION__ << ": " << msg_invalid_param_name(paramName) << endl;
            return false;
        }
        return set_simu_param(part_2, paramVal);
    }

    //----------------------------------------------
    // set parameter for external source
    //----------------------------------------------
    if (part_1 == "SOURCE") {
        if (num_part == 1) {
            if (paramVal[0] != '{' || paramVal[paramVal.size() - 1] != '}') {
                cerr << __FUNCTION__ << ": the value for '" << paramName << "' must be enclosed by paired {}" << endl;
                return false;
            }
            //delete the paired '{}'
            paramVal = paramVal.substr(1, paramVal.size() - 2);
            //split it to parts
            strsplit(paramVal, ",", parts);
            //gExSrc.clear();
            //gExSrc.reserve(parts.size());
            for (it = parts.begin(); it != parts.end(); ++it) {
                *it = strtrim(*it);
                if (!add_exsrc(*it)) return false;
            }
            return true;
        }
        //if (num_part == 3) {
        //   if (gExSrc.empty()) {
        //      cerr << "LCM::set_param: set parameter '" << paramName << "' failed! External source list is empty."<<endl;
        //      return false;
        //   }
        //   //parameter for all receptor
        //   if (part_2 == "GLOBAL") {
        //      for (vector<ExSource>::iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
        //         if (!it->set_param(part_3, paramVal)) return false;
        //      }
        //      return true;
        //   }
        //   //parameter for specific receptor
        //   for (vector<ExSource>::iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
        //      if (it->name() == part_2) {
        //         return it->set_param(part_3, paramVal);
        //      }
        //   }
        //   cerr << "LCM::set_param: cannot find a external source named '" << part_2 << "'!"<<endl;
        //   return false;
        //}
    }

    //----------------------------------------------
    // set parameter for stimulators
    //----------------------------------------------
    if (part_1 == "STIM") {
        if (num_part == 1) {
            if (paramVal[0] != '{' || paramVal[paramVal.size() - 1] != '}') {
                cerr << __FUNCTION__ << ": the value for '" << paramName << "' must be enclosed by paired {}" << endl;
                return false;
            }
            //delete the paired '{}'
            paramVal = paramVal.substr(1, paramVal.size() - 2);
            //split it to parts
            strsplit(paramVal, ",", parts);
            //gStim.clear();
            //gStim.reserve(parts.size());
            for (it = parts.begin(); it != parts.end(); ++it) {
                *it = strtrim(*it);
                if (!add_stim(*it)) return false;
                //cout<<*it<<endl;
            }
            return true;
        }
        if (num_part == 3) {
            if (gStim.empty()) {
                cerr << __FUNCTION__ << ": " << msg_invalid_param_value(paramName, paramVal) << endl;
                cerr << "   Stimulator list is empty." << endl;
                return false;
            }
            //parameter for all receptor
            if (part_2 == "GLOBAL") {
                //cout<<paramName<<" "<<paramVal<<endl;
                for (vector<Stimulator>::iterator it = gStim.begin(); it != gStim.end(); ++it) {
                    if (!set_stim_param(*it, part_3, paramVal)) return false;
                }
                return true;
            }
            //parameter for specific receptor
            for (vector<Stimulator>::iterator it = gStim.begin(); it != gStim.end(); ++it) {
                if (it->name() == part_2) {
                    return set_stim_param(*it, part_3, paramVal);
                }
            }
            cerr << __FUNCTION__ << ": unrecognized stimulator '" << part_2 << "' !" << endl;
            cerr << "  Existing stimulators in the model: ";
            for (vector<Stimulator>::iterator it = gStim.begin(); it != gStim.end(); ++it) {
                if (it == gStim.begin()) cerr << it->name();
                else cerr << ", " << it->name();
            }
            cerr << endl;
            return false;
        }
    }

    cerr << __FUNCTION__ << ": " << msg_invalid_param_name(paramName) << endl;
    //all other case, this parameter is not for this calss
    return false;
}

//------------------------------------------------------------
// function: bool LCM::set_param(map<string, string> &paramList)
//   Set the LCM parameters with the parameter name and value
//   pairs in paramList
// 
//   The first string in a pair is the full parameter name, e.g. NEURON.E1.PARA...
//   The second string in a pair is the parameter value
//------------------------------------------------------------
bool LCM::set_param(map<string, string>& paramList)
{
    if (paramList.empty()) return true;
    map<string, string>::iterator it;
    string tmp;

    char const *paramNames[] = { "NEURON", "LAYER", "RECEPTOR", "SOURCE", "STIM" };

    for (std::size_t idx = 0; idx < 5; ++idx) {
        it = paramList.find(paramNames[idx]);
        if (it == paramList.end()) {
            cerr << __FUNCTION__ << ": cannot find parameter '" << it->first << "' in the list! " << _FILE_LINE_ << endl;
            return false;
        }
        else if (!set_param(it->first, it->second)) {
            cerr << __FUNCTION__ << ": set '" << it->first << "' failed! " << _FILE_LINE_ << endl;
            cerr << "** " << it->first << " = " << it->second << endl;
            return false;
        }
        else {
            paramList.erase(it);
        }
    }

    if (!ng_check_idx(gNeur)) {
        cerr << __FUNCTION__ << ": neuron group indices are consistent! " << _FILE_LINE_ << endl;
        return false;
    }

    ly_sort(gLayer);
    if (!ly_chk_idx(gLayer)) {
        cerr << __FUNCTION__ << ": set cortical layer failed! " << _FILE_LINE_ << endl;
        return false;
    }

    //processing the rest of the list
    it = paramList.begin();
    while (it != paramList.end()) {
        if (set_param(it->first, it->second)) {
            paramList.erase(it);
            it = paramList.begin();
        }
        else {
            cerr << __FUNCTION__ << ": " << msg_invalid_param_value(it->first, it->second) << endl;
            return false;
        }
    }
    return true;
}

//-------------------------------------------------------
// function:: bool LCM::init(void)
//   Initialize the model
//   Return true if the model is ready for running, return false otherwise
//-------------------------------------------------------
bool LCM::init(void)
{
    //check parameter in LCM
    for (TInt ii = 0; ii < LCM_PARA_NUM; ++ii) {
        if (!_lcm_paramFlg[ii]) {
            cerr << __FUNCTION__ << ": " << msg_param_not_set(string("LCM.") + string(LCM_paramName[ii])) << endl;
            _l_state = false;
            return false;
        }
    }

    //please note gInv_step, gElmt_num and gElmt_size are calculated 
    //when the corresponding parameter values are set

    //---------------------------------------------
    //check layers
    //---------------------------------------------
    for (vector<Layer>::iterator it = gLayer.begin(); it != gLayer.end(); ++it) {
        if (!it->is_ready()) {
            cerr << __FUNCTION__ << ": " << msg_object_not_ready(it->name()) << endl;
            cerr << it->print() << endl;
            _l_state = false;
            return false;
        }
    }

    //---------------------------------------------
    //check the bounday of cortical layers
    //---------------------------------------------
    if (!ly_chk_boundary(gLayer)) {
        cerr << "ERROR! LCM::init: check the boundary of cortical layers failed! " << _FILE_LINE_ << endl;
        _l_state = false;
        return false;
    }
    gLayer_num = gLayer.size();  //layer number

    //---------------------------------------------
    //check receptors
    //---------------------------------------------

    gRcpt_type = gRcpt.size();

    for (vector<Receptor>::iterator it = gRcpt.begin(); it != gRcpt.end(); ++it) {
        it->init();
        //cout<<it->name()<<endl;
        it->precalc(gStep_size);

        if (!it->is_ready()) {
            cerr << __FUNCTION__ << ": " << msg_object_not_ready(it->name()) << endl;
            cerr << it->print() << endl;
            _l_state = false;
            return false;
        }

        if (it->type() == cEXCIT) {
            gRcpt_excit.push_back(*it);
        }
        else {
            gRcpt_inhib.push_back(*it);
        }
    }

    if (gRcpt_excit.empty()) {
        cerr << __FUNCTION__ << ": there is no excitatory receptor! " << _FILE_LINE_ << endl;
        return false;
    }
    if (gRcpt_inhib.empty()) {
        cerr << __FUNCTION__ << ": there is no inhibitory receptor! " << _FILE_LINE_ << endl;
        return false;
    }

    //gRcpt.clear(); //keep the list for external use

    //----------------------------------------------
    //check neuron groups
    //----------------------------------------------
    gNG_num = gNeur.size();    //how many neuron groups are there?
    gV_rev_min = gNeur[0].V_rev();
    gV_rev_max = gV_rev_min;
    for (vector<NeurGrp>::iterator it = gNeur.begin(); it != gNeur.end(); ++it) {
        it->init(time_step());
        //it->precalc(gElmt_size, gGrid_row, gStep_size);
        if (!it->is_ready()) {
            cerr << __FUNCTION__ << ": " << msg_object_not_ready(it->name()) << endl;
            cerr << it->print() << endl;
            _l_state = false;
            return false;
        }
        if (it->V_rev() > gV_rev_max)  gV_rev_max = it->V_rev();
        if (it->V_rev() < gV_rev_min)  gV_rev_min = it->V_rev();
    }

    //----------------------------------------------
    //add stimulator to their external sources
    //----------------------------------------------
    gStim_num = gStim.size();
    for (vector<Stimulator>::iterator it = gStim.begin(); it != gStim.end(); ++it) {
        bool flag = false;
        for (vector<ExSource>::iterator it2 = gExSrc.begin(); it2 != gExSrc.end(); ++it2) {
            if (it->source() == it2->index()) {
                it2->add_stim(*it);
                flag = true;
                break;
            }
        }
        if (!flag) {
            cerr << __FUNCTION__ << ": " << it->name() << ": the source is not correct! " << _FILE_LINE_ << endl;
            cerr << it->print() << endl;
            return false;
        }
    }
    gStim.clear();

    //---------------------------------------------
    //check parameter in external source
    //---------------------------------------------
    gExSrc_num = gExSrc.size();
    for (vector<ExSource>::iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
        it->init(); // it will also check the attached simulators
        if (it->stim_num() == 0) {
            cerr << __FUNCTION__ << ": " << it->name() << " has no stimulator attached! " << _FILE_LINE_ << endl;
            return false;
        }
        if (!it->is_ready()) {
            cerr << __FUNCTION__ << ": " << msg_object_not_ready(it->name()) << endl;
            return false;
        }
    }

    //sort the synapses by its postsynaptic neuron index, laminar location
    for (vector<NeurGrp>::iterator neur_it = gNeur.begin(); neur_it != gNeur.end(); ++neur_it) {
        if (neur_it->synp_conn_num() == 0)
            continue;
        for (vector<SynpConn>::iterator it = neur_it->synp_conn().begin(); it != neur_it->synp_conn().end(); ++it) {
            for (vector<SynpConn>::iterator it2 = it + 1; it2 != neur_it->synp_conn().end(); ++it2) {
                if (it->postsynp() < it2->postsynp()) {
                    continue;
                }
                if (it->postsynp() > it2->postsynp()) {
                    sy_swap(*it, *it2);
                    continue;
                }
                if (it->layer() < it2->layer()) {
                    continue;
                }
                if (it->layer() > it2->layer()) {
                    sy_swap(*it, *it2);
                    continue;
                }
            }
        }
    }

    //calculate distances between layers
    gLy_dist.clear();
    gLy_dist.resize(gLayer_num*gLayer_num, 0);
    for (std::size_t ily = 0; ily < gLayer.size(); ++ily) {
        for (std::size_t jly = 0; jly < gLayer.size(); ++jly) {
            if (ily != jly) {
                gLy_dist[LY_DIST_IDX(ily, jly)] = fabs(gLayer[ily].depth() - gLayer[jly].depth());
            }
            else {
                //the average of absolute difference between two random number is 1/3
                gLy_dist[LY_DIST_IDX(ily, jly)] = gLayer[ily].thickness() / 3.;
            }
        }
    }

    //set the time delay and decay factor for each synaptic connection

    TInt t_neur, slayer, tlayer, ilayer;

    for (vector<NeurGrp>::iterator ng_it = gNeur.begin(); ng_it != gNeur.end(); ++ng_it) {
        if (ng_it->synp_conn_num() == 0) continue;
        slayer = ng_it->layer();
        for (vector<SynpConn>::iterator sy_it = ng_it->synp_conn().begin(); sy_it != ng_it->synp_conn().end(); ++sy_it) {
            t_neur = sy_it->postsynp();
            tlayer = gNeur[t_neur].layer();
            ilayer = sy_it->layer();
            sy_it->set_spk_delay(static_cast<TInt>(ly_dist(ilayer, slayer) / ng_it->spk_speed() / gStep_size + 0.5));
            sy_it->set_psp_delay(static_cast<TInt>(ly_dist(ilayer, tlayer) / gNeur[t_neur].psp_speed() / gStep_size + 0.5));
            sy_it->set_psp_decay(gNeur[t_neur].eqn_psp_decay(ly_dist(ilayer, tlayer)));
            sy_it->set_eqM_const(1. / (gNeur[sy_it->presynp()].V_rev() - gNeur[sy_it->postsynp()].V_0()));
            //for Synpconn: sy_weight == sy_synp * sy_psp_decay * sy_eqM_const;
            //cout<<sy_it->presynp()<<" => "<<sy_it->postsynp()<<" @ "<<sy_it->layer()<<"="<<sy_it->spk_delay()<<endl;
        }
    }
    for (vector<ExSource>::iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
        if (it->synp_conn_num() == 0) continue;
        for (vector<SynpConn>::iterator sy_it = it->synp_conn().begin(); sy_it != it->synp_conn().end(); ++sy_it) {
            t_neur = sy_it->postsynp();
            tlayer = gNeur[t_neur].layer();
            ilayer = sy_it->layer();
            sy_it->set_spk_delay(0);
            sy_it->set_psp_delay(static_cast<TInt>(ly_dist(ilayer, tlayer) / gNeur[t_neur].psp_speed() / gStep_size + 0.5));
            sy_it->set_psp_decay(gNeur[t_neur].eqn_psp_decay(ly_dist(ilayer, tlayer)));
            sy_it->set_eqM_const(1. / (gV_rev_max - gNeur[sy_it->postsynp()].V_0()));
            //for Synpconn: sy_weight == sy_synp * sy_psp_decay * sy_eqM_const;
        }
    }

    if (gSpk_delay != NULL) {
        for (TInt ineur = 0; ineur < gNG_num; ++ineur) {
            delete[] gSpk_delay[ineur];
        }
        delete[] gSpk_delay;
    }

    gSpk_delay = new TInt*[gNeur.size()];

    if (gSynp_pct != NULL) {
        for (TInt ineur = 0; ineur < gNG_num; ++ineur) {
            delete[] gSynp_pct[ineur];
        }
        delete[] gSynp_pct;
    }
    gSynp_pct = new TReal*[gNeur.size()];

    TInt spk_delay_size = gGrid_row * gGrid_row * SPK_PATH_NUM;
    TInt synp_pct_size = gElmt_num * gElmt_num * SPK_PATH_NUM;

    for (TInt ineur = 0; ineur < gNeur.size(); ++ineur) {
        gSpk_delay[ineur] = new TInt[spk_delay_size];
        std::fill(gSpk_delay[ineur], gSpk_delay[ineur] + spk_delay_size, MAX_INT_NUM);

        gSynp_pct[ineur] = new TReal[synp_pct_size];

        TInt d_x, d_y;
        TReal tmp;

        TInt *spk_delay;
        TReal *synp_pct;
        for (TInt ielmt = 0; ielmt < gElmt_num; ++ielmt) {
            for (TInt jelmt = 0; jelmt < gElmt_num; ++jelmt) {

                d_x = abs(ielmt / gGrid_row - jelmt / gGrid_row);
                d_y = abs(ielmt%gGrid_row - jelmt%gGrid_row);

                //path 1
                spk_delay = gSpk_delay[ineur] + SPK_DELAY_IDX(d_x, d_y, 0);
                synp_pct = gSynp_pct[ineur] + SYNP_PCT_IDX(ielmt, jelmt, 0);

                if ((*spk_delay) == MAX_INT_NUM)
                    (*spk_delay) = static_cast<TInt>(sqrt(1.0*d_x*d_x + d_y*d_y)
                        * gElmt_size / (gNeur[ineur].spk_speed() * gStep_size) + 0.5);
                do {
                    tmp = rand_gauss(1., 0.2);
                } while (tmp < 0.);

                (*synp_pct) = tmp * gNeur[ineur].eqn_synp_ratio(d_x*gElmt_size, d_y*gElmt_size, gElmt_size);

                if ((*synp_pct) < SYNP_RATIO_EPS)
                    (*synp_pct) = 0.;

                //path 2
                ++spk_delay;
                ++synp_pct;

                if ((*spk_delay) == MAX_INT_NUM)
                    (*spk_delay) = static_cast<TInt>(sqrt(1.0*(gGrid_row - d_x)*(gGrid_row - d_x) + d_y*d_y)
                        * gElmt_size / (gNeur[ineur].spk_speed() * gStep_size) + 0.5);

                do {
                    tmp = rand_gauss(1., 0.2);
                } while (tmp < 0.);

                (*synp_pct) = tmp * gNeur[ineur].eqn_synp_ratio((gGrid_row - d_x)*gElmt_size, d_y*gElmt_size, gElmt_size);

                if ((*synp_pct) < SYNP_RATIO_EPS)
                    (*synp_pct) = 0.;

                //path_3
                ++spk_delay;
                ++synp_pct;
                if (*spk_delay == MAX_INT_NUM)
                    (*spk_delay) = static_cast<TInt>(sqrt(1.0*d_x*d_x + (gGrid_row - d_y)*(gGrid_row - d_y))
                        * gElmt_size / (gNeur[ineur].spk_speed() * gStep_size) + 0.5);
                do {
                    tmp = rand_gauss(1., 0.2);
                } while (tmp < 0.);
                (*synp_pct) = tmp * gNeur[ineur].eqn_synp_ratio(d_x*gElmt_size, (gGrid_row - d_y)*gElmt_size, gElmt_size);
                if ((*synp_pct) < SYNP_RATIO_EPS)
                    (*synp_pct) = 0.;

                //path_4
                ++spk_delay;
                ++synp_pct;
                if ((*spk_delay) == MAX_INT_NUM)
                    (*spk_delay) = static_cast<TInt>(sqrt(1.0*(gGrid_row - d_x)*(gGrid_row - d_x) + (gGrid_row - d_y)*(gGrid_row - d_y))
                        * gElmt_size / (gNeur[ineur].spk_speed() * gStep_size) + 0.5);
                do {
                    tmp = rand_gauss(1., 0.2);
                } while (tmp < 0.);
                (*synp_pct) = tmp * gNeur[ineur].eqn_synp_ratio((gGrid_row - d_x)*gElmt_size, (gGrid_row - d_y)*gElmt_size, gElmt_size);
                if ((*synp_pct) < SYNP_RATIO_EPS)
                    (*synp_pct) = 0.;
            }
        }
    }

    _l_state = true;

    //cout<<"LCM initialization finished! "<<endl;
    return true;
}

//-----------------------------------------------
// function: string LCM::print(void)
//   Return the parameter setting of the model
//   The parameter output follows the parameter 
//     input formate
//-----------------------------------------------
string LCM::print(void) const
{
    ostringstream oss;
    bool bgnFlg;

    oss << "//" << endl;
    oss << "//Here comes the simulation information." << endl;
    oss << "//Units: time - ms, length - mm" << endl;
    oss << "LCM {" << endl;
    oss << "\t" << LCM_paramName[LCM_IDX_GRID_SIZE] << " = " << gROI_size << "; // mm" << endl;
    oss << "\t" << LCM_paramName[LCM_IDX_GRID_ROW] << " = " << gGrid_row << ";" << endl;
    oss << "\t" << LCM_paramName[LCM_IDX_SIMU_TIME] << " = " << gTotal_time << "; // msec" << endl;
    oss << "\t" << LCM_paramName[LCM_IDX_TIME_STEP] << " = " << gStep_size << "; // msec" << endl;
    oss << "};" << endl << endl;

    oss << "//" << endl;
    oss << "//cortical layer list." << endl;
    oss << "//number of layers = " << Layer::count() << endl;
    if (!gLayer.empty()) {
        oss << "LAYER = {";
        vector<Layer>::const_iterator it = gLayer.begin();
        oss << it->name();
        ++it;
        while (it != gLayer.end()) {
            oss << ", " << it->name();
            ++it;
        }
        oss << "};" << endl << endl;
    }
    else {
        oss << "//The layer list is empty!" << endl;
    }

    if (!gLayer.empty()) {
        oss << "//" << endl;
        oss << "//Here comes the cortical layer information." << endl;
        for (vector<Layer>::const_iterator it = gLayer.begin(); it != gLayer.end(); ++it) {
            oss << it->print() << endl;
        }
    }

    oss << "//" << endl;
    oss << "//neuron group list." << endl;
    oss << "//number of neuron groups = " << NeurGrp::ng_count() << endl;
    if (!gNeur.empty()) {
        oss << "NEURON = {";
        vector<NeurGrp>::const_iterator it = gNeur.begin();
        oss << it->name();
        ++it;
        while (it != gNeur.end()) {
            oss << ", " << it->name();
            ++it;
        }
        oss << "};" << endl << endl;
    }
    else {
        oss << "//The neuron group list is empty!" << endl;
    }

    if (!gNeur.empty()) {
        oss << "//" << endl;
        oss << "//Here comes the neuron group information." << endl;
        for (vector<NeurGrp>::const_iterator it = gNeur.begin(); it != gNeur.end(); it++) {
            oss << it->print(gLayer) << endl;
        }
    }

    oss << "//" << endl;
    oss << "//external spike source list." << endl;
    oss << "//number of external spike sources = " << ExSource::es_count() << endl;
    if (!gExSrc.empty()) {
        oss << "SOURCE = {";
        vector<ExSource>::const_iterator it = gExSrc.begin();
        oss << it->name();
        ++it;
        while (it != gExSrc.end()) {
            oss << ", " << it->name();
            ++it;
        }
        oss << "};" << endl << endl;
    }
    else {
        oss << "//The external spike source list is empty!" << endl;
    }

    bgnFlg = false;
    oss << "//" << endl;
    oss << "//stimulator list." << endl;
    oss << "//number of stimulators = " << Stimulator::count() << endl;
    if (!gExSrc.empty()) {
        for (vector<ExSource>::const_iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
            if (it->elmt_num() == 0) continue;
            for (TInt idx = 0; idx < it->stim_num(); ++idx) {
                if (!bgnFlg) {
                    oss << "STIM = {";
                    oss << it->stim_name(idx);
                    bgnFlg = true;
                }
                else {
                    oss << ", " << it->stim_name(idx);
                }
            }
        }
        if (bgnFlg) {
            oss << "};" << endl << endl;
        }
        else {
            oss << "//The simulator list is empty!" << endl;
        }
    }

    if (!gExSrc.empty()) {
        oss << "//" << endl;
        oss << "//Here comes the external spike source information." << endl;
        for (vector<ExSource>::const_iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
            oss << it->print(gStep_size) << endl;
        }
    }
    oss << endl;

    bgnFlg = false;
    oss << "//" << endl;
    oss << "//receptor list." << endl;
    oss << "//number of receptors = " << gRcpt_excit.size() + gRcpt_inhib.size() << endl;
    if (!gRcpt_excit.empty()) {
        for (std::size_t irc = 0; irc < gRcpt_excit.size(); ++irc) {
            if (!bgnFlg) {
                oss << "RECEPTOR = {" << gRcpt_excit[irc].name();
                bgnFlg = true;
            }
            else {
                oss << ", " << gRcpt_excit[irc].name();
            }
        }
    }

    if (!gRcpt_inhib.empty()) {
        for (std::size_t irc = 0; irc < gRcpt_inhib.size(); ++irc) {
            if (!bgnFlg) {
                oss << "RECEPTOR = {" << gRcpt_inhib[irc].name();
                bgnFlg = true;
            }
            else {
                oss << ", " << gRcpt_inhib[irc].name();
            }
        }
    }

    if (bgnFlg) {
        oss << "};" << endl << endl;
    }
    else {
        oss << "//No receptor exist." << endl << endl;
    }


    if (!gRcpt_excit.empty()) {
        oss << "//" << endl;
        oss << "//Here comes the receptor information." << endl;
        for (std::size_t irc = 0; irc < gRcpt_excit.size(); ++irc) {
            oss << gRcpt_excit[irc].print() << endl;
        }
    }

    if (!gRcpt_inhib.empty()) {
        oss << "//" << endl;
        oss << "//Here comes the receptor information." << endl;
        for (std::size_t irc = 0; irc < gRcpt_inhib.size(); ++irc) {
            oss << gRcpt_inhib[irc].print() << endl;
        }
    }

    oss << endl << "//" << endl;
    oss << "//Here comes the synaptic connection information." << endl;
    oss << "//number of synaptic connections = " << SynpConn::count() << endl;
    oss << "SYNAPSE {" << endl;
    for (vector<NeurGrp>::const_iterator it = gNeur.begin(); it != gNeur.end(); ++it) {
        if (it->synp_conn_num() == 0) {
            oss << "\t//WARNING: " << it->name() << " does not project to any neuron group!" << endl;
            continue;
        }
        for (vector<SynpConn>::const_iterator sy_it = it->synp_conn().begin(); sy_it != it->synp_conn().end(); ++sy_it) {
            oss << "\t" << it->name() << "." << gNeur[sy_it->postsynp()].name() << "." << \
                gLayer[sy_it->layer()].name() << " = " << sy_it->synp() << "; //spk_delay = " \
                << sy_it->spk_delay() << ", psp_delay = " << sy_it->psp_delay() << ", psp_decay = "\
                << sy_it->psp_decay() << "; weight = " << sy_it->weight() << endl;
        }
    }
    for (vector<ExSource>::const_iterator it = gExSrc.begin(); it != gExSrc.end(); ++it) {
        if (it->synp_conn_num() == 0) {
            oss << "\t//WARNING: " << it->name() << " does not project to any neuron group!" << endl;
            continue;
        }
        for (vector<SynpConn>::const_iterator sy_it = it->synp_conn().begin(); sy_it != it->synp_conn().end(); ++sy_it) {
            oss << "\t" << it->name() << "." << gNeur[sy_it->postsynp()].name() << "." << \
                gLayer[sy_it->layer()].name() << " = " << sy_it->synp() << "; //spk_delay = " \
                << sy_it->spk_delay() << ", psp_delay = " << sy_it->psp_delay() << ", psp_decay = "\
                << sy_it->psp_decay() << "; weight = " << sy_it->weight() << endl;
        }
    }
    oss << "};" << endl;
    return oss.str();
}
