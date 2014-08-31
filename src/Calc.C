#include "Calc.H"
#include "Tokens.H"
#include "Convert.H"
#include <iostream>

Calc::Calc(const std::string& expr,
           const std::string& time)
  : mExpr(split(expr))
  , mTime(split(time))
{
}

Calc::tVec
Calc::split(const std::string& expr)
{
  const std::string uniOps("(),+-*/");
  const Tokens toks(expr, " \t\n"); // whitespace split
  tVec a;

  for (Tokens::const_iterator it(toks.begin()), et(toks.end()); it != et; ++it) {
    std::string str(*it);
    while (!str.empty()) {
      const std::string::size_type j(str.find_first_of(uniOps));
      if (j == str.npos) { // No ops left
        a.push_back(str);
        str.clear();
        break;
      }
      if (j != 0) { // Not first, so pull off what is in front
        a.push_back(str.substr(0, j));
      }
      a.push_back(str.substr(j,1)); // push operator
      str = str.substr(j+1); // drop past operator
    } // while
  } // for
  return a;
} // split

std::string
Calc::expr() const
{
  std::string str;
  for (tVec::const_iterator it(mExpr.begin()), et(mExpr.end()); it != et; ++it) {
    str += *it;
  }
  return str;
}

std::string
Calc::time() const
{
  if (mTime.size() == 1) {
    return mTime[0];
  }

  std::string str;
  std::string delim;

  for (tVec::const_iterator it(mTime.begin()), et(mTime.end()); it != et; ++it) {
    str += delim + *it;
    delim = " ";
  }
  return str;
}

bool
Calc::remap(const tHash2Key& h2k,
            tVec& vec)
{
  for (tVec::iterator it(vec.begin()), et(vec.end()); it != et; ++it) {
    std::string::size_type i(it->find("::"));
    if (i != it->npos) { // Found it, so strip off key and remap
      const std::string key(it->substr(0,i));
      tHash2Key::const_iterator jt(h2k.find(key));
      if (jt == h2k.end()) {
        std::cout << "Did not find a key for '" << key << "'" << std::endl;
        return false;
      }
      *it = Convert::toStr(jt->second) + it->substr(i);
    } 
  }
  return true;
}
