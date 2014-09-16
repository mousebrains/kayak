#ifndef INC_String_H_
#define INC_String_H_

#include <string>

namespace String {
  std::string trim(const std::string& str);
  std::string tolower(const std::string& str);
  // std::string toupper(const std::string& str);
  std::string capitalize(const std::string& str);
}

#endif // INC_String_H_
