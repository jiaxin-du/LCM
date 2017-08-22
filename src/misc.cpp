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
#include "misc.h"
#include <map>
#include <string>
#include <algorithm>

using namespace std;

//--------------------------------------------------
// function: TInt nextpow2(TInt x)
// return the next pow of 2 that >= x
// x must be positive
//--------------------------------------------------
TInt nextpow2(TInt x)
{
   assert(x > 0);

   TInt y = x;
   while (x &= (x ^ (~x + 1)))
      y = x << 1;
   return y;
}

//--------------------------------------------------
// function: string lowerstr(string)
// change the string to lowercase
//--------------------------------------------------
string lowerstr(string str)
{
   for (TInt idx = 0; idx != str.size(); ++idx)
      str[idx] = static_cast<char>(tolower(str[idx]));
   return str;
}

//--------------------------------------------------
// function: string upperstr(string)
// convert the string to uppercase 
//--------------------------------------------------
string upperstr(string str)
{
   for (TInt idx = 0; idx != str.size(); ++idx)
      str[idx] = static_cast<char>(toupper(str[idx]));
   return str;
}

//--------------------------------------------------
// function: string float2str(const TReal& x)
//   convert x to a string
//--------------------------------------------------
std::string float2str(const TReal& x)
{
   char buff[32];
   sprintf(buff, "%f", x);
   return string(buff);
}

//--------------------------------------------------
// function: string int2str(TInt, TInt)
//   convert x to a string, if the width is less than digit
//   zero will be padded to the front of the string
//   for example:
//      int2str(19, 3) => "019"
//      int2str(119, 2) => "119"
//--------------------------------------------------
string int2str(const TInt& x, const TInt& digit)
{
   char buff[32];
   TInt len = sprintf(buff, "%d", x);

   string str;

   if (len < digit && digit < 32) {
      str = string(digit - len, '0') + string(buff);
   }
   else {
      str = string(buff);
   }
   return str;
}

//--------------------------------------------------
// function: string neur2str(TNeur)
//   convert the neur to a string
//--------------------------------------------------
string neur2str(const TNeur &neur)
{
   if (neur == cEXCIT)
      return string("EXCIT");
   if (neur == cINHIB)
      return string("INHIB");
   return string("UNSET_VALUE");
}

//--------------------------------------------------
// funtion: string nums2str(vector<TInt>)
//    convert a vector of number to string,
//    the numbers will be seperated using ',', 
//    and consective numbers are jointed together
//--------------------------------------------------
string nums2str(vector<TInt> nums)
{
   if (nums.empty()) return string(" ");

   std::sort(nums.begin(), nums.end());

   ostringstream oss;
   bool flag = false;

   TInt cnt = 0;

   oss << nums[cnt++];
   while (cnt < nums.size()) {
      //check whether the nums[cnt-1] and nums[cnt] is successive
      if (nums[cnt] - 1 == nums[cnt - 1]) {
         flag = true;
         cnt++;
      }
      else {
         //successive numbers, using the formate "1-3"
         if (flag) {
            flag = false;
            oss << "-" << nums[cnt - 1] << "," << nums[cnt];
         }
         //
         else {
            oss << "," << nums[cnt];
         }
         ++cnt;
      }
   }

   //check the last number
   if (flag) oss << "-" << nums[cnt - 1];

   oss.flush();
   return oss.str();
}

//--------------------------------------------------
// function: bool str2float(string, TReal)
//   Convert the string to a number. If no such
//   conversion is avaiable, the function returns false
//--------------------------------------------------
bool str2float(string str, TReal &val)
{
   str = strtrim(str);

   if (str.empty()) return false;

   istringstream iss(str);

   iss >> val;

   if (iss.eof() && (!iss.fail())) return true;

   return false;
}

//--------------------------------------------------
// function: bool str2uint(string, TInt)
//   convert the string to a integer.
//   If no such conversion is available,
//   the function will return false
//--------------------------------------------------
bool str2int(string str, TInt &val)
{
   str = strtrim(str);

   if (str.empty()) return false;

   istringstream iss(str);
   iss >> val;

   if (iss.eof() && (!iss.fail())) return true;

   return false;
}

//--------------------------------------------------
// function: bool str2uint(string, TInt)
//   convert the string to a integer.
//   If no such conversion is available,
//   the function will return false
//--------------------------------------------------
bool str2uint(string str, TInt &val)
{
   str = strtrim(str);

   if (str.empty()) return false;

   istringstream iss(str);
   iss >> val;

   if (iss.eof() && (!iss.fail())) return true;

   return false;
}

