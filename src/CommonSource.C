#include "CommonSource.H"
#include "Gauges.H"
#include "Convert.H"
#include <sstream>

CommonSource::CommonSource(const std::string& prefix)
  : Table(SourceFields(prefix))
  , mFields(prefix)
{
}

void
CommonSource::setGauge(const std::string& name,
                       const int gaugeKey)
{
  if (!name.empty() && (gaugeKey > 0)) {
    MyDB::Stmt s(mDB);
    s << "INSERT OR REPLACE INTO " << mFields.table() 
      << " (" << mFields.name() << "," << mFields.gaugeKey() 
      << ") VALUES ('"
      << name << "'," << gaugeKey
      << ");";
    s.query();
  }
}

MyDB::tInts
CommonSource::gaugeKey2Keys(const size_t gaugeKey)
{
  MyDB::Stmt s(mDB);
  s << "SELECT " << mFields.key() 
    << " FROM " << mFields.table()
    << " WHERE " << mFields.gaugeKey() << "=" << gaugeKey << ";";

  return s.queryInts();
}

CommonSource::tSourceKey2GaugeKey
CommonSource::dataKeys(const MyDB::tInts& gaugeKeys)
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
