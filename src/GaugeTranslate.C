#include "GaugeTranslate.H"

namespace {
  class MyFields {
  public:
    const char *table() {return "gaugeTranslate";}
    const char *str() {return "str";}
    const char *description() {return "description";}
    const char *location() {return "location";}
  };

  MyFields fields;
} // anonymous

GaugeTranslate::Info
GaugeTranslate::operator () (const std::string& str)
{
  tInfo::const_iterator it(mInfo.find(str));
  if (it == mInfo.end()) {
    MyDB::Stmt s(mDB);
    s << "SELECT "
      << fields.description()
      << "," << fields.location()
      << " FROM " << fields.table() << " WHERE " << fields.str() << "='" << str << "';";
    const MyDB::Stmt::tStrings a(s.queryStrings());

    Info info;
    if (a.size() >= 2) {
      info.description = a[0];
      info.location = a[1];
    }

    it = mInfo.insert(std::make_pair(str, info)).first;
  }

  return it->second;
}
