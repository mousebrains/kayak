#include "Gauges.H"
#include "MyDB.H"
#include "Fields.H"
#include "Convert.H"
#include <iostream>
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
    const char *idUnit() {return "idUnit";}
    const char *location() {return "location";}
    const char *state() {return "state";}
    const char *county() {return "county";}
    const char *elevation() {return "elevation";}
    const char *drainageArea() {return "drainageArea";}
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

  std::string whereKeys(const MyDB::tInts& keys) {
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

  bool withinLimits(const double a, const double lLim, const double uLim) {
    return !isnan(a) && (a >= lLim) && (a <= uLim);
  }
} // anonymous

Gauges::Gauges()
  : Table(fields)
{
}

void
Gauges::latLon(const int key,
               const double lat,
               const double lon)
{
  if ((lat >= -90) && (lat <= 90) && (lon >= -180) && (lon <= 180)) {
    MyDB::Stmt s(mDB);
    s << "UPDATE " << fields.table() << " SET " 
      << fields.latitude() << "=" << lat 
      << ","
      << fields.longitude() << "=" << lon
      << " WHERE " << fields.key() << "=" << key << ";";
    s.query();
  }
}

void Gauges::idUSGS(const int key, const std::string& a) { updateString(key, a, fields.idUSGS()); }
void Gauges::idCBTT(const int key, const std::string& a) { updateString(key, a, fields.idCBTT()); }
void Gauges::idUnit(const int key, const std::string& a) { updateString(key, a, fields.idUnit()); }
void Gauges::location(const int key, const std::string& a) { updateString(key, a, fields.location()); }
void Gauges::state(const int key, const std::string& a) { updateString(key, a, fields.state()); }
void Gauges::county(const int key, const std::string& a) { updateString(key, a, fields.county()); }
void Gauges::description(const int key, const std::string& a) { updateString(key, a, fields.description()); }
void Gauges::elevation(const int key, const double a) { updateDouble(key, a, fields.elevation()); }
void Gauges::drainageArea(const int key, const double a) { updateDouble(key, a, fields.drainageArea()); }

void
Gauges::updateString(const int key,
                     const std::string& value,
                     const std::string& field)
{
  if (!value.empty()) {
    MyDB::Stmt s(mDB);
    s << "UPDATE " << fields.table() << " SET " << field << "='" << value 
      << "' WHERE " << fields.key() << "=" << key << ";";
    s.query();
  }
}

void
Gauges::updateDouble(const int key,
                     const double value,
                     const std::string& field)
{
  if (!isnan(value)) {
    MyDB::Stmt s(mDB);
    s << "UPDATE " << fields.table() << " SET " << field << "=" << value 
      << " WHERE " << fields.key() << "=" << key << ";";
    s.query();
  }
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
    MyDB::tDoubles items(s.queryDoubles());

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
  , idUnit(s.getString())
  , state(s.getString())
  , county(s.getString())
  , elevation(s.getDouble())
  , drainageArea(s.getDouble())
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

Gauges::tLevelInfo
Gauges::levelInfo(const MyDB::tInts& gaugeKeys)
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
