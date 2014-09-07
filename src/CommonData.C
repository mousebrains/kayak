#include "CommonData.H"
#include "Gauges.H"
#include "Convert.H"
#include <iostream>
#include <sstream>
#include <cstdlib>

CommonData::~CommonData()
{
}

CommonData::Type 
CommonData::decodeType(const std::string& str)
{
  const std::string s(Convert::tolower(str));

  if ((s.find("flow") == 0) || (s == "f")) {
    return FLOW;
  } else if ((s.find("gauge") == 0) || (s == "g")) {
    return GAUGE;
  } else if ((s.find("temp") == 0) || (s == "t")) {
    return TEMPERATURE;
  }
  return LASTTYPE;
}

CommonData::tKeys
CommonData::allKeys()
{
  MyDB::Stmt s(mDB);
  s << "SELECT DISTINCT " << mFields.key() << " FROM " << mFields.table() << ";";

  MyDB::Stmt::tInts k(s.queryInts());
  tKeys keys;

  keys.insert(k.begin(), k.end());
  return keys; 
}

void
CommonData::dropRows(const tKeys& keys)
{
  mDB.dropRows(mFields.table(), keys);
}

size_t 
CommonData::nRows(const int key)
{
  MyDB::Stmt s(mDB);
  s << "SELECT count(*) FROM " << mFields.table() 
    << " WHERE " << mFields.key() << "=" << key << ";";
  MyDB::Stmt::tInts a(s.queryInts());
  return a.empty() ? 0 : (size_t) a[0];
}

std::ostream&
operator << (std::ostream& os,
             const CommonData::Type t)
{
  switch (t) {
    case CommonData::FLOW:
      os << "flow";
      break;
    case CommonData::GAUGE:
      os << "gauge";
      break;
    case CommonData::TEMPERATURE:
      os << "temp";
      break;
    case CommonData::INFLOW:
      os << "inflow";
      break;
    case CommonData::LASTTYPE:
      os << "????";
      break;
  }
  return os;
}
