#ifndef INC_Gauges_H_
#define INC_Gauges_H_

#include "Data.H"
#include <iosfwd>

class Gauges : public Table {
public:
  struct Limits {
    double minFlow, maxFlow;
    double minGauge, maxGauge;
    double minTemperature, maxTemperature;
    Limits() : minFlow(0), maxFlow(1e9), minGauge(-1e3), maxGauge(2e4)
             , minTemperature(32), maxTemperature(140) {}
    Limits(const double minf, const double maxf,
           const double ming, const double maxg,
           const double mint, const double maxt)
      : minFlow(minf), maxFlow(maxf), minGauge(ming), maxGauge(maxg)
      , minTemperature(mint), maxTemperature(maxt) {}
  };
private:
  // void updateString(const int key, const std::string& value, const std::string& name);
  // void updateDouble(const int key, const double value, const std::string& name);

  typedef std::map<int, Limits> tLimits;
  tLimits mLimits;

  int id2gaugeKey(const std::string& id, const std::string& field);
public:
  Gauges();
 
  int idUSGS2gaugeKey(const std::string& id);
  int idCBTT2gaugeKey(const std::string& id);
 
  bool chkLimits(const int key, const Data::Type type, const double val);

  struct PlotInfo {
    std::string name;
    std::string location;
    double latitude;
    double longitude;
    PlotInfo() {}
    PlotInfo(const std::string& n, const std::string& loci, const double lat, const double lon) 
      : name(n), location(loci), latitude(lat), longitude(lon) {}
  };

  PlotInfo plotInfo(const size_t key);

  struct Info {
    size_t gaugeKey;
    time_t date;
    std::string name;
    double latitude; 
    double longitude; 
    std::string description;
    std::string location;
    std::string idUSGS;
    std::string idCBTT;
    std::string idUSBR;
    std::string idUnit;
    std::string state;
    std::string county;
    double elevation; 
    double drainageArea; 
    double bankfullStage; 
    double floodStage; 
    double minFlow; 
    double maxFlow; 
    double minGauge; 
    double maxGauge; 
    double minTemperature; 
    double maxTemperature; 
    std::string calcFlow; 
    std::string calcGauge; 

    Info();
    Info(MyDB::Stmt& s);

    bool operator < (const Info& rhs) const {return gaugeKey < rhs.gaugeKey;}
  };

  Info getInfo(const size_t key);
  Info getLike(const size_t key);

  typedef std::set<Info> tInfo;
  tInfo getInfo(const Types::Keys& keys);
  tInfo getLike(const Types::Keys& keys);
  tInfo getAll();

  void putInfo(const Info& info); // Write updated info back to database
  void putInfo(const tInfo& info);

  typedef std::pair<std::string, std::string> tStringPair;
  tStringPair remapString(const std::string& str); // Return description and location for str

  struct LevelInfo {
    size_t gaugeKey;
    std::string location;
    bool qCalc;
  }; // LevelInfo
  typedef std::vector<LevelInfo> tLevelInfo;
  tLevelInfo levelInfo(const MyDB::tInts& gaugeKeys);
};

std::ostream& operator << (std::ostream& os, const Gauges::Limits& l);
#endif // INC_Gauges_H_
