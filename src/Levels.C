#include "Levels.H"
#include "Data.H"
#include "Master.H"
#include "Gauges.H"
#include "Fields.H"
#include "Convert.H"
#include "Tokens.H"
#include <sstream>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace {
  struct MyFields {
    const char *table() const {return "levels";}
    const char *state() const {return "state";}
    const char *key() const {return "key";}
  };
  MyFields fields;

  class Values {
  private:
    typedef std::vector<double> tValues;
    typedef std::map<time_t, tValues> tObs;
    mutable tObs mObs;

    double median(tValues& vals) const {
      const tValues::size_type n(vals.size());
      if (!n) return NAN;
      if (n == 1) return vals[0];
      if (n == 2) return (vals[0] + vals[1]) / 2;
      std::sort(vals.begin(), vals.end());
      const tValues::size_type mid(n / 2);
      return (n % 2) == 1 ? vals[mid] : (vals[mid] + vals[mid + 1]) / 2;
    }
  public:
    void operator () (const time_t t, const double obs) {
      tObs::iterator it(mObs.find(t));
      if (it == mObs.end())
        it = mObs.insert(std::make_pair(t, tValues())).first;
      it->second.push_back(obs); 
    }
    time_t lastTime() const {
      return mObs.empty() ? 0 : mObs.rbegin()->first;
    }
    double lastObs() const {
      return mObs.empty() ? NAN : median(mObs.rbegin()->second);
    }
    double delta() const {
      if (mObs.size() < 1) return NAN;

      tValues fracs;
      time_t prevT(0);
      double prevVal(NAN);
      for (tObs::reverse_iterator it(mObs.rbegin()), et(mObs.rend()); 
           (it != et) && (fracs.size() < 5); ++it) {
        const time_t t(it->first);
        const double val(median(it->second));
        if (!isnan(prevVal)) {
          const double dt(t - prevT);
          const double dv(val - prevVal);
          fracs.push_back(dv / prevVal / dt * 3600); // rate of change per hour
        }
        prevT = t;
        prevVal = val;
      }
      return median(fracs); // fractional change/day
    }

    bool empty() const {return mObs.empty();}
  }; // Values

  
  struct LInfo { // Information from the database table
    int key; // key in master table
    int gaugeKey; // gaugeKey in master table
    std::string state; // state name
    std::string sortKey; // sorting key
    std::string name; // displayName in master
    std::string location; // location from master or gauges
    std::string grade; // class from master
    bool qCalc; // is this from a calculation
    Levels::Level level; // Levels::Level
    Values flow;
    Values gauge;
    Values temperature;
    double flowLow;
    double flowHigh;
    double gaugeLow;
    double gaugeHigh;

    LInfo()
      : key(0)
      , gaugeKey(0)
      , qCalc(false)
      , level(Levels::UNKNOWN)
      , flowLow(NAN)
      , flowHigh(NAN)
      , gaugeLow(NAN)
      , gaugeHigh(NAN)
    {}

    bool operator < (const LInfo& rhs) const {
      return (sortKey < rhs.sortKey) || 
             ((sortKey == rhs.sortKey) && (key < rhs.key));
    }
  }; // LInfo

  Levels::Level mkLevel(const LInfo& info) {
    if (!info.flow.empty() && (!isnan(info.flowLow) || !isnan(info.flowHigh))) {
      const double flow(info.flow.lastObs());
      return (flow >= info.flowLow && flow <= info.flowHigh) ? Levels::OKAY : 
             (flow <= info.flowLow ? Levels::LOW : 
             (flow >= info.flowHigh ? Levels::HIGH : Levels::UNKNOWN));
    }
    if (!info.gauge.empty() && (!isnan(info.gaugeLow) || !isnan(info.gaugeHigh))) {
      const double gauge(info.gauge.lastObs());
      return (gauge >= info.gaugeLow && gauge <= info.gaugeHigh) ? Levels::OKAY : 
             (gauge <= info.gaugeLow ? Levels::LOW : 
             (gauge >= info.gaugeHigh ? Levels::HIGH : Levels::UNKNOWN));
    }
    return Levels::UNKNOWN;
  }

  std::string mkClass(const LInfo& info) {
    if (info.grade.empty()) return std::string();

    const Tokens toks(info.grade, ","); // split by comma deliminated fields
    const double flow(info.flow.lastObs());
    const double gauge(info.gauge.lastObs());

    for (Tokens::size_type i(0), e(toks.size()); i < e; ++i) {
      const Tokens fields(toks[i]);
      const Tokens::size_type n(fields.size());
      if (!n) continue;
      if (n == 1) return toks[i];
      if (fields[n-1].find_first_not_of("I-V") == std::string::npos) return toks[i];
      const bool qGauge(fields[n-1].find("ft") != std::string::npos);
      const bool qDual(fields[n-2].find_first_not_of("I-V") != std::string::npos);
      const double val0(qDual ? Convert::strTo<double>(fields[n-2]) :
                        i != 0 ? Convert::strTo<double>(fields[n-1]) : -INFINITY);
      const double val1(qDual || (i == 0) ?  Convert::strTo<double>(fields[n-1]) : INFINITY);
      std::string str(fields[0]);
      for (Tokens::size_type j(1), je(n-1-qDual); j < je; ++j) {
        str += "," + fields[j];
      }

      if ( qGauge && (val0 <= gauge) && (gauge <= val1)) return str;
      if (!qGauge && (val0 <= flow)  && (flow <= val1))  return str;
    }
    return std::string();
  }

   void mkDeltaIndex(std::ostream& os, double delta) {
    delta = round(delta);
    if (!isnan(delta) && (delta != 0)) {
      os << (delta < 0 ? "-" : "") << fmin(11,fabs(delta));
    }
  }
} // anonymous

