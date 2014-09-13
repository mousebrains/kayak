#include "Gauges.H"
#include "MyDB.H"
#include "Fields.H"
#include "Convert.H"
#include <iomanip>
#include <sstream>
#include <cmath>

namespace {
  class MyFields : public Fields {
  public:
    MyFields() : Fields("gauges", "gaugeKey") {}

    const char *latitude() {return "latitude";}
    const char *longitude() {return "longitude";}
    const char *idUSGS() {return "idUSGS";}
    const char *idCBTT() {return "idCBTT";}
    const char *idUSBR() {return "idUSBR";}
    const char *idUnit() {return "idUnit";}
    const char *location() {return "location";}
    const char *state() {return "state";}
    const char *county() {return "county";}
    const char *elevation() {return "elevation";}
    const char *drainageArea() {return "drainageArea";}
    const char *bankfullStage() {return "bankfullStage";}
    const char *floodStage() {return "floodStage";}
    const char *description() {return "description";}
    const char *minFlow() {return "minFlow";}
    const char *maxFlow() {return "maxFlow";}
    const char *minGauge() {return "minGauge";}
    const char *maxGauge() {return "maxGauge";}
    const char *minTemperature() {return "minTemperature";}
    const char *maxTemperature() {return "maxTemperature";}
    const char *calcFlow() {return "calcFlow";}
    const char *calcGauge() {return "calcGauge";}
  };
  MyFields fields;

  std::string whereKeys(const MyDB::Stmt::tInts& keys) {
    if (keys.empty()) return std::string();
    std::ostringstream oss;
    oss << " WHERE " << fields.key();
    if (keys.size() == 1) {
      oss << "=" << keys[0];
    } else {
      oss << " IN (" << keys << ")";
    }
    return oss.str();
  }

  int maybeInfo(std::ostream& os0, std::ostream& os1, const std::string& name, 
                const std::string& value) {
    if (value.empty()) return 0;
    os0 << "," << name;
    os1 << "," << MyDB::Stmt::quotedString(value);
    return 1;
  }

  int maybeInfo(std::ostream& os0, std::ostream& os1, const std::string& name, const time_t t) {
    if (t <= 0) return 0;
    os0 << "," << name;
    os1 << "," << t;
    return 1;
  }

  int maybeInfo(std::ostream& os0, std::ostream& os1, const std::string& name, const double x) {
    if (isnan(x)) return 0;
    os0 << "," << name;
    os1 << "," << std::setprecision(12) << x; // up to 12 digits after the decimal point
    return 1;
  }
} // anonymous

Gauges::Gauges()
  : Table(fields)
{
}

int
Gauges::idUSGS2gaugeKey(const std::string& id)
{
  return id2gaugeKey(id, fields.idUSGS());
}

int
Gauges::idCBTT2gaugeKey(const std::string& id)
{
  return id2gaugeKey(id, fields.idCBTT());
}

int
Gauges::id2gaugeKey(const std::string& id,
                    const std::string& name)
{
  MyDB::Stmt s(mDB);
  s << "SELECT " << fields.key() << " FROM " << fields.table()
    << " WHERE " << name << "='" << id << "' COLLATE NOCASE;";
  const MyDB::Stmt::tInts a(s.queryInts());
  return a.empty() ? 0 : a[0];
}

bool
Gauges::chkLimits(const int key,
                  const Data::Type type,
                  const double val)
{
  if ((key <= 0) | isnan(val)) return false;

  tLimits::const_iterator it(mLimits.find(key));

  if (it == mLimits.end()) {
    MyDB::Stmt s(mDB);
    s << "SELECT " 
      << fields.minFlow() << "," << fields.maxFlow()
      << "," << fields.minGauge() << "," << fields.maxGauge()
      << "," << fields.minTemperature() << "," << fields.maxTemperature()
      << " FROM " << fields.table() 
      << " WHERE " << fields.key() << "=" << key << ";";
    MyDB::Stmt::tDoubles items(s.queryDoubles());

    if (items.size() >= 6) {
      Limits l(items[0], items[1], items[2], items[3], items[4], items[5]);
      it = mLimits.insert(std::make_pair(key, l)).first;
    } else {
      it = mLimits.insert(std::make_pair(key, Limits())).first;
    }
  }

  const Limits& lim(it->second);

  switch (type) {
    case Data::INFLOW:
    case Data::FLOW: return (val >= lim.minFlow) && (val <= lim.maxFlow);
    case Data::GAUGE: return (val >= lim.minGauge) && (val <= lim.maxGauge);
    case Data::TEMPERATURE: return (val >= lim.minTemperature) && (val <= lim.maxTemperature);
    case Data::LASTTYPE: return false;
  }
  return false;
}

Gauges::PlotInfo
Gauges::plotInfo(const size_t key)
{
  if (key == 0) {
    return PlotInfo();
  }

  MyDB::Stmt s(mDB);

  s << "SELECT " 
    << fields.name() << "," << fields.location()
    << "," << fields.latitude() << "," << fields.longitude()
    << " FROM " << fields.table() 
    << " WHERE " << fields.key() << "=" << key << ";";

  int rc(s.step());

  if (rc == SQLITE_ROW) {
    return PlotInfo(s.getString(0), s.getString(1),
                    s.getDouble(2), s.getDouble(3));
  }

  s.errorCheck(rc, std::string(__FILE__) + " line " + Convert::toStr(__LINE__));
  return PlotInfo("", "", 1e9, 1e9);
}

std::ostream&
operator << (std::ostream& os,
             const Gauges::Limits& a)
{
  os << a.minFlow 
     << "," << a.maxFlow
     << "," << a.minGauge
     << "," << a.maxGauge
     << "," << a.minTemperature
     << "," << a.maxTemperature
     ;
  return os;
}

