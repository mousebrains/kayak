#ifndef INC_Calc_H_
#define INC_Calc_H_

#include <string>
#include <vector>
#include <map>

class Calc {
public:
  typedef std::map<std::string, int> tHash2Key;
private:
  typedef std::vector<std::string> tVec;
  tVec mExpr;
  tVec mTime;

  tVec split(const std::string& a);
  bool remap(const tHash2Key& hash2key, tVec& a);
public:
  Calc(const std::string& expr, const std::string& time);

  bool remap(const tHash2Key& h2k) {return remap(h2k, mExpr) && remap(h2k, mTime);}

  std::string expr() const;
  std::string time() const;
};

#endif // INC_Calc_H_
