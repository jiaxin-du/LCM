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

#ifndef LAYER_H
#define LAYER_H
//-------------------------------------------------
//  This class defines a cortical layer, which has 
//  two parameters: UPPER_BOUND and LOWER_BOUND
//
//  A global counter "ly_count" is also defined to
//  count the total cortical layers created in the
//  program. The value of "ly_count" will automatically 
//  increase by one if a new layer is created.
//
//-------------------------------------------------
#include "misc.h"

#ifndef LY_PARA_NUM
#define LY_PARA_NUM  2
#define LY_IDX_UPPER 0
#define LY_IDX_LOWER 1
#endif

#ifndef LY_DEFAULT_NAME
#define LY_DEFAULT_NAME ("UNAMED_LAYER")
#endif

class Layer {
public:
    //default constructor
    Layer(const std::string &xname = LY_DEFAULT_NAME, const TInt &idx = LY_COUNT);

    //copy constructor
    Layer(const Layer &p);

    //assignment operator
    Layer& operator=(const Layer &p);

    //destructor
    ~Layer() {
        --LY_COUNT;
    };

    //set the boundary of the layer
    void set_boundary(const TReal& lower, const TReal& upper);

    //set a parameter of the layer
    bool set_param(const std::string& paramName, const TReal& val);

    //return whether the layer is sucessfully initialised
    bool is_ready() const;

    //initialise the layer
    void init(void);

    //swap the content of the object with another object
    void swap(Layer &b);

    //set the name of the layer
    inline void set_name(const std::string &xname) {
        if (!xname.empty()) {
            std::cerr << __FUNCTION__ << ": name can not be empty! In "
                << __FILE__ << " line" << __LINE__ << "." << std::endl;
            exit(-1);
        }
        _ly_name = xname;
    };

    //return the name of the layer
    inline std::string name() const { return _ly_name; };

    //return the index of the layer
    inline TInt   index() const { return _ly_idx; };

    //return the upper boundary of the layer
    inline TReal  upper() const { return _ly_paramVal[LY_IDX_UPPER]; };

    //return the lower boundary of the layer
    inline TReal  lower() const { return _ly_paramVal[LY_IDX_LOWER]; };

    //return the depth at the middle of the layer
    inline TReal  depth() const { return _ly_depth; };

    //return thickness of the layer
    inline TReal  thickness() const { return _ly_thickness; };

    //return the distance between the layer and another layer
    inline TReal  dist(const Layer& p) const { return fabs(depth() - p.depth()); };

    //convert the information of the layer to a string
    std::string print() const;

    //return how many layers has been created in current program
    static TInt count() { return LY_COUNT; };

    //return the the name of a layer. It looks for a layer in a array according to its index
    //and return its name. It returns UNNAMED_LAYER if not found 
    //please note this program does not check the consistence between indices
    static std::string idx2name(const TInt&, const std::vector<Layer>&);

    //parameter name, only use for parameter input and output
    static const char *LY_paramName[];

    friend void ly_sort(std::vector<Layer>&);

private:
    //layer name, only used for parameter input and results output
    std::string  _ly_name;

    //index of the layer, it should be unique for each layer in a program
    TInt    _ly_idx;

    //parameter values
    TReal   _ly_paramVal[LY_PARA_NUM];

    //flag for whether the parameter is set or not
    bool    _ly_paramFlg[LY_PARA_NUM];

    //The depth at the middle of the layer
    TReal   _ly_depth;

    //the thickness of the layer
    TReal   _ly_thickness;

    //ly_status indicates whether the layer is ready (all parameter is set)
    bool    _ly_state;

    //layer number counter
    static TInt LY_COUNT;
};

//swap the content of layer a and b
inline void ly_swap(Layer &a, Layer &b)
{
    a.swap(b);
};

inline TReal ly_dist(const Layer& a, const Layer& b)
{
    return a.dist(b);
};

//sort the layers based on their depth
void ly_sort(std::vector<Layer> &);

//check whether the index of layers is unique
bool ly_chk_idx(const std::vector<Layer> &);

//check the boundary of the layers is separated
bool ly_chk_boundary(const std::vector<Layer> &);

#endif /* end of #ifndef LAYER_H */
