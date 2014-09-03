#ifndef INC_Calc_H_
#define INC_Calc_H_

#include "Data.H"
#include "Convert.H"
#include <vector>
#include <map>
#include <iosfwd>

class Calc {
public:
  class Field {
  private:
    std::string mField;
  public:
    Field(const std::string& field) : mField(field) {}
    bool qRef() const {return mField.find("::") != mField.npos;}
    const std::string& str() const {return mField;}
    std::string keyString() const {
      const std::string::size_type i(mField.find("::"));
      return i == mField.npos ? std::string() : mField.substr(0,i);
    }
    int key() const {
      const std::string a(keyString());
      return a.empty() ? 0 : Convert::strTo<int>(a);
    }
    std::string comment() const {
      const std::string::size_type i(mField.find("::"));
      const std::string::size_type j(mField.rfind("::"));
      return i == mField.npos ? mField : mField.substr(i+2, j - i - 2);
    }
    Data::Type type() const {
      const std::string::size_type i(mField.rfind("::"));
      return i == mField.npos ? Data::LASTTYPE : Data::decodeType(mField.substr(i+2));
    }
    void encode(const int key, const std::string& comment, const Data::Type type) {
      mField = Convert::toStr(key) + "::" + comment + "::" + Convert::toStr(type);
    }
    bool operator < (const Field& rhs) const {return mField < rhs.mField;}
  }; // Field
private:
  MyDB mDB;
  int mGaugeKey;
  Data::Type mType;
  std::string mName;

  typedef std::vector<Field> tFields;
  typedef std::map<int, Data::Type> tTimes;

  tFields mExpr;
  tTimes mTime;

  void split(const std::string& expr, const Data::Type type);
  const std::string& mkName();
  int mkQuery(MyDB::Stmt& s, const std::string& field, 
              const int gaugeKey, const Data::Type type, 
              const time_t stime, const time_t dt) const;
public:
  explicit Calc(const int gaugeKey);

  Calc(const int gaugeKey, const std::string& expr, const Data::Type type);

  void update(Data& data, time_t t0 = 0, time_t delta = 7200);

  typedef tFields::const_iterator const_iterator;
  typedef tFields::iterator iterator;
  typedef tFields::size_type size_type;

  iterator begin() {return mExpr.begin();}
  iterator end() {return mExpr.end();}

  const_iterator begin() const {return mExpr.begin();}
  const_iterator end() const {return mExpr.end();}
  size_type size() const {return mExpr.size();}
  bool empty() const {return mExpr.empty();}

  Field& operator [] (const tFields::size_type i) {return mExpr[i];}
  const Field& operator [] (const tFields::size_type i) const {return mExpr[i];}
}; // Calc

std::ostream& operator << (std::ostream& os, const Calc& calc);

#endif // INC_Calc_H_
