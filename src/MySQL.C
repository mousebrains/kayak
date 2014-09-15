#include "MySQL.H"
#include <iostream>

namespace {
  std::string loadString(const std::string str) {
    return str == "NULL" ? std::string() : str;
  }
}

typedef mysqlpp::StoreQueryResult Result;
typedef mysqlpp::Row Row;

MySQL::MySQL(const std::string& dbname)
  : mDBName(dbname)
  , mConnection(dbname.c_str(), "mysql.wkcc.org", "levels", "Deschutes")
  , mQuery(mConnection.query())
{
}

MySQL::~MySQL()
{
}

const MySQL::tTables&
MySQL::tables()
{
  if (mTables.empty()) {
    mQuery << "show tables;";
    const Result result(mQuery.store());
    for (Result::size_type i = 0, e = result.size(); i < e; ++i) {
      mTables.insert(result.at(i).at(0).c_str());
    }
  }
  return mTables;
}

size_t
MySQL::nData(const std::string& table)
{
  mQuery << "select count(value) from " << table << ";";
  const Result result(mQuery.store());
  return result.size() ? (size_t) result.at(0).at(0) : 0;
}

time_t
MySQL::maxDataTime(const std::string& table)
{
  mQuery << "select unix_timestamp(max(time)) from " << table << ";";
  const Result result(mQuery.store());
  return result.size() ? (time_t) result.at(0).at(0) : 0;
}

MySQL::tData
MySQL::data(const std::string& name)
{
  tData data;

  mQuery << "select unix_timestamp(time),value from "
         << name << " order by time desc;";
  const Result result(mQuery.store());
  for (Result::size_type i = 0, e = result.size(); i < e; ++i) {
    const Row& row(result.at(i));
    data.insert(std::make_pair((time_t) row.at(0), row.at(1)));
  }

  return data;
}

MySQL::tRows
MySQL::master(const std::string& table)
{
  typedef std::vector<std::string> tColumns;
  tColumns cols;
  {
    mQuery << "SHOW COLUMNS FROM " << table << ";";
    const Result result(mQuery.store());
    for (Result::size_type i = 0, e = result.size(); i < e; ++i) {
      const Row& row(result.at(i));
      cols.push_back(row.at(0).c_str());
    }
  }

  tRows a;

  mQuery << "select * from " << table;

  if (table == "Master") {
    mQuery << " WHERE" 
           << " state IS NULL OR"
           << " state LIKE '%Washington%' OR" 
           << " state LIKE '%Idaho%' OR" 
           << " state LIKE '%California%' OR" 
           << " state LIKE '%Oregon%'" 
           ;
  }

  mQuery << ";";

  const Result result(mQuery.store());
  for (Result::size_type i = 0, e = result.size(); i < e; ++i) {
    const Row& row(result.at(i));
    tMaster b;
    for (Row::size_type j = 0, je = row.size(); j < je; ++j) {
      const std::string val(row.at(j).c_str());
      if (val != "NULL") {
        b.insert(std::make_pair(cols[j], val));
      }
    }
    if (!b.empty())
      a.push_back(b);
  }
  return a;
}

std::ostream&
operator << (std::ostream& os,
             const MySQL::tMaster& a)
{
  os << '{';
  for (MySQL::tMaster::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    os << it->first << "='" << it->second << "'" << std::endl;
  }
  os  << '}';
  return os;
}
