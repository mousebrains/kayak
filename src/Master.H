#ifndef INC_Master_H_
#define INC_Master_H_

#include "MyDB.H"
#include "Data.H"
#include <string>
#include <set>


class Master {
private:
  MyDB mDB;
public:
  Master() {}

  MyDB& db() {return mDB;}

  MyDB::tInts stateKeysWithGauges(const std::string& state);
  MyDB::tInts keysWithGauges(const MyDB::tInts& keys);
  
  typedef std::set<std::string> tSet;
  tSet allStates();

  bool qModifiedSince(const time_t t);

  size_t gaugeKey(const size_t key);

  struct PlotInfo {
    size_t gaugeKey;
    std::string name;
    std::string location;
    double low;
    double high;
    double lowOptimal;
    double highOptimal;
    double latitude;
    double longitude;
    PlotInfo() : gaugeKey(0), low(-1), high(-1) , lowOptimal(-1), highOptimal(-1) {}
    PlotInfo(const size_t gkey, const std::string& str, const std::string& loci,
             const double lreg, const double hreg,
             const double loptimal, const double hoptimal,
             const double lat, const double lon)
      : gaugeKey(gkey), name(str), location(loci), low(lreg), high(hreg)
      , lowOptimal(loptimal), highOptimal(hoptimal)
      , latitude(lat), longitude(lon) {}
  };

  PlotInfo plotInfo(const size_t key, const Data::Type type);

  static size_t deHash(const std::string& str);
  static std::string mkHash(size_t val);

  class Info {
  public:
    size_t key;
    bool qNoShow;
    time_t created;
    time_t modified;
    std::string nature;
    std::string classString;
    std::string difficulties;
    std::string displayName;
    std::string drainage;
    double drainageArea;
    double elevation;
    double elevationLost;
    std::string features;
    double gradient;
    double latitudePutin;
    double longitudePutin;
    double latitudeTakeout;
    double longitudeTakeout;
    std::string location;
    double length;
    double lowFlow;
    double highFlow;
    double lowGauge;
    double highGauge;
    double optimalLowFlow;
    double optimalHighFlow;
    std::string notes;
    std::string pageNumber;
    std::string region;
    std::string remoteness;
    std::string riverName;
    std::string runNumber;
    std::string scenery;
    std::string season;
    std::string section;
    std::string sortKey;
    std::string state;
    std::string watershedType;
    size_t gaugeKey;
    std::string gaugeName;

    Info();
    explicit Info(MyDB::Stmt& s);
  }; // Info

  Info getInfo(const size_t key);

  typedef std::vector<Info> tInfo;
  tInfo getLike(const std::string& sql);
  tInfo getLike(const size_t key);
  tInfo getAll();
};

#endif // INC_Master_H_
