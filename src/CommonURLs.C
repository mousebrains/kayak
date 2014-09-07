#include "CommonURLs.H"
#include "Fields.H"
#include "Tokens.H"

namespace {
  bool chkOkay(const int a, const std::string& b) {
    if (b.empty() || (b == "*")) {
      return true;
    }
    const Tokens toks(b, ", \t\n"); 
    for (Tokens::const_iterator it(toks.begin()), et(toks.begin()); it != et; ++it) {
      std::istringstream iss(*it);
      int c;
      if ((iss >> c) && (a == c)) {
        return true;
      }
    }
    return false;
  }
}

CommonURLs::CommonURLs(const std::string& prefix)
  : Table(URLsFields(prefix))
  , mFields(prefix)
{
}

CommonURLs::~CommonURLs()
{
}

std::string 
CommonURLs::parser(const int key)
{
  MyDB::Stmt s(mDB);
  s << "SELECT " << mFields.parser() << " FROM " << mFields.table() 
    << " WHERE " << mFields.key() << "=" << key << ";";
  MyDB::Stmt::tStrings a(s.queryStrings());
  return a.empty() ? std::string() : a[0];
}

bool 
CommonURLs::qOkayToRun(const int key, 
                       const time_t t)
{
  MyDB::Stmt s(mDB);

  s << "SELECT " 
    << mFields.qRun() 
    << "," << mFields.monthOfYear() 
    << "," << mFields.dayOfMonth() 
    << "," << mFields.hourOfDay() 
    << " FROM " << mFields.table()
    << " WHERE " << mFields.key() << "=" << key 
    << ";";

  int rc(s.step());

  if (rc != SQLITE_ROW) { // no data
    s.errorCheck(rc, "qOkayToRun");
    return false;
  }

  if (s.getInt(0) == 0) { // Not okay to run
    return false;
  }

  const struct tm *ptr(localtime(&t));

  return chkOkay(ptr->tm_mon + 1, s.getString(1)) &&
         chkOkay(ptr->tm_mday, s.getString(2)) &&
         chkOkay(ptr->tm_hour, s.getString(3));
}
