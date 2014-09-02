#ifndef INC_Calc_H_
#define INC_Calc_H_

#include "Data.H"
#include <vector>
#include <map>
#include <iosfwd>

class Calc {
public:
  typedef std::vector<std::string> tStrings;
  static tStrings split(const std::string& expr);
private:
  MyDB mDB;
  int mGaugeKey;
  int mDataKey;
  Data::Type mType;
  std::string mExpr;

  tStrings mTime;

  std::string buildQuery(const std::string& name, const std::string& key, 
                         const std::string& type) const;

  void load(const std::string& expr, const Data::Type type);
  time_t time();
  double calc();
  int dataKey();
public:
  explicit Calc(const int gaugeKey);

  Calc(const int gaugeKey, const std::string& expr, const Data::Type type);

  void update();

  friend std::ostream& operator << (std::ostream& os, Calc& calc);
}; // Calc

std::ostream& operator << (std::ostream& os, Calc& calc);

#endif // INC_Calc_H_