//--------------------------------------------------
// function: TNeur str2neur(string)
//   Convert the string to a TNeur type value
//--------------------------------------------------
TNeur str2neur(const string &str)
{
   if (upperstr(strtrim(str)) == "EXCIT")
      return cEXCIT;
   else if (upperstr(strtrim(str)) == "INHIB")
      return cINHIB;
   else
      return cNaN;
}

//--------------------------------------------------
// function bool str2nums(string, vector<TInt>)
//   Read numbers from a string 
//   The function will return flase, if a error is encountered
//--------------------------------------------------
bool str2nums(string str, vector<TInt> &nums)
{
   nums.clear();
   string::size_type pos, pos1, pos2;
   TInt val1, val2, val;

   str = strtrim(str);

   //search for 1-20
   pos = str.find("-");
   if (pos != string::npos) {
      if (!str2int(str.substr(0, pos), val1) ||
         !str2int(str.substr(pos + 1), val2)) {
         return false;
      }
      if (val1 > val2) { // 20-1 is also acceptable
         data_swap<TInt>(val1, val2);
      }

      for (TInt ii = val1; ii <= val2; ++ii) {
         nums.push_back(ii);
      }
      return true;
   }
   //check for expression like 1:2:10
   pos1 = str.find(":");
   if (pos1 != string::npos) {
      pos2 = str.find(":", pos1 + 1);
      if (pos2 == string::npos) return false;
      if (!str2int(str.substr(0, pos1), val1) ||
         !str2int(str.substr(pos1 + 1, pos2 - pos1 - 1), val) ||
         !str2int(str.substr(pos2 + 1), val2)) {
         return false;
      }

      for (TInt ii = val1; ii <= val2; ii += val) {
         nums.push_back(ii);
      }
      return true;
   }
   //single number
   if (str2int(str, val))
      return false;
   nums.push_back(val);
   return true;
}

//--------------------------------------------------
// function: string rm_str(string)
//   remove comments from a string
//--------------------------------------------------
string remove_comments(string str)
{
   std::size_t pos = str.find("//");
   std::size_t pos2;
   while (pos != string::npos) {
      pos2 = str.find("\n", pos);
      str.erase(pos, pos2);
      pos = str.find("//");
   }
   return str;
}

//--------------------------------------------------
// function: string strstrim(string)
//   remove the leading and ending space from a string
//--------------------------------------------------
string strtrim(const string &str)
{
   if (str.size() != 0) {
      TInt pos_bgn = 0;
      while (str[pos_bgn] == ' ' || str[pos_bgn] == '\f' || str[pos_bgn] == '\n'
         || str[pos_bgn] == '\r' || str[pos_bgn] == '\t' || str[pos_bgn] == '\v')
         pos_bgn++;

      TInt pos_end = str.size();

      do {
         pos_end--;
      } while (str[pos_end] == ' ' || str[pos_end] == '\f' || str[pos_end] == '\n'
         || str[pos_end] == '\r' || str[pos_end] == '\t' || str[pos_end] == '\v');

      if (pos_bgn <= pos_end)
         return str.substr(pos_bgn, pos_end - pos_bgn + 1);
      else
         return string("");
   }
   return str;
}

//--------------------------------------------------
// function: string strstrip(string)
//   remove all white space from a string
//--------------------------------------------------
string strstrip(const string &str)
{
   if (str.size() != 0) {

      string result = "";
      result.reserve(str.size());

      TInt pos_end = str.size();

      for (TInt idx = 0; idx < pos_end; ++idx) {
         if (str[idx] != ' ' && str[idx] != '\f' && str[idx] != '\n'
            && str[idx] != '\r' && str[idx] != '\t' && str[idx] != '\v')
            result += str[idx];
      }
      return result;
   }
   return str;
}

//--------------------------------------------------
// function: string sec2str(const double& sec);
//   convert time difference (in sec) to a string
//--------------------------------------------------
string sec2str(const double& sec)
{
   TInt minute = static_cast<TInt>(sec / 60.);
   TInt second = static_cast<TInt>(sec - 60 * minute);

   ostringstream ss;
   if (minute == 0) {
      ss << second << " sec";
   }
   else {
      ss << minute << " min " << second << " sec";
   }
   return ss.str();
}

