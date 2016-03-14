#include "Convert.H"
#include "CommonData.H"
#include "String.H"
#include <sstream>
#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <cerrno>
#include <cstring>
#include <typeinfo>

namespace {
  time_t tzOffset(const std::string& tz) {
    typedef std::map<std::string, int> tMap;
    static const tMap tzMap = {
        {"edt", -4}, {"-04:00", -4},
        {"est", -5}, {"-05:00", -5},
        {"cdt", -5}, {"-05:00", -5},
        {"cst", -6}, {"-06:00", -6},
        {"mdt", -6}, {"-06:00", -6},
        {"mst", -7}, {"-07:00", -7},
        {"pdt", -7}, {"-07:00", -7},
        {"pst", -8}, {"-08:00", -8},
        {"gmt",  0}, {"z",       0}
      };
    tMap::const_iterator it(tzMap.find(String::tolower(tz)));
    if (it != tzMap.end()) return it->second * 3600;
    std::cerr << "Timezone '" << tz << "' not found" << std::endl;
    exit(1);
  }

  bool getNumber(const std::string& str,
                 size_t& offset,
                 int& value,
                 int minDigits,
                 int maxDigits,
                 const int llim,
                 const int ulim)
    {
      value = 0;
      for (size_t e(str.size()); offset < e && maxDigits; ++offset, --maxDigits, --minDigits) {
        const char c(str[offset]);
        if ((c < '0') || (c > '9')) break;
        value = (value * 10) + (c - '0');
      }
      return (minDigits <= 0) && (value >= llim) && (value <= ulim);
    }

  size_t getZone(std::string str, std::string& tz) {
    size_t n(0);
    std::string::size_type i(str.find_first_not_of(" \t\n\r"));
    if (i != str.npos) {
      n = i; // How much whitespace I am taking off the front
      str = str.substr(i);
    }
    i = str.find_first_of("\t\n\r");
    tz = str.substr(0,i);
    return n + tz.size();
  }

  size_t getMonth(std::string str, int &mon) {
    typedef std::vector<std::string> tStrings;
    static std::vector<tStrings>  a;
    if (a.empty()) { // Needed for -std=c++0x, could use nested braces for c++11
      a.push_back(tStrings({"january", "jan"}));
      a.push_back(tStrings({"febuary", "feb"}));
      a.push_back(tStrings({"march", "mar"}));
      a.push_back(tStrings({"april", "apr"}));
      a.push_back(tStrings({"may"}));
      a.push_back(tStrings({"june", "jun"}));
      a.push_back(tStrings({"july", "jul"}));
      a.push_back(tStrings({"august", "aug"}));
      a.push_back(tStrings({"september", "sep"}));
      a.push_back(tStrings({"october", "oct"}));
      a.push_back(tStrings({"november", "nov"}));
      a.push_back(tStrings({"december", "dec"}));
    }

    size_t n(0);
    { // Strip off leading whitespace
      std::string::size_type i(str.find_first_not_of(" \t\n\r"));
      if (i != str.npos) { // Strip off leading whitespace
        n = i;
        str = str.substr(i);
      }
    }
    str = String::tolower(str);
    for (size_t i(0), e(a.size()); i < e; ++i) { // Walk through months
      const std::vector<std::string>& b(a[i]);
      for (size_t j(0), je(a[i].size()); j < je; ++j) { // Walk through representations
        if (str.find(b[j]) == 0) {
          mon = i;
          return n + b[j].size();
        }
      }
    }
    std::cerr << "Unrecognized month '" << str << "'" << std::endl;
    return 0;
  }

