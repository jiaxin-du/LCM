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

#ifndef ARRAY_H
#define ARRAY_H

#include "misc.h"

//----------------------------------------
//            Forward Array
//
// The class implemented a fix-sized array. 
// The array may move forward or backward without 
// relocating the data
//
//   _f_size: the size of the array   
//   _f_front: the position of the front 
//   _f_rear: the position of the rear
//   _f_val: the default value of array
//
// Please note: this class is heavily used in the simulation,
// therefore the code must stay efficient. Think twice before 
// making any changes
//
// To speed up the calculation, binary logic operations 
// are used to calculate data index. the operations are 
// only valid when the following conditions are met:
//   1. _f_size == 2**n
//   2. _f_last == _f_size-1
// In this case, for _f_front<_f_size, "_f_front|_f_size"  
// equivalent to "_f_front+_f_size", and "_f_front & _f_last" 
// equivalent to "_f_front % _f_size". For example:
//    _f_size                       0b00100000 (32)  
//    _f_last                       0b00011111 (31)
//    _f_front                      0b00001010 (10)
//    _f_front|_f_size              0b00101010 (42)
//    (_f_front|_f_size)&_f_last    0b00001010 (10)  
//
// The binary operations avoid the use of modulo operation,
// which is slow
//----------------------------------------

class DynamicArray
{
private:
    TReal* _p_bgn;    //pointer to the beginning of physical storage
    TInt  _f_front; // index of the front of the array
    TInt  _f_last;  // maximum array index, _f_last == _f_size-1; 
    TInt  _f_rear;  // index of the rear of the array
    TInt  _f_size;  // must be a power of 2

    TReal* _p_rear;  // pointer to the logical end
    TReal* _p_end;   // pointer to the end of physical storage
    TReal* _p_front; // pointer to the logical front

    TReal  _f_val;  // default value

public:
    //constructor
    //the default value will be set to val 
    DynamicArray(const TInt& size = 0, const TReal& val = 0.);

    //deep copy constructor
    DynamicArray(const DynamicArray& p) : _p_bgn(NULL) {
        *this = p;
    };

    //destructor
    ~DynamicArray(void) {
        if (_p_bgn != NULL) {
            delete[] _p_bgn;
            _p_bgn = NULL;
        }
    };

    //deep copy operator
    DynamicArray& operator= (const DynamicArray&);

    //set the default value
    inline void set_default(const TReal& val) {
        _f_val = val;
    };

    //resize the array and reset the all values to val if given
    //or default value _f_val
    void resize(const TInt& size);

    inline void resize(const TInt& size, const TReal &val) {
        resize(size);
        fill(val);
    };

    //clear the array, and delete the allocated memory
    void clear(void) { resize(0); };

    //return the logic size of the array
    inline TInt size(void) const {
        return (_f_size == 0) ? 0 : (((_f_front | _f_size) - _f_rear) & _f_last) + 1;
    };

    //return the logic size of the array
    inline TInt capacity(void) const {
        return _f_size;
    };

    //return the address of the rear
    inline TReal* rear_addr(void) {
        assert(_p_rear != NULL);
        return _p_rear;
    }

    //return the address of the rear
    inline TReal* front_addr(void) {
        assert(_p_front != NULL);
        return _p_front;
    }

    //return the data in the rear
    inline TReal rear(void) const {
        assert(_p_rear != NULL);

        return *_p_rear;
    };

    //return the data in the front
    inline TReal front(void) const {
        assert(_p_front != NULL);

        return *_p_front;
    };

    //fill the array with val
    inline void fill(const TReal& val) {
        if (_p_bgn != NULL && _p_end != NULL) {
            std::fill(_p_bgn, _p_end, val);
        }
    };

    inline TReal& operator[] (const TInt& eps) {
        assert(_p_bgn != NULL); //array should not be empty
        assert(eps < _f_size); //offset should not be out of range

        return *(_p_bgn + ((_f_rear + eps)&_f_last));
    };

    inline const TReal& operator[] (const TInt& eps) const {
        assert(_p_bgn != NULL); //array should not be empty
        assert(eps < _f_size); //offset should not be out of range

        return *(_p_bgn + ((_f_rear + eps)&_f_last));
    };

    inline TReal* at(const TInt eps) {
        assert(_p_bgn != NULL); //array should not be empty
        assert(eps < _f_size); //offset should not be out of range

        return _p_bgn + ((_f_rear + eps)&_f_last);
    };

    //add a serial of data to the end of the array
    inline void add2rear(const TReal& val, const TInt& eps = 0) {
        assert(_p_bgn != NULL);  //array should not be empty
        assert(eps < _f_size); //offset should not be out of range

        *(_p_bgn + ((_f_rear + eps)&_f_last)) += val;
    }

    //all the idx in the argument list is the position relative to _f_rear
    inline TReal get_rear(const TInt& eps = 0) const {
        assert(_p_bgn != NULL); //array should not be empty
        assert(eps < _f_size); //offset should not be out of range

        return *(_p_bgn + ((_f_rear + eps)&_f_last));
    };

    //caution: Forward::get_front is a hotspot of the program
    //all the idx in the argument list is the position relative to _f_rear
    inline TReal get_front(const TInt& eps = 0) const {
        assert(_p_bgn != NULL); //array should not be empty
        assert(eps < _f_size); //offset should not be out of range

        //return data[(_f_front+_f_size-eps) % _f_size];
        return *(_p_bgn + (((_f_front | _f_size) - eps)&_f_last));
    };

    inline void set_rear(const TReal& val, const TInt& eps = 0) {
        assert(_p_bgn != NULL); //array should not be empty
        assert(eps < _f_size); //offset should not be out of range

        *(_p_bgn + ((_f_rear + eps)&_f_last)) = val;
    };

    inline void set_front(const TReal& val, const TInt& eps = 0) {
        assert(_f_size > 0);
        assert((eps < _f_size));

        *(_p_bgn + (((_f_front | _f_size) - eps)&_f_last)) = val;
    };

    //
    //this function performs the following function
    //  array[eps:eps+num-1]+mag * val[0:inc:num-1]
    void add2rear(const TReal* RESTRICT val, const TInt& num, const TInt& eps = 0, const TReal& mag = 1.0);

    // move all data one step backward, a new data added in the front
    void step_backward(void);

    // move all data one step forward, a new data added in the rear
    void step_forward(void);

    std::string print(void) const;
};

#endif /* end of #ifndef ARRAY_H */
