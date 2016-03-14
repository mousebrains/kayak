#include "MySQL.H"
#include <errmsg.h>
#include <iostream>
#include <sstream>

MySQL::MySQL(const std::string& dbname)
  : mDBName(dbname)
  , mConnection(0)
{
  initialize();
}

void
MySQL::initialize()
{
  if (mConnection) {
    mysql_close(mConnection);
  }

  mConnection = mysql_init(NULL);

  if (!mConnection) {
    std::cerr << "Error initializing connection, " << mysql_error(mConnection) << std::endl;
    exit(1);
  }

  // const char *host("mysql.wkcc.org");
  const char *host("208.97.160.148");
  const char *user("levels");
  const char *passwd("Deschutes");

  if (!mysql_real_connect(mConnection,  host, user, passwd, mDBName.c_str(), 0, 0, 0)) {
    std::cerr << "Error connecting to database server, " << mysql_error(mConnection) << std::endl;
    exit(1);
  }

  std::cout << "Connected to " << user << "@" << host << " db " << mDBName << std::endl;
}

MySQL::~MySQL()
{
  if (mConnection) {
    mysql_close(mConnection);
  }
}

bool
MySQL::qRetry(const std::string& str)
{
  if (mysql_errno(mConnection) != CR_SERVER_GONE_ERROR) 
    return false;
  std::cerr << "Reinitializing for '" << str << "'" << std::endl;
  initialize();
  return true;
}

void
MySQL::queryNoReturn(const std::string& str)
{
  if (mysql_query(mConnection, str.c_str()) && 
      (!qRetry(str) || mysql_query(mConnection, str.c_str()))) {
    std::cerr << "Error executing query\n" << str << "\n" << mysql_error(mConnection) << std::endl;
    exit(1);
  }
}

MYSQL_RES *
MySQL::queryReturn(const std::string& str)
{
  if (mysql_query(mConnection, str.c_str()) && 
      (!qRetry(str) || mysql_query(mConnection, str.c_str()))) {
    return 0;
  }
  return mysql_store_result(mConnection);
}
const MySQL::tTables&
MySQL::tables()
{
  if (mTables.empty()) {
    MYSQL_RES *result(queryReturn("show tables"));
    if (!result) return mTables;
    for (MYSQL_ROW row; (row = mysql_fetch_row(result));) {
      mTables.insert(row[0]);
    }
  }
  return mTables;
}

size_t
MySQL::nData(const std::string& table)
{
  std::ostringstream oss;
  oss << "select count(value) from " << table << ";";
  MYSQL_RES *result(queryReturn(oss.str())); 
  if (!result) return 0;
  MYSQL_ROW row(mysql_fetch_row(result));
  return row ? (size_t) strtoul(row[0], 0, 10) : 0;
}

time_t
MySQL::maxDataTime(const std::string& table)
{
  std::ostringstream oss;
  oss << "select unix_timestamp(max(time)) from " << table << ";";
  MYSQL_RES *result(queryReturn(oss.str()));
  if (!result) return 0;
  MYSQL_ROW row(mysql_fetch_row(result));
  return row ? (time_t) strtoul(row[0], 0, 10) : 0;
}

MySQL::tData
MySQL::data(const std::string& name)
{
  tData data;
  std::ostringstream oss;
  oss << "select unix_timestamp(time),value from " << name << " order by time desc;";

  MYSQL_RES *result(queryReturn(oss.str()));

  if (!result) {
    std::cerr << "Error executing '" << oss.str() << "', " << mysql_error(mConnection) << std::endl;
    return data;
  }

  for (MYSQL_ROW row; (row = mysql_fetch_row(result));) {
    const time_t t(strtoul(row[0], 0, 10));
    data.insert(std::make_pair(t, row[1]));
  }
  return data;
}

MySQL::tRows
MySQL::master(const std::string& table)
{
  tRows a;
  typedef std::vector<std::string> tColumns;
  tColumns cols;
  {
    std::ostringstream oss;
    oss << "SHOW COLUMNS FROM " << table << ";";
    MYSQL_RES *result(queryReturn(oss.str()));
    if (!result) return a;
    for (MYSQL_ROW row; (row = mysql_fetch_row(result));) {
      cols.push_back(row[0]);
    }
  }

  std::ostringstream oss;
  oss << "select * from " << table;

  if (table == "Master") {
    oss << " WHERE" 
           << " state IS NULL OR"
           << " state LIKE '%Washington%' OR" 
           << " state LIKE '%Idaho%' OR" 
           << " state LIKE '%Oregon%'" 
           ;
  }

  oss << ";";

  MYSQL_RES *result(queryReturn(oss.str()));
  if (!result) return a;

  const size_t n(mysql_num_fields(result));

  for (MYSQL_ROW row; (row = mysql_fetch_row(result));) {
    tMaster b;
    for (size_t j(0); j < n; ++j) {
      if (row[j]) {
        const std::string val(row[j]);
        if (val != "NULL") {
          b.insert(std::make_pair(cols[j], val));
        }
      }
    }
    if (!b.empty())
      a.push_back(b);
  }
  return a;
}

std::string
MySQL::fixId(const std::string& str)
{
 typedef std::map<std::string, std::string> tMap;
  static const tMap cbttMap = {
    {"NSJO", "NSGO3"},
    };

  tMap::const_iterator it(cbttMap.find(str));
  if (it != cbttMap.end()) return it->second;

  if (str.size() != 4) return str;

  switch (str[str.size() - 1]) {
    case 'I': return str + "1"; // Idaho
    case 'O': return str + "3"; // Oregon
    case 'W': return str + "1"; // Washington
  }

  return str;
} // fixId

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
