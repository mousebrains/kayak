#include "StateCounty.H"
#include "String.H"
#include "String.H"
#include <iostream>

namespace {
  struct MyFields {
    const char *table() const {return "countyCodes";}
    const char *countyKey() const {return "countyKey";}
    const char *stateKey() const {return "stateKey";}
    const char *state() const {return "state";}
    const char *county() const {return "county";}
  } fields;

  std::string pruneCounty(const std::string& county) {
    const std::string key("county");
    const std::string::size_type i(String::tolower(county).find(key));
    if (i != (county.size() - key.size())) { // not at end of string
      return county;
    }
    return String::trim(county.substr(0,i)); // Trim off any trailing whitespace
  }
} // anonymous

std::string
StateCounty::state(const int code)
{
  tMap::const_iterator it(mStates.find(code));
  if (it == mStates.end()) {
    MyDB::Stmt s(mDB);
    s << "SELECT DISTINCT " << fields.state() << " FROM " << fields.table()
      << " WHERE " << fields.stateKey() << "=" << code << ";";
    MyDB::Stmt::tStrings a(s.queryStrings());
    it = mStates.insert(std::make_pair(code, a.empty() ? std::string() : a[0])).first;
  }
  return it->second;
}

std::string
StateCounty::county(const int state,
                    const int county)
{
  const int code(county * 1000 + state);
  tMap::const_iterator it(mCounties.find(code));
  if (it == mCounties.end()) {
    MyDB::Stmt s(mDB);
    s << "SELECT DISTINCT " << fields.county() << " FROM " << fields.table()
      << " WHERE " << fields.stateKey() << "=" << state 
      << " AND " << fields.countyKey() << "=" << county
      << ";";
    MyDB::Stmt::tStrings a(s.queryStrings());
    it = mCounties.insert(std::make_pair(code, a.empty() ? std::string() : pruneCounty(a[0]))).first;
  }
  return it->second;
}