Gauges::Info::Info()
  : gaugeKey(0)
  , date(0)
  , latitude(1e9)
  , longitude(1e9)
  , elevation(NAN)
  , drainageArea(NAN)
  , bankfullStage(NAN)
  , floodStage(NAN)
  , minFlow(NAN)
  , maxFlow(NAN)
  , minGauge(NAN)
  , maxGauge(NAN)
  , minTemperature(NAN)
  , maxTemperature(NAN)
{
}

Gauges::Info::Info(MyDB::Stmt& s)
  : gaugeKey(s.getInt())
  , date(s.getInt())
  , name(s.getString())
  , latitude(s.getDouble())
  , longitude(s.getDouble())
  , description(s.getString())
  , location(s.getString())
  , idUSGS(s.getString())
  , idCBTT(s.getString())
  , idUSBR(s.getString())
  , idUnit(s.getString())
  , state(s.getString())
  , county(s.getString())
  , elevation(s.getDouble())
  , drainageArea(s.getDouble())
  , bankfullStage(s.getDouble())
  , floodStage(s.getDouble())
  , minFlow(s.getDouble())
  , maxFlow(s.getDouble())
  , minGauge(s.getDouble())
  , maxGauge(s.getDouble())
  , minTemperature(s.getDouble())
  , maxTemperature(s.getDouble())
  , calcFlow(s.getString())
  , calcGauge(s.getString())
{
}

Gauges::Info
Gauges::getInfo(const size_t key)
{
  MyDB::Stmt s(mDB);

  s << "SELECT * FROM " << fields.table()
    << " WHERE " << fields.key() << "=" << key << ";";

  int rc(s.step());

  if (rc == SQLITE_ROW) {
    Info info(s);
    return info;
  }

  s.errorCheck(rc, std::string(__FILE__) + " line " + Convert::toStr(__LINE__));
  return Info();
}

Gauges::tInfo
Gauges::getAll()
{
  MyDB::Stmt s(mDB);

  s << "SELECT * FROM " << fields.table() << ";";

  tInfo info;
  int rc;
 
  while ((rc = s.step()) == SQLITE_ROW) {
    info.insert(Info(s));
    return info;
  }

  if (rc != SQLITE_DONE) {
    s.errorCheck(rc, std::string(__FILE__) + " line " + Convert::toStr(__LINE__));
  }
  return info;
}

void
Gauges::putInfo(const Info& info)
{
  if (info.gaugeKey <= 0) return; // Invalid gaugeKey

  MyDB::Stmt s(mDB);
  std::ostringstream oss;

  s << "INSERT OR REPLACE INTO " << fields.table() << " (" << fields.key();
  oss << info.gaugeKey;

  int n(0); // Number of fields to set

  n += maybeInfo(s, oss, fields.time(), info.date);
  n += maybeInfo(s, oss, fields.name(), info.name);
  n += maybeInfo(s, oss, fields.latitude(), info.latitude);
  n += maybeInfo(s, oss, fields.longitude(), info.longitude);
  n += maybeInfo(s, oss, fields.description(), info.description);
  n += maybeInfo(s, oss, fields.location(), info.location);
  n += maybeInfo(s, oss, fields.idUSGS(), info.idUSGS);
  n += maybeInfo(s, oss, fields.idCBTT(), info.idCBTT);
  n += maybeInfo(s, oss, fields.idUSBR(), info.idUSBR);
  n += maybeInfo(s, oss, fields.idUnit(), info.idUnit);
  n += maybeInfo(s, oss, fields.state(), info.state);
  n += maybeInfo(s, oss, fields.county(), info.county);
  n += maybeInfo(s, oss, fields.elevation(), info.elevation);
  n += maybeInfo(s, oss, fields.drainageArea(), info.drainageArea);
  n += maybeInfo(s, oss, fields.bankfullStage(), info.bankfullStage);
  n += maybeInfo(s, oss, fields.floodStage(), info.floodStage);
  n += maybeInfo(s, oss, fields.minFlow(), info.minFlow);
  n += maybeInfo(s, oss, fields.maxFlow(), info.maxFlow);
  n += maybeInfo(s, oss, fields.minGauge(), info.minGauge);
  n += maybeInfo(s, oss, fields.maxGauge(), info.maxGauge);

  if (!n) return; // Nothing to do

  s << ") VALUES(" << oss.str() << ");";
  s.query();
}

void
Gauges::putInfo(const tInfo& info)
{
  mDB.beginTransaction();
  for (tInfo::const_iterator it(info.begin()), et(info.end()); it != et; ++it) {
    putInfo(*it);
  }
  mDB.endTransaction();
}

Gauges::tLevelInfo
Gauges::levelInfo(const MyDB::Stmt::tInts& gaugeKeys)
{
  MyDB::Stmt s(mDB);

  s << "SELECT " << fields.key() 
    << "," << fields.location()
    << ",((" 
    << fields.calcFlow() << " IS NOT NULL)OR(" 
    << fields.calcGauge() << " IS NOT NULL))"
    << " FROM " << fields.table()
    << whereKeys(gaugeKeys) 
    << ";";

  tLevelInfo info;
  int rc;

  while ((rc = s.step()) == SQLITE_ROW) {
    LevelInfo a;
    a.gaugeKey = s.getInt();
    a.location = s.getString();
    a.qCalc = s.getInt();
    info.push_back(a);
  }
  if (rc != SQLITE_DONE) {
    s.errorCheck(rc, std::string(__FILE__) + " line " + Convert::toStr(__LINE__));
  }
  return info;
}
