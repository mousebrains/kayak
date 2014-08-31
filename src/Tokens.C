#include "Tokens.H"
#include <iostream>

Tokens::Tokens(const std::string& str,
               const std::string& delim,
               const bool qCollapse)
{
  std::string::size_type pos(0);

  for (std::string::size_type index;
       (index = str.find_first_of(delim, pos)) != str.npos;) {
    if (index == 0) {
      if (!qCollapse)
        mTokens.push_back(std::string());
      pos = qCollapse ? str.find_first_not_of(delim, index) : (index + 1);
      if ((pos >= str.size()) && !qCollapse)
        mTokens.push_back(std::string());
    } else {
      mTokens.push_back(str.substr(pos, index - pos));
      pos = qCollapse ? str.find_first_not_of(delim, index) : (index + 1);
      if ((pos >= str.size()) && !qCollapse)
        mTokens.push_back(std::string());
    } 
  }

  if (pos < str.size())
    mTokens.push_back(str.substr(pos));
}

std::ostream&
operator << (std::ostream& os,
             const Tokens& t)
{
  std::string delim;

  for (Tokens::const_iterator it(t.begin()), et(t.end()); it != et; ++it) {
    os << delim << *it;
    delim = ",";
  }

  return os;
}