Levels::Levels()
{
}

Levels::Levels(const std::string& state)
  : mState(state)
{
  std::ostringstream oss;
  oss << " WHERE " << fields.state() << " LIKE '%" << state << "%'";
  load(oss.str());
}

Levels::Levels(const MyDB::tInts& keys)
{
  if (keys.empty()) {
    load(std::string());
    return;
  } 
  std::ostringstream oss;
  oss << " WHERE " << fields.key();
  if (keys.size() == 1) {
    oss << "=" << keys[0];
  } else { // keys.size() > 1
    oss << " IN (" << keys << ")";
  }
  load(oss.str());
}

Levels::tStates
Levels::states()
{
  std::ostringstream oss;
  oss << "SELECT " << fields.state() << " FROM " << fields.table() << ";";
  MyDB::tStrings a(mDB.queryStrings(oss.str()));
  tStates states;
  for (MyDB::tStrings::size_type i(0), e(a.size()); i < e; ++i) {
    Tokens toks(a[i], ", \t\n");
    states.insert(toks.begin(), toks.end());
  }
  return states;
}

void
Levels::update()
{
  Data data; 
  typedef CommonSource::tSourceKey2GaugeKey tKey2Key; // map int->int
  const tKey2Key data2gauge(data.source().dataKeys(MyDB::tInts()));

  if (data2gauge.empty()) { // No data sources found
    mDB.query(std::string("DELETE FROM ") + fields.table()); // Clear out table
    return; 
  }

  MyDB::tInts dataKeys; // vector of data keys
  MyDB::tInts gaugeKeys; // vector of gauge keys

  { // populate data and gauge keys vectors
    std::set<int> gkeys;
    for (tKey2Key::const_iterator it(data2gauge.begin()), et(data2gauge.end()); it != et; ++it) {
      dataKeys.push_back(it->first); // dataKeys are unique
      gkeys.insert(it->second); // Make gaugeKeys unique
    }
    gaugeKeys = MyDB::tInts(gkeys.begin(), gkeys.end());
  }

  typedef std::vector<LInfo> tLInfo;
  tLInfo linfo;

  typedef std::multimap<int, int> tMultiKey2Key;
  tMultiKey2Key gauge2key; // gauge key to master key, one to many

  { // Load master information for gaugeKeys
    class MasterFields mf;
    MyDB::Stmt s(mDB);

    s << "SELECT " << mf.key() 
      << "," << mf.gaugeKey()
      << "," << mf.state()
      << "," << mf.displayName()
      << "," << mf.location()
      << "," << mf.sortKey()
      << "," << mf.grade()
      << "," << mf.lowFlow()
      << "," << mf.highFlow()
      << "," << mf.lowGauge()
      << "," << mf.highGauge()
      << " FROM " << mf.table() 
      << " WHERE " << mf.gaugeKey() << " IN (" << gaugeKeys 
      << ") AND qNOSHOW==0;";

    int rc;
    while ((rc = s.step()) == SQLITE_ROW) {
      LInfo info;
      info.key = s.getInt();
      info.gaugeKey = s.getInt();
      info.state = s.getString();
      info.name = s.getString();
      info.location = s.getString();
      info.sortKey = s.getString();
      info.grade = s.getString();
      info.flowLow = s.getDouble();
      info.flowHigh = s.getDouble();
      info.gaugeLow = s.getDouble();
      info.gaugeHigh = s.getDouble();
      if (info.sortKey.empty()) {
        info.sortKey = info.name;
      }
      gauge2key.insert(std::make_pair(info.gaugeKey, linfo.size())); // gaugeKey to index
      linfo.push_back(info);
    }
  }

  typedef std::pair<tMultiKey2Key::const_iterator, tMultiKey2Key::const_iterator> tPair;

  { // Get location and calc information from gauges
    Gauges gauges;
    const Gauges::tLevelInfo ginfo(gauges.levelInfo(gaugeKeys));
    for (Gauges::tLevelInfo::size_type i(0), e(ginfo.size()); i < e; ++i) {
      const Gauges::LevelInfo& a(ginfo[i]);
      tPair iters(gauge2key.equal_range(a.gaugeKey));
      for (tMultiKey2Key::const_iterator jt(iters.first), jet(iters.second); jt != jet; ++jt) {
        LInfo& info(linfo[jt->second]);
        info.qCalc = a.qCalc;
        if (info.location.empty()) {
          info.location = a.location;
        }
      }
    }
  }

  { // Update latest observations
    const int nDays(3); // Number of days to look back in time for data
    const time_t tMin(time(0) - nDays * 86400); // Only look at data for the previous nDays
    const Data::tRawObs obs(data.rawObservations(dataKeys, tMin)); // recent data
    for (Data::tRawObs::const_iterator it(obs.begin()), et(obs.end()); it != et; ++it) {
      tKey2Key::const_iterator kt(data2gauge.find(it->key)); // Look up data key to get gauge key
      if (kt == data2gauge.end()) continue; // Not found
      tPair iters(gauge2key.equal_range(kt->second));
      for (tMultiKey2Key::const_iterator jt(iters.first), jet(iters.second); jt != jet; ++jt) {
        LInfo& info(linfo[jt->second]);
        switch (it->type) {
          case Data::FLOW:
            info.flow(it->t, it->obs);
            break;
          case Data::GAUGE:
            info.gauge(it->t, it->obs);
            break;
          case Data::TEMPERATURE:
            info.temperature(it->t, it->obs);
            break;
          case Data::INFLOW:
          case Data::LASTTYPE:
            break;
        }
      }
    }
  }

  mInfo.clear(); // start a new copy
  { // populate mInfo
    std::sort(linfo.begin(), linfo.end());
    for (tLInfo::size_type i(0), e(linfo.size()); i < e; ++i) {
      const LInfo& a(linfo[i]);
      Info b;
      b.key = a.key;
      b.sortKey = mInfo.size();
      b.state = a.state;
      b.name = a.name;
      b.location = a.location;
      b.grade = mkClass(a);
      b.qCalc = a.qCalc;
      b.level = mkLevel(a);
      b.flow = a.flow.lastObs();
      b.flowTime = a.flow.lastTime();
      b.flowDelta = a.flow.delta();
      b.gauge = a.gauge.lastObs();
      b.gaugeTime = a.gauge.lastTime();
      b.gaugeDelta = a.gauge.delta();
      b.temperature = a.temperature.lastObs();
      b.temperatureTime = a.temperature.lastTime();
      b.temperatureDelta = a.temperature.delta();
      mInfo.push_back(b);
    }
  }

  { // Update levels table
    MyDB::Stmt s(mDB);
    s << "INSERT OR REPLACE INTO " << fields.table() 
      << " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";
    mDB.beginTransaction();
    mDB.query(std::string("DELETE FROM ") + fields.table()); // Clear out table
    for (size_type i(0), e(mInfo.size()); i < e; ++i) {
      const Info& info(mInfo[i]);
      s.bind(info.key);
      s.bind(info.sortKey);
      s.bind(info.state);
      s.bind(info.name);
      s.bind(info.location);
      s.bind(info.grade);
      s.bind(info.qCalc);
      s.bind(info.level);
      s.bind(info.flow);
      s.bind(info.flowTime);
      s.bind(info.flowDelta);
      s.bind(info.gauge);
      s.bind(info.gaugeTime);
      s.bind(info.gaugeDelta);
      s.bind(info.temperature);
      s.bind(info.temperatureTime);
      s.bind(info.temperatureDelta);
      s.step();
      s.reset();
    }
    mDB.endTransaction();
  }
}

