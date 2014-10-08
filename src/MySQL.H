#ifndef INC_MySQL_H_
#define INC_MySQL_H_

#include <mysql++/mysql++.h>
#include <iosfwd>
#include <string>
#include <vector>
#include <set>
#include <map>

class MySQL {
public:
  typedef std::set<std::string> tTables;
  typedef std::multimap<time_t, std::string> tData;

  typedef std::map<std::string, std::string> tMaster;
  typedef std::vector<tMaster> tRows;

private:
  const std::string mDBName;
  mysqlpp::Connection mConnection;
  mysqlpp::Query mQuery;

  tTables mTables;
public:
  MySQL(const std::string& dbname);
  ~MySQL();

  const tTables& tables();
  tData data(const std::string& name);
  size_t nData(const std::string& table);
  time_t maxDataTime(const std::string& table);

  tRows master(const std::string& table);
};

std::ostream& operator << (std::ostream& os, const MySQL::tMaster& m);

#endif // INC_MySQL_H_
