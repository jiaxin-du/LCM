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

#ifndef DEFINES_H
#define DEFINES_H

//----------------------------------------------------
// This header file does house-keeping jobs by including
// approriate files used in the program. It also deal with 
// differences between compilers.
//
// The program has been tested in GUN C++ compiler 4.7
// Intel C++ compiler ver 15.00, and Miscrosoft Visual C++ 
// compiler 2012.
//-----------------------------------------------------

//debug options
//comment out the following line to ebnable debug info output
#ifndef DEBUG
#define NDEBUG
#endif
#include <cassert>

//OpenMP: share memory parallelisation
//openmp only effective when the program
//is compiled with "-fopenmp" or equivalent options
#ifdef _OPENMP
#include "omp.h"
#endif

//directive for retrict variables
#ifdef __INTEL_COMPILER
#define RESTRICT restrict
#else
#define RESTRICT __restrict
#endif

//math library
#ifdef __INTEL_COMPILER
#include "mathimf.h" //mathimf is for intel compiler only
#else
#include <cmath>
#endif

//C library
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

//C++ library
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

//#define  ALIGNMENT      64

#ifndef  MAX_NUMS
#define  MAX_NUMS
#define  MAX_UINT_NUM   (0xFFFFFFFFu) //maximum value of an unsigned integer type variable
#define  MAX_INT_NUM    (0x7FFFFFFF) //maximum value of an integer type variable
#endif

//constant definition
#define  PI             (3.141592653589793)
#define  EULER          (2.718281828459045)
#define  SQRT_2         (1.414213562373095)
#define  SQRT_3         (1.732050807568877)

#define  LOG_2          (0.693147180559945)
#define  LOG_3          (1.098612288668110)

//FILE and line info used in error message
#if defined(__CINT__) || defined(__CLING__) /* ROOT interpreter */
#define _FILE_LINE_     "(File: UNKNOWN; line: UNKNOWN)"
#else
#define _FILE_LINE_     "(File: "<<__FILE__<<"; line: "<<__LINE__<<")"
#endif

//file path separator
#if defined(_WIN32) || defined(_WIN64)
#define FILE_PATH_SEP   ('\\')
#else
#define FILE_PATH_SEP   ('/')
#endif

//define common error message
#define MEMORY_ERROR    __FUNCTION__<<": memory allocation failed in '"<<__FILE__<<"' line "<<__LINE__<<". "

//data type definition
typedef double         TReal;      //type for real numbers 
typedef float          TFloat;     //type for output numbers
typedef int            TInt;       //type for integer numbers
//typedef unsigned int   UInt;       //unsigned integer
typedef unsigned int   TSize;
typedef enum tNeur {
    cNaN = -1,
    cEXCIT = 0,
    cINHIB = 1
} TNeur; //neuronal type

//return the next pow of 2, for example, nextpow2(3) -> 4, nextpow2(8) -> 8
#endif /* end of #ifndef DEFINES_H */
