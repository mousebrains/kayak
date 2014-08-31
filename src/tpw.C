#include "MyDB.H"
#include <sqlite3.h>
#include <cstdlib>
#include <iostream>
#include <sstream>

int main (int, char **)
{
  typedef std::vector<std::string> tStrings;
  tStrings strings;
  const std::string chars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvxyz0123456789_.");

  for (int i(0); i < 2000; ++i) {
    std::string str;
    for (int j(0), n((random() % 20) + 5); j < n; ++j) {
      str += chars[random() % chars.size()];
    }
    strings.push_back(str);
  }

  MyDB::setFilename("b.db");
  MyDB db;
  // db.query("pragma journal_mode=WAL;");
  db.query("pragma synchronous=1;");
  db.query("DROP TABLE IF EXISTS a;");
  db.query("CREATE TABLE a (a INTEGER, b DOUBLE, PRIMARY KEY (a,b));");
  db.query("CREATE TABLE b (key INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE);");
  db.query("CREATE INDEX aa ON a(a);");
  db.query("CREATE INDEX bName ON b(name);");

  MyDB::Stmt s2(db.mkStmt("INSERT INTO b (name) VALUES(?);"));
  db.beginTransaction();
  for (int i(0), e(strings.size()); i < e; ++i) {
    s2.bindString(strings[i]);
    s2.step(); 
    s2.reset(); 
  }
  db.endTransaction();

  MyDB::Stmt s0(db.mkStmt("INSERT INTO a VALUES(?,?);"));

  db.beginTransaction();

  for (int i(0); i < 10; ++i) {
    const size_t key(random() % strings.size());
    const double val(drand48());
    // std::ostringstream oss;
    // oss << "INSERT INTO a VALUES(" << key << "," << val << ");";
    // db.query(oss.str());
    s0.bindInt(key);
    s0.bindDouble(val);
    s0.step();
    s0.reset();
  }
  
  db.endTransaction();

  return 0;
}
