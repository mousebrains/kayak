#ifndef INC_Levels_H_
#define INC_Levels_H_

#include "MyDB.H"
#include <vector>
#include <set>

class Levels {
public:
  enum Level {
    UNKNOWN = 0,
    LOW,
    OKAY,
    HIGH
  };

  struct Info {
    int key; // master key
    int sortKey; // index in mInfo
    std::string state; 
    std::string name; // displayName
    std::string location; // location from master or gauges
    std::string grade; // class
    bool qCalc; // is this a calculation
    Level level; // UNKNOWN, LOW, OKAY, or HIGH
    double flow;
    time_t flowTime;
    double flowDelta;
    double gauge;
    time_t gaugeTime;
    double gaugeDelta;
    double temperature;
    time_t temperatureTime;
    double temperatureDelta;
    Info();
    explicit Info(MyDB::Stmt& s);

    bool operator < (const Info& rhs) const;
  };
private:
  MyDB mDB;

  typedef std::vector<Info> tInfo;
  tInfo mInfo; 

  const std::string mState;
  bool mqFlow;
  bool mqGauge;
  bool mqTemperature;
  bool mqClass;
  bool mqCalc;

  void load(const std::string& criteria);
public:
  Levels();
  explicit Levels(const std::string& state);
  explicit Levels(const MyDB::tInts& keys);

  void update();

  typedef tInfo::size_type size_type;
  typedef tInfo::const_iterator const_iterator;

  const_iterator begin() const {return mInfo.begin();}
  const_iterator end() const {return mInfo.end();}
  size_type size() const {return mInfo.size();}
  bool empty() const {return mInfo.empty();}

  const std::string& state() const {return mState;}

  bool qFlow() const {return mqFlow;}
  bool qGauge() const {return mqGauge;}
  bool qTemperature() const {return mqTemperature;}
  bool qClass() const {return mqClass;}
  bool qCalc() const {return mqCalc;}

  std::string json() const;
  std::ostream& json(std::ostream& os) const;

  typedef std::set<std::string> tStates;
  tStates states();
}; // Levels

#endif // INC_Levels_H_
