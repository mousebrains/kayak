#include "Master.H"
#include "Tokens.H"
#include "Convert.H"
#include <sstream>
#include <ctime>
#include <iostream>
#include <cmath>

namespace {
  class MasterFields fields; // Short cut
  const std::string hash("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
}

Master::tSet
Master::allStates()
{
  std::ostringstream oss;
  oss << "SELECT DISTINCT " << fields.state() << " FROM " << fields.table() << ";";
  MyDB::tStrings result(mDB.queryStrings(oss.str()));

  tSet a;

  for (MyDB::tStrings::const_iterator it(result.begin()), et(result.end()); it != et; ++it) {
    const Tokens toks(*it);
    a.insert(toks.begin(), toks.end());
  }
  return a;
}

bool
Master::qModifiedSince(const time_t t)
{
  std::ostringstream oss;
  oss << "SELECT max(" << fields.modified() << ") FROM " << fields.table() << ";";
  MyDB::tInts result(mDB.queryInts(oss.str()));

  return result.empty() ? true : ((t-60) <= (time_t) result[0]);
}

size_t
Master::gaugeKey(const size_t key)
{
  std::ostringstream oss;
  oss << "SELECT " << fields.gaugeKey() << " FROM " << fields.table()
      << " WHERE " << fields.key() << "=" << key << ";";
  MyDB::tInts result(mDB.queryInts(oss.str()));
  return (result.empty() || (result[0] < 0)) ? 0 : result[0];
}

Master::PlotInfo
Master::plotInfo(const size_t key,
                 const Data::Type type)
{
  bool qFlow(false);
  bool qGauge(false);

  MyDB::Stmt s(mDB);

  s << "SELECT " << fields.gaugeKey() 
    << "," << fields.displayName() 
    << "," << fields.location();

  switch (type) {
    case Data::FLOW: 
    case Data::INFLOW: 
      qFlow = true;
      s << "," << fields.lowFlow() 
        << "," << fields.highFlow() 
        << "," << fields.optimalLowFlow() 
        << "," << fields.optimalHighFlow();
      break;
    case Data::GAUGE: 
      qGauge = true;
      s << "," << fields.lowGauge() 
        << "," << fields.highGauge();
      break;
    case Data::TEMPERATURE: 
    case Data::LASTTYPE:
      break;
  }

  s << "," << fields.latitudePutin() 
    << "," << fields.longitudePutin() 
    << " FROM " << fields.table()
    << " WHERE " << fields.key() << "=" << key << ";";

  int rc(s.step());

  if (rc == SQLITE_ROW) {
    const int gaugeKey(s.getInt());
    const std::string displayName(s.getString());
    const std::string location(s.getString());
    const double low (qFlow || qGauge ? s.getDouble() : 0);
    const double high(qFlow || qGauge ? s.getDouble() : 0);
    const double lowOptimal (qFlow ? s.getDouble() : 0);
    const double highOptimal(qFlow ? s.getDouble() : 0);
    const double lat(s.getDouble());
    const double lon(s.getDouble());
    return PlotInfo(gaugeKey, displayName, location, low, high, lowOptimal, highOptimal, lat, lon);
  }

  s.errorCheck(rc, std::string(__FILE__) + " line " + Convert::toStr(__LINE__));
  return PlotInfo();
}

size_t 
Master::deHash(const std::string& str)
{
  size_t val(0);

  for (int i(str.size()-1), n(hash.size()); i >= 0; --i) {
    val *= n;
    val += hash.find(str[i]);
  }

  return val;
}

std::string
Master::mkHash(size_t val)
{
  if (val == 0) {
    return std::string().append(1,hash[0]);
  }

  std::string str;

  for (size_t n(hash.size()); val != 0; val /= n) {
    str.append(1, hash[val % n]); 
  }

  return str;
}

Master::Info::Info()
  : key(0)
  , qNoShow(false)
  , created(0)
  , modified(0)
  , drainageArea(NAN)
  , elevation(NAN)
  , elevationLost(NAN)
  , gradient(NAN)
  , latitudePutin(NAN)
  , longitudePutin(NAN)
  , latitudeTakeout(NAN)
  , longitudeTakeout(NAN)
  , length(NAN)
  , lowFlow(NAN)
  , highFlow(NAN)
  , lowGauge(NAN)
  , highGauge(NAN)
  , optimalLowFlow(NAN)
  , optimalHighFlow(NAN)
  , gaugeKey(0)
{
}

Master::Info::Info(MyDB::Stmt& s)
  : key(s.getInt())
  , qNoShow(s.getInt())
  , created(s.getInt())
  , modified(s.getInt())
  , nature(s.getString())
  , classString(s.getString())
  , difficulties(s.getString())
  , displayName(s.getString())
  , drainage(s.getString())
  , drainageArea(s.getDouble())
  , elevation(s.getDouble())
  , elevationLost(s.getDouble())
  , features(s.getString())
  , gradient(s.getDouble())
  , latitudePutin(s.getDouble())
  , longitudePutin(s.getDouble())
  , latitudeTakeout(s.getDouble())
  , longitudeTakeout(s.getDouble())
  , location(s.getString())
  , length(s.getDouble())
  , lowFlow(s.getDouble())
  , highFlow(s.getDouble())
  , lowGauge(s.getDouble())
  , highGauge(s.getDouble())
  , optimalLowFlow(s.getDouble())
  , optimalHighFlow(s.getDouble())
  , notes(s.getString())
  , region(s.getString())
  , remoteness(s.getString())
  , riverName(s.getString())
  , scenery(s.getString())
  , season(s.getString())
  , section(s.getString())
  , sortKey(s.getString())
  , state(s.getString())
  , watershedType(s.getString())
  , gaugeKey(s.getInt())
  , gaugeName(s.getString())
{
}

Master::Info
Master::getInfo(const size_t key)
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

Master::tInfo
Master::getLike(const size_t key)
{
  const Info ref(getInfo(key));

  tInfo info;
  std::ostringstream oss;

  oss << "SELECT * FROM " << fields.table() 
      << " WHERE " << fields.riverName() << " LIKE '" << ref.riverName << "';";;

  return getLike(oss.str());
}

Master::tInfo
Master::getLike(const std::string& sql)
{
  MyDB::Stmt s(mDB, sql);
  tInfo info;
  int rc;
 
  while ((rc = s.step()) == SQLITE_ROW) {
    info.push_back(Info(s));
    return info;
  }

  if (rc == SQLITE_DONE) {
    return info;
  }

  s.errorCheck(rc, std::string(__FILE__) + " line " + Convert::toStr(__LINE__));
  return info;
}

Master::tInfo
Master::getAll()
{
  std::ostringstream oss;

  oss << "SELECT * FROM " << fields.table();
  return getLike(oss.str());
}

MyDB::tInts
Master::stateKeysWithGauges(const std::string& state)
{
  std::ostringstream oss;
  oss << "SELECT " << fields.key() 
      << " FROM " << fields.table() 
      << " WHERE " << fields.state() << " like '%" << state << "%' AND "
      << fields.gaugeKey() << "!=0;";
        
  return mDB.queryInts(oss.str());
}

MyDB::tInts
Master::keysWithGauges(const MyDB::tInts& keys)
{
  std::ostringstream oss;
  oss << "SELECT " << fields.key() 
      << " FROM " << fields.table() 
      << " WHERE ";

  if (keys.size() == 1) {
    oss << fields.key() << "=" << keys[0] << " ";
  } else if (keys.size() > 1) {
    oss << fields.key() << "in (" << keys << ") ";
  }

  oss << fields.gaugeKey() << "!=0;";
        
  return mDB.queryInts(oss.str());
}
