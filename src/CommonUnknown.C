#include "CommonUnknown.H"
#include "Fields.H"
#include "Tokens.H"

CommonUnknown::CommonUnknown(const std::string& prefix)
  : Table(UnknownFields(prefix))
  , mFields(prefix)
{
}

CommonUnknown::~CommonUnknown()
{
}

CommonUnknown::Usage
CommonUnknown::usage(const int key)
{
  MyDB::Stmt s(mDB);
  s << "SELECT " << mFields.usage() << " FROM " << mFields.table() 
    << " WHERE " << mFields.key() << "=" << key << ";";
  MyDB::tInts a(s.queryInts());
  return a.empty() ? SEEN : (Usage) a[0];
}

std::ostream& 
operator << (std::ostream& os, 
             const CommonUnknown::Usage u)
{
  switch (u) {
    case CommonUnknown::SEEN: os << "Seen"; return os;
    case CommonUnknown::NOTIFIED: os << "Notified"; return os;
    case CommonUnknown::IGNORE: os << "Ignore"; return os;
    case CommonUnknown::LASTUSAGE: os << "GotMe"; return os;
  }
  return os;
}
