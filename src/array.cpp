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
#include "array.h"

using namespace std;

DynamicArray::DynamicArray(const TInt& size, const TReal& val) :
    _p_bgn(NULL),
    _f_front(0),
    _f_last(0),
    _f_rear(0),
    _f_size(0),
    _p_rear(NULL),
    _p_end(NULL),
    _p_front(NULL),
    _f_val(val)
{
    if (size != 0)
        resize(size);
}

DynamicArray& DynamicArray::operator= (const DynamicArray& p)
{
    if (this != &p) {

        _f_size = p._f_size;
        _f_last = p._f_last;

        _f_front = p._f_front;
        _f_rear = p._f_rear;

        _f_val = p._f_val;

        if (_p_bgn != NULL) delete[] _p_bgn;

        if (_f_size != 0) {
            try {
                _p_bgn = new TReal[_f_size];
            }
            catch (bad_alloc &e) {
                cerr << MEMORY_ERROR << endl << e.what() << endl;
                exit(-1);
            }

            //copy data
            std::copy(p._p_bgn, p._p_end, _p_bgn);

            //the iterators are different from these of p
            _p_end = _p_bgn + _f_size;
            _p_rear = _p_bgn + _f_rear;
            _p_front = _p_bgn + _f_front;
        }
        else {
            _p_bgn = NULL;
            _p_end = NULL;
            _p_front = NULL;
            _p_rear = NULL;

            _f_size = 0;
            _f_last = 0;
            _f_front = 0;
            _f_rear = 0;

        }
    }
    return *this;
}
//
// Please note: resize array will reset the array, i.e.,
// all elements will be restored to the default value, 
// and the front and rear pointer will be reset!!!
void DynamicArray::resize(const TInt& size)
{

    if (size == 0) {

        if (_f_size != 0) {
            delete[] _p_bgn;
        }

        _p_bgn = NULL;
        _p_end = NULL;
        _p_front = NULL;
        _p_end = NULL;

        _f_size = 0;
        _f_last = 0;
        _f_front = 0;
        _f_rear = 0;

        return;
    }

    //only to move the front pointer if the space is enough
    if (size <= _f_size) {
        _f_front = ((_f_rear + size - 1) & _f_last);
        _p_front = _p_bgn + _f_front;

        //fill the array with default data
        std::fill(_p_bgn, _p_end, _f_val);
        return;
    }

    _f_size = nextpow2(size);

    _f_last = _f_size - 1;

    //initialise the data pointers
    _f_rear = 0;
    _f_front = size - 1;

    try {
        _p_bgn = new TReal[_f_size];
    }
    catch (bad_alloc &e) {
        cerr << MEMORY_ERROR << endl << e.what() << endl;
        exit(-1);
    }
    //the iterators are different from these of p
    _p_end = _p_bgn + _f_size;
    _p_rear = _p_bgn + _f_rear;
    _p_front = _p_bgn + _f_front;

    //fill the array with default data
    std::fill(_p_bgn, _p_end, _f_val);

}

//
//this function performs the following function
//  array[eps:eps+num-1]+mag * val[0:inc:num-1]
void DynamicArray::add2rear(const TReal* RESTRICT p_val, const TInt &num, const TInt &eps, const TReal &mag)
{
    assert(num > 0 && eps >= 0 && eps + num <= _f_size); //"array::add2rear the length of val is larger than that of data! ";

    if (num == 0) return;

    // Here is a equivalent but quicker implementation
    TReal *it_bgn = _p_bgn + ((_f_rear + eps)&_f_last);
    TReal *it_end = _p_bgn + ((_f_rear + eps + num - 1) & _f_last) + 1; //point to the element afeter the last one
    TReal *it_tmp = it_bgn;

    //#define _TMP_SW_ARRAY_METHOD_1

#ifdef _TMP_SW_ARRAY_METHOD_1

    while (it_tmp != it_end) {
        if (it_tmp == _p_end)
            it_tmp = _p_bgn;
        *it_tmp += (*p_val) * mag;
        ++it_tmp;
        ++p_val;
    }

#else
    if (it_end > it_bgn) {
        //the memory is continuous
        // p_data|----------------------------------|_p_end
        //             t_beg ->->->->->-> t_end
        while (it_tmp != it_end) {
            *it_tmp += (*p_val) * mag;
            ++it_tmp;
            ++p_val;
        }
    }
    else {
        //the memory to write is not continuous
        // p_data|----------------------------------|_p_end
        //        ->->-> t_end            t_bgn ->->
        while (it_tmp != _p_end) { //process the first part
            *it_tmp += (*p_val) * mag;
            ++it_tmp;
            ++p_val;
        }
        //now it_tmp == _p_end
        it_tmp = _p_bgn;
        while (it_tmp != it_end) { //process the second part
            *it_tmp += (*p_val) * mag;
            ++it_tmp;
            ++p_val;
        }
    }
#endif
}

//
// Instead of moving all the data, I move only the pointers
//
void DynamicArray::step_backward()
{
    //move all data point a step forward
    _f_front = ((_f_front + 1) & _f_last);
    _f_rear = ((_f_rear + 1) & _f_last);

    _p_front = _p_bgn + _f_front;
    _p_rear = _p_bgn + _f_rear;

    (*_p_front) = _f_val;

}

//
// Instead of moving all the data, I move only the pointers
//
void DynamicArray::step_forward()
{
    //move all data point a step forward
    _f_front = ((_f_front + _f_size - 1) & _f_last);
    _f_rear = ((_f_rear + _f_size - 1) & _f_last);

    _p_front = _p_bgn + _f_front;
    _p_rear = _p_bgn + _f_rear;

    (*_p_rear) = _f_val;
}

string DynamicArray::print(void) const
{
    if (_f_size == 0) return string("<empty>");
    ostringstream oss;
    TReal *it = _p_rear;
    while (it != _p_front) {
        oss << *it << ",\t";
        ++it;
        if (it == _p_end) it = _p_bgn;
    }
    oss << *it;
    return oss.str();
}