  bool mkTime(const std::string& str, const std::string& fmt, time_t& t) {
    std::string tz;
    struct tm tm({0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
    size_t j(0), je(str.size());;
    
    for (size_t i(0), e(fmt.size()), state(0); (i < e) && (j < je); ++i) {
      if (state == 0) { // Looking for a % in fmt
        if (fmt[i] == '%') {
          state = 1;
          continue;
        }
        if (isspace(fmt[i])) { // A whitespace, so gobble up what there is in str
          while (isspace(str[j]) && (j < je)) {
            ++j;
          }
        } else if (fmt[i] != str[j++]) { // Check for character match, then push j forwards
          return false; // Characters did not match
        }
        continue;
      }
      state = 0;
      switch (fmt[i]) {
        case '%': 
          if (str[j++] != '%') return false; // Check for % and then push j forwards
          break;
        case 'Y': 
          if (!getNumber(str, j, tm.tm_year, 4, 4, 1900, 2100)) return false; // Not a valid year
          tm.tm_year -= 1900;
          break;
        case 'm': 
          if (!getNumber(str, j, tm.tm_mon, 1, 2, 1, 12)) return false; // Not a valid month
          --tm.tm_mon; // set to [0,11] instead of [1,12]
          break;
        case 'd': 
          if (!getNumber(str, j, tm.tm_mday, 1, 2, 1, 31)) return false; // Not a valid month day
          break;
        case 'H': 
          if (!getNumber(str, j, tm.tm_hour, 1, 2, 0, 24)) return false; // Not a valid hour
          break;
        case 'M': 
          if (!getNumber(str, j, tm.tm_min, 1, 2, 0, 59)) return false; // Not a valid minute
          break;
        case 'S': 
          if (!getNumber(str, j, tm.tm_sec, 1, 2, 0, 59)) return false; // Not a valid second
          break;
        case 'B':   // full month name
        case 'b': { // abbreviated month name
            size_t n(getMonth(str.substr(j), tm.tm_mon));
            if (!n) return false;
            j += n;
          }
          break;
        case 'Z': 
        case 'z': {
            size_t n(getZone(str.substr(j), tz));
            if (tz.empty()) return false;
            j += n;
          }
          break;
        default: 
          std::cerr << "Unsupported time format '" << fmt[i] << "' in " << fmt << std::endl;
          exit(1);
      } // switch
    } // for

    if (j == je) { // Parsed cleanly, now check if tz specified
      if (tz.empty()) { // no tz, so use mktime
        t = mktime(&tm);
        return true;
      }
      t = timegm(&tm) - tzOffset(tz); // Convert assuming UTC and add back in timezone offset
      return true;
    }
  
    if (j > je) { // past
      std::cout << "Ate too much";
    } else {
      std::cout << "Leftover(" << str.substr(j) << ")";
    }
    std::cout << " str(" << str << ") fmt(" << fmt << ")" << std::endl;
    return false;
  } // mkTime
} // anonymous

time_t
Convert::toTime(const std::string& str)
{
  static const std::vector<std::string> formats = { // Longest first
    "%Y-%m-%dT%H:%M:%S%z", // will also swallow %Z
    "%Y-%m-%dT%H:%M:%S",
    "%Y-%m-%d %H:%M:%S",
    "%Y-%m-%d"
    };

  for (size_t i(0), e(formats.size()); i < e; ++i) {
    time_t t(0);
    if (mkTime(str, formats[i], t)) return t;
  }
  std::cerr << "Unable to parse '" << str << "'" << std::endl;
  return 0;
}

std::string
Convert::toComma(double a)
{
  std::string b(toStr(a));
  std::string::size_type i(b.find('.'));
 
  for (std::string::size_type j((i == b.npos ? b.size() : i) - 3), je(b.size() + 1); 
       (j != 0) && (j < je); j -= 3) {
    b.insert(j, ",");
  }

  return b;
}

std::string
Convert::toLatLon(double a)
{
  double b(fabs(a));
  std::ostringstream oss;

  if (b <= 180) { // In range
    oss << (a < 0 ? "-" : "+")
        << floor(b) << "&deg; ";
    b = (b - floor(b)) * 60;
    oss << floor(b) << "' ";
    b = (b - floor(b)) * 60;
    oss << round(b) << "&quot;";
  }

  return oss.str();
}

std::string 
Convert::toStr(const time_t t, 
               const std::string& format)
{
  struct tm *ptr(localtime(&t));
  char buffer[1024];
  strftime(buffer, sizeof(buffer), format.c_str(), ptr);
  return buffer;
}

template <typename T>
std::string
Convert::toStr(const T& val)
{
  std::ostringstream oss;
  oss << val;
  return oss.str();
}

template <typename T>
T
Convert::strTo(const std::string& str,
               bool *flag)
{
  std::istringstream iss(str);
  T t;

  if (iss >> t) {
    if (flag) {
      *flag = iss.eof();
    }
    return t;
  }

  if (flag) {
    *flag = false;
  } else {
    std::cerr << "Error in Convert::strTo '" << str << "', type " 
	      << typeid(t).name()
	      << ", " 
              << strerror(errno)
              << std::endl; 
  }
  return t;
}

template std::string Convert::toStr<int>(const int& val);
template std::string Convert::toStr<time_t>(const time_t& val);
template std::string Convert::toStr<size_t>(const size_t& val);
template std::string Convert::toStr<double>(const double& val);
template std::string Convert::toStr<CommonData::Type>(const CommonData::Type& val);

template double Convert::strTo<double>(const std::string& str, bool *flag);
template size_t Convert::strTo<size_t>(const std::string& str, bool *flag);
template int Convert::strTo<int>(const std::string& str, bool *flag);
