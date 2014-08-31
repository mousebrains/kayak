#ifndef INC_Tokens_H_
#define INC_Tokens_H_

#include <iosfwd>
#include <vector>
#include <string>

class Tokens {
public:
  typedef std::vector<std::string> tTokens;
  typedef tTokens::const_iterator const_iterator;
  typedef tTokens::size_type size_type;
private:
  tTokens mTokens;
public:
  Tokens() {}

  explicit Tokens(const std::string& str, 
                  const std::string& delim = std::string(" \t\n"), 
                  const bool qCollapse = true);

  const std::string& operator [] (const tTokens::size_type index) const {return mTokens[index];}

  bool empty() const {return mTokens.empty();}
  size_type size() const {return mTokens.size();}
  const_iterator begin() const {return mTokens.begin();}
  const_iterator end() const {return mTokens.end();}
};

std::ostream& operator << (std::ostream& os, const Tokens& t);

#endif // INC_Tokens_H_
