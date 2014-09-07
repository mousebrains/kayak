#ifndef INC_Data_H_
#define INC_Data_H_

#include "MyDB.H"
#include "CommonData.H"
#include <iosfwd>
#include <string>
#include <set>
#include <map>

class Data : public CommonData {
private:
  struct Datum {
    const std::string name;
    const time_t t;
    const double value;
    const Type tNum;
    const std::string url;
    Datum(const std::string& n, const time_t tt, const double v, const Type tn,
          const std::string& u)
      : name(n), t(tt), value(v), tNum(tn), url(u) {}
    Datum operator = (const Datum& rhs) {return Datum(rhs.name,rhs.t,rhs.value,rhs.tNum,rhs.url);}
  };

  typedef std::vector<Datum> tData;
  tData mData;

  void dump();
public:
  Data();
  ~Data();

  void add(const std::string& name, const time_t t, const double v, 
           const Type num, const std::string& url);

  typedef std::multimap<time_t, size_t> tTimeKeys;
  tTimeKeys timeKeys(const int key, const Type t);

  typedef std::multimap<time_t, double> tObs;

  tObs observations(const MyDB::Stmt::tInts& keys, const Type t, 
                    const time_t tMin=0, const time_t tMax=0);

  MyDB::Stmt::tInts types(const MyDB::Stmt::tInts& keys); // Which data types are known these source keys

  struct AnObs {
    int key;
    time_t time;
    mutable double flow;
    mutable double gauge;
    mutable double temperature;
    std::string name;
    std::string url;

    AnObs();
    bool operator < (const AnObs& a) const;
  }; // AnObs
  typedef std::set<AnObs> tAnObs;

  tAnObs observations(const MyDB::Stmt::tInts& keys, const time_t tMin=0, const time_t tMax=0);

  struct RawObs {
    int key;
    time_t t;
    Type type;
    double obs;

    RawObs(int aKey, time_t tt, Type atype, double aobs) 
      : key(aKey), t(tt), type(atype), obs(aobs) {}
  }; // RawObs

  typedef std::vector<RawObs> tRawObs;
  tRawObs rawObservations(const MyDB::Stmt::tInts& keys, 
                          const time_t tMin = 0, const time_t tMax=0);
};

#endif // INC_Data_H_
