#include "Data.H"
#include "Gauges.H"
#include "Convert.H"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>

namespace {
  class MyFields : public DataFields {
  public:
    MyFields() : DataFields("data") {}
    const char *obs() const {return "obs";}
    const char *type() const {return "type";}
  } fields;

  std::string whereKeysTime(const MyDB::tInts keys, const time_t tMin=0, const time_t tMax=0) {
    if (keys.empty() && (tMin <= 0) && (tMax <= 0)) {
      return std::string();
    }

    bool qAnd(false);
    std::ostringstream oss;
    oss << " WHERE ";
    if (keys.size() == 1) {
      oss << fields.key() << "=" << keys[0];
      qAnd = true;
    } else {
      oss << fields.key() << " IN (" << keys << ")";
      qAnd = true;
    }
    if (tMin > 0) {
      if (qAnd) oss << " AND";
      oss << " " << fields.time() << ">=" << tMin;
      qAnd = true;
    }
    if (tMax < 0) {
      if (qAnd) oss << " AND";
      oss << " " << fields.time() << "<=" << tMax;
    } 
    return oss.str();
  } // whereKeysTime
}

Data::Data()
  : CommonData(fields)
{
}

Data::~Data()
{
  if (!mData.empty()) {
    dump();
  }
}
 
void
Data::add(const std::string& name,
          const time_t t,
          const double value,
          const Type tNum,
          const std::string& url)
{
  if (mData.size() > 1000000) { // Don't let it grow too big
    dump();
  }

  mData.push_back(Datum(name, t, value, tNum, url));
}

void
Data::dump()
{
  Gauges gauges;

  std::cout << "Dumping " << mData.size() << std::endl;

  MyDB::Stmt s(mDB);

  s << "REPLACE INTO " << fields.table() << " (" 
    << fields.key() << "," 
    << fields.type() << "," 
    << fields.time() << "," 
    << fields.urlKey() << ","
    << fields.obs() 
    << ") VALUES (?,?,?,?,?);";

  mDB.beginTransaction();

  for (tData::size_type i(0), e(mData.size()); i < e; ++i) {
    const Datum& datum(mData[i]);
    const int key(mSource.name2key(datum.name, datum.t));
    const int url(mURLs.name2key(datum.url, datum.t));

    if (gauges.chkLimits(key, datum.tNum, datum.value)) {
      s.bind(key);
      s.bind(datum.tNum);
      s.bind(datum.t);
      s.bind(url);
      s.bind(datum.value);
      s.step();
      s.reset();
    }
  }

  mDB.endTransaction();

  // Clear out existing information
  mData.clear();
}

Data::tTimeKeys
Data::timeKeys(const int key,
               const Type t)
{
  MyDB::Stmt s(mDB);
  s << "SELECT " << fields.time() << ",rowid FROM " << fields.table()
    << " WHERE " << fields.key() << "=" << key
    << " AND " << fields.type() << "=" << (int) t << ";";
  MyDB::tInts info(s.queryInts());

  tTimeKeys tk;

  for (MyDB::tInts::const_iterator it(info.begin()), et(info.end()); it != et; ++it) {
    const time_t t(*it);
    const size_t r(*(++it));
    tk.insert(std::make_pair(t, r));
  }

  return tk;
}

Data::tObs
Data::observations(const MyDB::tInts& keys, 
                   const Type t, 
                   const time_t tMin,
                   const time_t tMax)
{
  MyDB::Stmt s(mDB);

  s << "SELECT " << fields.time() << ",AVG(" << fields.obs()
    << ") FROM " << fields.table()
    << " WHERE " << fields.key();

  if (keys.size() == 1) {
    s << "=" << keys[0];
  } else {
    s << " IN (" << keys << ")";
  }

 
  if (tMin != 0) {
    s << " AND " << fields.time() << ">=" << tMin;
  }
  if (tMax != 0) {
    s << " AND " << fields.time() << "<=" << tMax;
  }
     
  s << " AND " << fields.type() << "=" << (int) t 
    << " GROUP BY " << fields.time()
    << ";";

  tObs obs;
  int rc;
  while ((rc = s.step()) == SQLITE_ROW) {
    obs.insert(std::make_pair((size_t) s.getInt(0), s.getDouble(1)));
  }
   
  if (rc != SQLITE_DONE) {
    s.errorCheck(rc, std::string(__FILE__) + " at line " + Convert::toStr(__LINE__));
  }

  return obs;
}

MyDB::tInts
Data::types(const MyDB::tInts& keys) // Which types are know for these source keys
{
  MyDB::Stmt s(mDB);
  s << "SELECT DISTINCT " << fields.type() << " FROM "
    << fields.table() 
    << whereKeysTime(keys)
    << ";";

  MyDB::tInts result(s.queryInts());

  std::sort(result.begin(), result.end());

  return result;
}

Data::AnObs::AnObs()
  : key(0)
  , time(0)
  , flow(NAN)
  , gauge(NAN)
  , temperature(NAN)
{
}

bool
Data::AnObs::operator <(const AnObs& rhs) const
{
  return (time < rhs.time) ||
         ((time == rhs.time) && 
          ((key < rhs.key) ||
           ((key == rhs.key) &&
            ((name < rhs.name) || 
            ((name == rhs.name) && (url < rhs.url))))));
}

Data::tAnObs
Data::observations(const MyDB::tInts& keys, 
                   const time_t tMin, 
                   const time_t tMax)
{
  MyDB::Stmt s(mDB);
  s << "SELECT " 
    << fields.key() 
    << "," << fields.time() 
    << "," << fields.type()
    << "," << fields.urlKey()
    << "," << fields.obs()
    << " FROM " << fields.table()
    << whereKeysTime(keys, tMin, tMax)
    << ";";

  tAnObs obs;
  int rc;
  while ((rc = s.step()) == SQLITE_ROW) {
    const int key(s.getInt());
    const time_t t(s.getInt());
    Data::Type type((Data::Type) s.getInt());
    const int urlKey(s.getInt());
    const double val(s.getDouble());
    if (key <= 0) {
      continue;
    }
    AnObs a;
    a.key = key;
    a.time = t;
    a.name = mSource.key2name(key);
    if (urlKey > 0) {
      a.url = mURLs.key2name(urlKey);
    }
    tAnObs::iterator it(obs.find(a));
    if (it == obs.end()) {
      it = obs.insert(a).first;
    }
    switch (type) {
      case FLOW: it->flow = val; break;
      case GAUGE: it->gauge = val; break;
      case TEMPERATURE: it->temperature = val; break;
      case INFLOW:
      case LASTTYPE: break;
    }
  }

  if (rc != SQLITE_DONE) {
    s.errorCheck(rc, std::string(__FILE__) + " at line " + Convert::toStr(__LINE__));
  }

  return obs;
}

Data::tRawObs
Data::rawObservations(const MyDB::tInts& keys, 
                      const time_t tMin,
                      const time_t tMax)
{
  MyDB::Stmt s(mDB);

  s << "SELECT " 
    << fields.key()
    << "," << fields.time()
    << "," << fields.type()
    << "," << fields.obs()
    << " FROM " << fields.table()
    << whereKeysTime(keys, tMin, tMax)
    << ";";

  tRawObs obs;
  int rc;

  while ((rc = s.step()) == SQLITE_ROW) {
    RawObs a(s.getInt(0), // key
             s.getInt(1), // time 
             (Type) s.getInt(2), // type 
             s.getDouble(3)); // obs 
    obs.push_back(a);
  }
  
  return obs;
}
