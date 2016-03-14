// Get all data from MySQL with a corresponding entry in SQLite database
// 
#include "MySQL.H"
#include "Data.H"
#include "Convert.H"
#include <iostream>

namespace {
  typedef std::set<std::string> tSet;

  tSet getIds(MyDB& db) {
    MyDB::Stmt s(db, "SELECT DISTINCT name FROM dataSource;");
    MyDB::tStrings a(s.queryStrings());
    tSet b(a.begin(), a.end());
    return b;
  }
} // anonymous

int
main (int argc,
    char **argv)
{
  int maxCount(argc > 1 ? atoi(argv[1]) : 1000000000);
  MySQL sql("levels_data");
  const MySQL::tTables& dTables(sql.tables());
  MyDB db;
  Data data;
  const tSet ids(getIds(db));
  int cnt(0);

  std::cout << "nTables " << dTables.size() 
            << " ids " << ids.size()
            << std::endl;

  for (MySQL::tTables::const_iterator it(dTables.begin()), et(dTables.end()); 
       (it != et) && cnt < maxCount; ++it) {
    const std::string::size_type offset(it->find('_'));
    if (offset == it->npos) continue;
    const Data::Type type(Data::decodeType(it->substr(0,offset)));
    const std::string name(MySQL::fixId(it->substr(offset + 1)));
    tSet::const_iterator kt(ids.find(name));
    if (kt == ids.end()) continue;
    ++cnt;
    MySQL::tData tbl(sql.data(*it));
    std::cout << "Data for " << type << " " << name << " n " << tbl.size() << std::endl;
    for (MySQL::tData::const_iterator jt(tbl.begin()), jet(tbl.end()); jt != jet; ++jt) {
      data.add(*kt, jt->first, Convert::strTo<double>(jt->second), type, std::string());
    } 
  }

  return 0;
}