//--------------------------------------------------
// function: void strsplit(string, string, vector<string>)
//   split a string (connected by delim) to an array of string
//--------------------------------------------------
void strsplit(const string& str, const string& delim, vector<string>& parts)
{
   parts.clear();

   if (str.empty()) return;

   std::size_t pre_pos = 0;
   std::size_t pos = str.find_first_of(delim);

   while (pos != string::npos) {
      parts.push_back(str.substr(pre_pos, pos - pre_pos));
      pre_pos = pos + 1;
      pos = str.find_first_of(delim, pre_pos);
   }

   parts.push_back(str.substr(pre_pos));
}

//--------------------------------------------------
// function: string strjoint(const vector<string>& parts, const string& delim)
//   joint the strings together with the delimiter inserted between them
//--------------------------------------------------
string strjoint(const vector<string>& parts, const string& delim)
{
   if (parts.size() == 0) return string("");

   vector<string>::const_iterator it = parts.begin();

   string str = (*it);
   ++it;

   while (it != parts.end()) {
      str += delim + (*it);
      ++it;
   }

   return str;
}

// Processing parameter setting text, 
//    converting to parameter name and value pairs
// 1. remive comments
// 2. split grouped parameters
// 3. remove non-necessary parts in parameter values, including "", {} pairs
// 4. put the parameter name and value pair in paramList
bool read_param(const string& paramText, map<string, string> &paramList)
{

   istringstream iss(paramText);
   string buff, str;
   string pre_delim;
   string paramName, paramVal;

   size_t pos, pos_1, pos_2;
   TInt nbracket;
   TInt lineno = 0;

   vector<string> parts;

   while (iss.good()) {
      //bracket=false;
      getline(iss, buff);
      ++lineno;
      //use strstrip to remove comments and all spaces
      buff = strtrim(remove_comments(buff));
      if (buff.empty()) continue;
      //test whether this is bracket
      nbracket = 0;
      for (pos = 0; pos < buff.size(); ++pos) {
         if (buff[pos] == '{') ++nbracket;
         else if (buff[pos] == '}') --nbracket;
      }
      if (nbracket < 0) {
         cerr << __FUNCTION__ << ": the bracket do not match! (line " << lineno << " in parameter text)" << endl;
         cerr << "   n_bracket=" << nbracket << endl;
         cerr << buff << endl;
         return false;
      }
      else {
         while (nbracket != 0) {
            getline(iss, str);
            ++lineno;
            if (!iss.good() && str.empty()) {
               cerr << __FUNCTION__ << ": the bracket do not match! (line " << lineno << " in parameter text)" << endl;
               cerr << "   n_bracket=" << nbracket << endl;
               cerr << buff << endl;
               return false;
            }
            str = strtrim(remove_comments(str));
            buff += str;
            while (pos<buff.size()) {
               if (buff[pos] == '{') ++nbracket;
               else if (buff[pos] == '}') --nbracket;
               ++pos;
            }
         }
      }

      //deal with group parameters
      pos = buff.find("=");
      pos_1 = buff.find_first_of("{");
      pos_2 = buff.find_last_of("}");
      if (pos_1 != string::npos && pos>pos_1 && pos_2 > pos) {
         pre_delim = strtrim(buff.substr(0, pos_1)) + ".";
         str = strtrim(buff.substr(pos_1 + 1, pos_2 - pos_1 - 1));

         if (str.empty()) continue;

         buff = strtrim(buff.substr(pos_2 + 1, string::npos));

         if (buff[0] == ';') {
            buff.erase(0, 1);
         }
         else {
            cerr << __FUNCTION__ << ": no semicolon is found after bracket! (line " << lineno << " in parameter text)" << endl;
            cerr << buff << endl;
            return false;
         }

         strsplit(str, ";", parts);
         for (vector<string>::iterator vit = parts.begin(); vit != parts.end(); ++vit) {
            *vit = strtrim(*vit);
         }
         if (parts.back().size() != 0) {
            cerr << __FUNCTION__ << ": no semicolon is found after the expression! (line " << lineno << " in parameter text)" << _FILE_LINE_ << endl;
            cerr << str <<endl;
            return false;
         }
         parts.pop_back(); //remove the last element

         str.clear();

         for (vector<string>::iterator vit = parts.begin(); vit != parts.end(); ++vit) {
            str += (pre_delim + *vit + ";");
         }
         buff.insert(0, str);
      }

      //cout<<"[[["<<buff<<"]]]\n";
      strsplit(buff, ";", parts);
      if (parts.back().size() != 0) {
         cerr << __FUNCTION__ << ": no semicolon is found after the expression! (line " << lineno << " in parameter text)" <<_FILE_LINE_<< endl;
         cerr << buff << endl;
         return false;
      }
      parts.pop_back();
      for (vector<string>::iterator vit = parts.begin(); vit < parts.end(); ++vit) {
         str = strtrim(*vit);
         pos = str.find("=");
         if (pos == string::npos) {
            cerr << __FUNCTION__ << ": cannot find a '=' in the expression '" << str << "'! (line " << lineno << " in parameter text)" << endl;
            cerr << buff << endl;
            return false;
         }

         paramName = format_para_name(str.substr(0, pos));
         paramVal = format_para_value(str.substr(pos + 1));

         if (paramVal.empty()) {
            cerr << __FUNCTION__ << ": parameter '" << paramName << "' has a empty value! (line " << lineno << " in parameter text)" << endl;
            return false;
         }
         if (paramList.find(paramName) == paramList.end()) {
            paramList[paramName] = paramVal;
         }
         else {
            cerr << __FUNCTION__ << ": parameter '" << paramName << " has been set twice! (line " << lineno << " in parameter text)" << endl;
            return false;
         }
      }
   }
   return true;
}

