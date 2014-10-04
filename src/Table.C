#include "Table.H"
#include "Fields.H"
#include <memory>

Table::Table(const Fields& fields)
  : mqModified(false)
  , mFields(fields)
{
}

Table::~Table()
{
  if (!mName2Info.empty()) {
    updateTimes();
  }
}
 
int
Table::name2key(const std::string& name,
                const time_t t,
                const bool qInsert)
{
  if (name.empty()) {
    return 0;
  }

  tNameInfo::iterator it(mName2Info.find(name));

  if (it != mName2Info.end()) {
    if (it->second.time < t) {
      it->second.time = t;
      it->second.qModified = true;
      mqModified = true;
    }
    return it->second.key;
  }

  MyDB::Stmt s(mDB);
  s << "SELECT " << mFields.key() << "," << mFields.time() 
    << " FROM " << mFields.table() 
    << " WHERE " << mFields.name() << "='" << name 
    << "' COLLATE NOCASE;";
  MyDB::tInts info(s.queryInts());

  if (info.empty()) { // Not set, so insert
    if (!qInsert) {
      return 0;
    }

    MyDB::Stmt s1(mDB);
    s1 << "INSERT " 
       << " INTO " << mFields.table() << " ("
       << mFields.name() << "," << mFields.time() 
       << ") VALUES('" << name << "'," << t << ");";
    s1.query();

    info = MyDB::Stmt(mDB, s.str()).queryInts(); // Get key again
  }

  struct Info item;
  item.key = info[0];
  item.qModified = info[1] < t;
  item.time = item.qModified ? t : info[1];
  mqModified |= item.qModified;

  mName2Info.insert(std::make_pair(name, item));
  mKey2Name.insert(std::make_pair(item.key, name));

  return item.key;
}

std::string
Table::key2name(const int key)
{
  if (key <= 0) {
    return std::string();
  }

  tKeyName::const_iterator it(mKey2Name.find(key));

  if (it != mKey2Name.end()) { // Not in lookup table, so load it
    return it->second;
  }

  MyDB::Stmt s(mDB);
  s << "SELECT " << mFields.name() << "," << mFields.time()
    << " FROM " << mFields.table()
    << " WHERE " << mFields.key() << "=" << key << ";";
  MyDB::Stmt::tStringInt info(s.queryStringInt());
     
  if (info.empty()) { // Does not exist
    return std::string();
  }

  const std::string& name(info[0].first);
  struct Info item;
  item.key = key;
  item.qModified = false;
  item.time = info[0].second;

  mName2Info.insert(std::make_pair(name, item));
  mKey2Name.insert(std::make_pair(key, name));
  return name;
}

Table::tNameKey
Table::allNameKeys()
{
  MyDB::Stmt s(mDB);
  s << "SELECT " << mFields.name() << "," << mFields.key()
    << " FROM " << mFields.table() << ";";
  MyDB::Stmt::tStringInt info(s.queryStringInt());
  tNameKey name2key;

  for (MyDB::Stmt::tStringInt::const_iterator it(info.begin()), et(info.end()); it != et; ++it) {
    name2key.insert(std::make_pair(it->first, it->second));
  }
  return name2key;
}

void
Table::updateTimes()
{
  if (!mqModified) { // Nothing to do
    return;
  }

  MyDB::Stmt s(mDB);

  s << "UPDATE " << mFields.table() << " SET " 
    << mFields.time() << "=MAX(" << mFields.time() 
    << ",?) WHERE " << mFields.key() << "=?;";

  mDB.beginTransaction();
  for (tNameInfo::const_iterator it(mName2Info.begin()), et(mName2Info.end()); it != et; ++it) {
    const struct Info& info(it->second);
    s.bind(info.time);
    s.bind(info.key);
    s.step();
    s.reset();
  }
  mDB.endTransaction();
}
