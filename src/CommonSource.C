#include "CommonSource.H"
#include "Gauges.H"
#include "Convert.H"
#include <sstream>

CommonSource::CommonSource(const std::string& prefix)
  : Table(SourceFields(prefix))
  , mFields(prefix)
{
}

CommonSource::~CommonSource()
{
  if (!mKeyMap.empty()) {
    Gauges gauges;
    MyDB::Stmt s(mDB);
    s << "UPDATE " << mFields.table() << " SET "
      << mFields.gaugeKey() << "=? WHERE " << mFields.key() << "=?;";
    mDB.beginTransaction();

    for (tKeyMap::const_iterator it(mKeyMap.begin()), et(mKeyMap.end()); it != et; ++it) {
      const int gKey(gauges.name2key(it->second));
      if (gKey > 0) {
        const int pKey(name2key(it->first, 0));
        s.bind(gKey);
        s.bind(pKey);
        s.step();
        s.reset();
      }
    }
    mDB.endTransaction();
  }
}

void
CommonSource::setGauge(const std::string& name,
                       const std::string& gauge)
{
  mKeyMap.insert(std::make_pair(name, gauge));
}

MyDB::Stmt::tInts
CommonSource::gaugeKey2Keys(const size_t gaugeKey)
{
  MyDB::Stmt s(mDB);
  s << "SELECT " << mFields.key() 
    << " FROM " << mFields.table()
    << " WHERE " << mFields.gaugeKey() << "=" << gaugeKey << ";";

  return s.queryInts();
}

CommonSource::tSourceKey2GaugeKey
CommonSource::dataKeys(const MyDB::Stmt::tInts& gaugeKeys)
{
  MyDB::Stmt s(mDB);

  s << "SELECT " << mFields.key()
    << "," << mFields.gaugeKey()
    << " FROM " << mFields.table();

  if (gaugeKeys.size() == 1) {
    s << " WHERE " << mFields.gaugeKey() << "=" << gaugeKeys[0];
  } else if (gaugeKeys.size() > 1) {
    s << " WHERE " << mFields.gaugeKey() << " IN (" << gaugeKeys << ")";
  }
  s << ";";

  int rc;
  tSourceKey2GaugeKey info;

  while ((rc = s.step()) == SQLITE_ROW) {
    info.insert(std::make_pair(s.getInt(0), s.getInt(1)));
  }

  if (rc != SQLITE_DONE) {
    s.errorCheck(rc, std::string(__FILE__) + " at line " + Convert::toStr(__LINE__));
  }

  return info;
}
