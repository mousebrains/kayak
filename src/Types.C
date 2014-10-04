#include "Types.H"
#include <iostream>
#include <iomanip>
#include <sstream>

std::ostream&
operator << (std::ostream& os,
             const Types::Ints& a)
{
  for (Types::Ints::size_type i(0), e(a.size()); i < e; ++i)
    os << (i == 0 ? "" : ",") << a[i];
  return os;
}

std::ostream&
operator << (std::ostream& os,
             const Types::Doubles& a)
{
  for (Types::Doubles::size_type i(0), e(a.size()); i < e; ++i)
    os << (i == 0 ? "" : ",") << std::setprecision(15) << a[i];
  return os;
}

std::ostream&
operator << (std::ostream& os,
             const Types::Strings& a)
{
  for (Types::Strings::size_type i(0), e(a.size()); i < e; ++i) {
    os << (i == 0 ? "" : ",");
    Types::quote(os, a[i]);
  }
  return os;
}

std::ostream&
operator << (std::ostream& os,
             const Types::Keys& a)
{
  std::string delim;
  for (Types::Keys::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    os << delim << *it;
    delim = ",";
  }
  return os;
}

std::ostream&
Types::quote(std::ostream& os, 
             const std::string& str)
{
  if (str.empty()) {
    os << "null";
    return os;
  }

  const char quote('\'');
  std::string::size_type j(str.find(quote));

  if (j == str.npos) { // no quotes in string
    os << quote << str << quote;
    return os;
  }

  std::string::size_type i(0);

  os << quote;

  while (j != str.npos) {
    if (i != j) {
      os << str.substr(i,j-i);
    }
    os << quote;
    i = j;
    j = str.find(quote, j+1);
  }
  os << str.substr(i) << quote;

  return os;
}

std::string
Types::quote(const std::string& str)
{
  std::ostringstream oss;
  quote(oss, str);
  return oss.str();
}
