#ifndef INC_Convert_H_
#define INC_Convert_H_

#include <string>

namespace Convert {
  time_t toTime(const std::string& str);

  std::string toComma(double a);
  std::string toLatLon(double a);

  std::string tolower(std::string str);

  std::string toStr(const time_t t, const std::string& format);

  template <typename T>
  std::string toStr(const T& val);

  template <typename T>
  T strTo(const std::string& str, bool *flag = 0);
}

#endif // INC_Convert_H_