void
Levels::load(const std::string& criteria)
{
  mqFlow = false;
  mqGauge = false;
  mqTemperature = false;
  mqClass = false;
  mqCalc = false;

  MyDB::Stmt s(mDB);
  s << "SELECT * FROM " << fields.table() <<  criteria << ";";

  int rc;

  typedef std::set<LInfo> tInfo;
  tInfo info;

  while ((rc = s.step()) == SQLITE_ROW) {
    Info a(s);
    mInfo.push_back(a);
    mqFlow |= !isnan(a.flow);
    mqGauge |= !isnan(a.gauge);
    mqTemperature |= !isnan(a.temperature);
    mqClass |= !a.grade.empty();
    mqCalc |= a.qCalc;
  }

  if (rc != SQLITE_DONE) {
    s.errorCheck(rc, std::string(__FILE__) + " at line " + Convert::toStr(__LINE__));
  }

  std::sort(mInfo.begin(), mInfo.end());
}

std::string
Levels::json() const
{
  std::stringstream oss;
  json(oss);
  return oss.str();
}

std::ostream& 
Levels::json(std::ostream& os) const
{
  const tInfo::size_type n(mInfo.size());

  os << "{";

  if (!n) { // empty
    os << "error:'No entries found'}";
    return os;
  }

  std::vector<time_t> times;
  time_t tMin(0);
  bool qAny(false);
  { // Build times and check for entries that are too old
    const time_t tAncient(time(0) - 14 * 86400);
    for (size_type i(0); i < n; ++i) {
      time_t t(qFlow() && !isnan(mInfo[i].flow) ? mInfo[i].flowTime : 0);
      t = qGauge() && !isnan(mInfo[i].gauge) && (t < mInfo[i].gaugeTime) 
        ? mInfo[i].gaugeTime : t;
      t = qTemperature() && !isnan(mInfo[i].temperature) && (t < mInfo[i].temperatureTime) 
        ? mInfo[i].temperatureTime : t;
      t = t < tAncient ? 0 : t;
      times.push_back(t);
      if (t > 0) {
        qAny = true;
        tMin = (tMin != 0) && (tMin < t) ? tMin : t;
      }
    }
  }

  if (!qAny) {
    os << "error:'No current entries found'}";
    return os;
  }

  { // hash
    std::string delim;
    os << "hash:['";
    for(size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        os << delim << Master::mkHash(mInfo[i].key);
        delim = "','";
      }
    }
    os << "']";
  }

  { // dump times
    std::string delim;
    os << ",tMin:" << tMin << ",tEnc:'Min',t:[";
    for(size_type i(0); i < n; ++i) {
      const time_t t(times[i]);
      if (t > 0) {
        os << delim << (t - tMin);
        delim = ",";
      }
    }
    os << "]";
  }

  { // displayName
    std::string delim;
    os << ",name:['";
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        os << delim << mInfo[i].name;
        if (!mInfo[i].location.empty()) {
          os << "@" << mInfo[i].location;
        }
        delim = "','";
      }
    }
    os << "']";
  }

  if (qFlow()) { // flow
    std::string delim;
    os << ",flow:[";
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        const double val(mInfo[i].flow);
        os << delim;
        if (!isnan(val)) os << round(val);
        delim = ",";
      }
    }
    os << "],flowDelta:[";
    delim.clear();
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        os << delim;
        mkDeltaIndex(os, mInfo[i].flowDelta);
        delim = ",";
      }
    }
    os << "]";
  }

  if (qGauge()) { // gauge
    std::string delim;
    os << ",gauge:[";
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        const double val(mInfo[i].gauge);
        os << delim;
        if (!isnan(val)) os << (round(val * 10) / 10);
        delim = ",";
      }
    }
    os << "],gaugeDelta:[";
    delim.clear();
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        os << delim;
        mkDeltaIndex(os, mInfo[i].gaugeDelta);
        delim = ",";
      }
    }
    os << "]";
  }

  if (qTemperature()) { // flow
    std::string delim;
    os << ",temperature:[";
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        const double val(mInfo[i].temperature);
        os << delim;
        if (!isnan(val)) os << round(val);
        delim = ",";
      }
    }
    os << "],temperatureDelta:[";
    delim.clear();
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        os << delim;
        mkDeltaIndex(os, mInfo[i].temperatureDelta);
        delim = ",";
      }
    }
    os << "]";
  }

  if (qClass()) {  // Class
    std::string delim;
    os << ",class:['";
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        os << delim << mInfo[i].grade;
        delim = "','";
      }
    }
    os << "']";
  }

  { // Level
    std::string delim;
    os << ",level:[";
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        os << delim;
        if (mInfo[i].level != UNKNOWN) os << mInfo[i].level;
        delim = ",";
      }
    }
    os << "]";
  }

  if (qCalc()) { // qCalc
    std::string delim;
    os << ",qCalc:[";
    for (size_type i(0); i < n; ++i) {
      if (times[i] > 0) {
        os << delim << (mInfo[i].qCalc ? "1" : "");
        delim = ",";
      }
    }
    os << "]";
  }


  if (!mState.empty()) {
    os << ",state:'" << mState << "'";
  }

  os << "}";

  return os;
}

Levels::Info::Info()
  : sortKey(-1)
  , qCalc(false)
  , level(Levels::UNKNOWN)
  , flow(NAN)
  , flowTime(0)
  , flowDelta(NAN)
  , gauge(NAN)
  , gaugeTime(0)
  , gaugeDelta(NAN)
  , temperature(NAN)
  , temperatureTime(0)
  , temperatureDelta(NAN)
{
}

Levels::Info::Info(MyDB::Stmt& s)
  : key(s.getInt())
  , sortKey(s.getInt())
  , state(s.getString())
  , name(s.getString())
  , location(s.getString())
  , grade(s.getString())
  , qCalc(s.getInt())
  , level((Levels::Level) s.getInt())
  , flow(s.getDouble())
  , flowTime(s.getInt())
  , flowDelta(s.getDouble())
  , gauge(s.getDouble())
  , gaugeTime(s.getInt())
  , gaugeDelta(s.getDouble())
  , temperature(s.getDouble())
  , temperatureTime(s.getInt())
  , temperatureDelta(s.getDouble())
{
}

bool
Levels::Info::operator < (const Info& rhs) const
{
  return (sortKey < rhs.sortKey) ||
         ((sortKey == rhs.sortKey) && (key < rhs.key));
}