//
//return formatted parameter name
//1. remove unnecessary spaces
//2. convert to upper case
string format_para_name(const string& paramName)
{
   vector<string> parts;
   string str;
   strsplit(paramName, ".", parts);
   for (vector<string>::iterator it = parts.begin(); it != parts.end(); ++it) {
      *it = strtrim(*it);
   }

   return upperstr(strjoint(parts, "."));
}

//
//return formatted parameter value
//1. remove unnecessary spaces
//2. convert to upper case
string format_para_value(const string& paramValue)
{
   vector<string> parts;
   string str;
   strsplit(paramValue, ",", parts);
   for (vector<string>::iterator it = parts.begin(); it != parts.end(); ++it) {
      *it = strtrim(*it);
   }
   str = upperstr(strjoint(parts, ","));
   if (str[0] == '{' && str[str.size() - 1] == '}') {
      str = string("{") + strtrim(str.substr(1, str.size() - 2)) + string("}"); //remove space inside {}
   }
   return upperstr(str);
}

//
//calculate the coefficients for 3rd order butterworth filter
// a[0] * y[n] = a[1] * y[n-1] + a[2] * y[n-2] + a[3] * y[n-3] + 
//               b[0] * x[n] + b[1] * x[n-1] + b[2] * x[n-3] + b[3] * x[n-3]
void calc_3rd_butter_coeff(const TReal &f, std::vector<TReal> &b, std::vector<TReal> &a)
{
   a.resize(4);
   b.resize(4);

   double q = 1. / tan(f * PI);
   b[0] = 1. / (1. + q * (2. + q * (2. + q)));
   b[1] = 3. * b[0];
   b[2] = b[1];
   b[3] = b[0];

   a[0] = 1.0;
   a[1] = b[0] * (3. + q * ( 2. + q * (-2. - 3. * q)));
   a[2] = b[0] * (3. + q * (-2. + q * (-2. + 3. * q)));
   a[3] = b[0] * (1. + q * (-2. + q * ( 2. - q )));
}

//
//return formatted message for invalid parameter value
string msg_allocation_error(const std::bad_alloc& e)
{
   string str = "memory allocation failed.";
   str.append(e.what());
   return str;
}

//
//return formatted message for invalid parameter value
string msg_invalid_param_value(const string &paramName, const string &paramVal)
{
   string str = "invalid parameter value: ";
   str.append(paramName);
   str.append(" = ");
   str.append(paramVal);
   str.append(". ");
   return str;
}

//
//return formatted message for invalid parameter value
string msg_invalid_param_value(const string &paramName, const TReal &val)
{
   return msg_invalid_param_value(paramName, float2str(val));
}

//
//return formatted message for invalid parameter value
string msg_invalid_param_value(const string &paramName, const TInt &val)
{
   return msg_invalid_param_value(paramName, int2str(val));
}

//
//return formatted message for invalid parameter name 
string msg_invalid_param_name(const string &paramName)
{
   string str = "invalid parameter name: ";
   str.append(paramName);
   str.append(".");
   return str;
}

//
//return formatted message for un-initialised object
string msg_object_not_ready(const string &objName)
{
   string str = objName;
   str.append(" is not ready to run.");
   return str;
}

//
//return formatted message for un-initialised parameter
string msg_param_not_set(const string &paramName)
{
   string str = "parameter ";
   str.append(paramName);
   str.append(" has not been set!");
   return str;
}

