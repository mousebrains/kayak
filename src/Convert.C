#include "Convert.H"
#include "CommonData.H"
#include <sstream>
#include <iostream>
#include <cmath>

namespace {
 const char *format[] = {
    "%Y%b%d %H:%M",
    "%Y-%m-%d %H:%M:%S",
    "%Y%m%d%H%M%S",
    "%Y%m%d%H%M",
    "%Y%m%d%H",
    "%Y%m%d",
    "%m/%d/%Y %H%M%S",
    "%m/%d/%Y %H:%M:%S",
    "%m/%d/%Y %H%M",
    "%m/%d/%Y %H:%M",
    "%m/%d.%H:%M",
    "%m/%d%H:%M",
    "%d%b%Y%H%M%S",
    "%d%b%Y%H%M",
    "%I%M %p PST %a %b %d %Y", // 130 PM PST SAT APR 01 2006
    "%I%M %p PDT %a %b %d %Y", // 130 PM PDT SUN APR 02 2006
    "%Y %a %b%d %H:%M:%S", // 2006 Wed Mar21 12:00
    "%Y %a %b%d %H:%M", // 2006 Wed Mar21 12:00
    "%d %b %Y %H:%M:%S", // 03 APR 2006
    "%d %b %Y %H:%M", // 03 APR 2006
    "%a %b %d, %Y %H:%M", // MONDAY APRIL 3, 2006 00:00
    "%Y-%m-%dT%H:%M:%S-00:00", // 2006-03-26T04:00:00-00:00
    "%Y-%m-%dT%H:%M:%SZ", // 2010-09-24T04:30:00Z
    "%Y-%m-%d", // 2010-09-24
    "%m-%d-%Y", // 9-24-2010
    "%m/%d/%Y", // 9/24/2010
    0};
} // anonymous

time_t
Convert::toTime(const std::string& str)
{
  const char *txt(str.c_str());
  const time_t now(time(0));
  struct tm reftm;
  localtime_r(&now, &reftm);
  reftm.tm_sec = 0; // Set reference time to midnight
  reftm.tm_min = 0;
  reftm.tm_hour = 0;

  for (size_t i(0); format[i]; ++i) {
    struct tm tm = reftm;
    const char *ptr(strptime(txt, format[i], &tm));
    if (ptr && (*ptr == 0)) { // Found a match
      tm.tm_sec = 0; // Zero out the seconds
      return mktime(&tm);
    }
  }
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
Convert::tolower(std::string str)
{
  for (std::string::size_type i(0), e(str.size()); i < e; ++i) {
    str[i] = std::tolower(str[i]);
  }

  return str;
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
  iss >> t;

  if (flag) {
    *flag = str.size() == (size_t) iss.tellg();
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
