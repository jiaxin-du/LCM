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

#ifndef MISC_H
#define MISC_H

#include "defines.h"
#include <map>

//--------------------------------------------------
// This file (along with misc.cpp) defines miscellaneous
// functions used in the program but has not been defined
// elsewhere.
//
//--------------------------------------------------

TInt nextpow2(TInt);

//change to lower case
std::string lowerstr(std::string str);

//change to upper case
std::string upperstr(std::string str);

//convert a float number to a string
std::string float2str(const TReal& x);

//convert a integer number to a string
std::string int2str(const TInt& x, const TInt& digit = 0);

//convert a TNeur type to string
std::string neur2str(const TNeur& neur);

//convert a std::vector of numbers to string
std::string nums2str(std::vector<TInt> nums);

//convert a string to a float number
bool str2float(std::string str, TReal& num);

//convert a string to a int number
bool str2int(std::string str, TInt &num);

//convert a string to a unsigned number
bool str2uint(std::string str, TInt& num);

//convert a string to a TNeur type
TNeur str2neur(const std::string& str);

//convert a string to a number vector
bool str2nums(std::string str, std::vector<TInt>& nums);

//remove comments from the string
std::string remove_comments(std::string str);

//trim the string, delete leading and ending spaces
std::string strtrim(const std::string& str);

//strip all white space from a string
std::string strstrip(const std::string& str);

//convert time difference (in sec) to a string
std::string sec2str(const double& sec);

//split a string to a array
void strsplit(const std::string& str, const std::string& delimiter, std::vector<std::string>& part);

//joint a string array and insert the delimiter between them
std::string strjoint(const std::vector<std::string>&, const std::string&);

//converting parameter text to parameter list
bool read_param(const std::string&, std::map<std::string, std::string>&);

//format parameter name
std::string format_para_name(const std::string&);

//format parameter value
std::string format_para_value(const std::string&);

//swap the content of two variables
template <class DataType>
inline void data_swap(DataType& a, DataType& b)
{
    DataType c = a;
    a = b;
    b = c;
};

//return the coefficient of 3rd order butterworthfilter
// y [n] = b[0] * x[n] + b[1] * x[n-1] + b[2] * x[n-2] + b3 * x[n-3]
//                     - a[1] * y[n-1] - a[2] * y[n-2] - a3 * y[n-3]
// the first parameter is f-cut/f_sample (input)
// the second parameter is a reference to a rray for b (output)
// the third parameter is a reference to a array for a (output)
void calc_3rd_butter_coeff(const TReal &, std::vector<TReal> &, std::vector<TReal> &);

//define error messages
std::string msg_allocation_error(const std::bad_alloc &);

std::string msg_invalid_param_value(const std::string &, const std::string &);

std::string msg_invalid_param_value(const std::string &, const TReal &);

std::string msg_invalid_param_value(const std::string &, const TInt &);

std::string msg_invalid_param_name(const std::string &);

std::string msg_object_not_ready(const std::string &);

std::string msg_param_not_set(const std::string &);

//return compiler version
inline std::string cpp_ver(void)
{
#if defined(__INTEL_COMPILER) && defined(_MSC_VER)
    return std::string("Intel C++ compiler (ver ") + int2str(__ICL / 100) + std::string(".") + int2str(__ICL % 100, 2)
        + std::string(") and Visual C++ compiler (ver ") + int2str(_MSC_VER / 100) + std::string(".")
        + int2str(_MSC_VER % 100, 2) + std::string(")");
#elif defined(__INTEL_COMPILER) && defined(__GNUC__)
    return std::string("Intel C++ compiler (ver ") + int2str(__ICC / 100) + std::string(".") + int2str(__ICC % 100, 2)
        + std::string(") and GNU C++ compiler (ver ") + int2str(__GNUC__) + std::string(".")
        + int2str(__GNUC_MINOR__, 2) + std::string(")");
#elif defined(_MSC_VER)
    return std::string("Visual C++ compiler (ver ") + int2str(_MSC_VER / 100) + std::string(".") + int2str(_MSC_VER % 100, 2) + std::string(")");
#elif defined(__GNUC__)
    return std::string("GNU C++ compiler (ver ") + int2str(__GNUC__) + std::string(".") + int2str(__GNUC_MINOR__, 2) + std::string(")");
#else
    return std::string(__VERSION__);
#endif
};

#endif /* end of #ifndef MISC_H */
