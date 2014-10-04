#ifndef INC_Types_H_
#define INC_Types_H_

#include <vector>
#include <set>
#include <iosfwd>

namespace Types {
  typedef std::vector<int> Ints;
  typedef std::vector<double> Doubles;
  typedef std::vector<std::string> Strings;

  typedef std::set<int> Keys;

  std::string quote(const std::string& str);
  std::ostream& quote(std::ostream& os, const std::string& str);
} // Types

std::ostream& operator << (std::ostream& os, const Types::Ints& a);
std::ostream& operator << (std::ostream& os, const Types::Doubles& a);
std::ostream& operator << (std::ostream& os, const Types::Strings& a);
std::ostream& operator << (std::ostream& os, const Types::Keys& a);

#endif // INC_Types_H_
