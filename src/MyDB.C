#include "MyDB.H"
#include "Paths.H"
#include "MyException.H"
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cmath>

namespace {
  std::string defaultFilename(Paths::dbname());

  typedef std::map<std::string, std::pair<int, sqlite3 *> > tDBMap;
  tDBMap dbMap;
 
  int errChk(sqlite3* db, const int rc, const std::string& msg) {
    std::string id;
 
    switch (rc) {
      case SQLITE_OK:
      case SQLITE_ROW: 
      case SQLITE_DONE: 
        return rc;
      case SQLITE_ERROR: id = sqlite3_errmsg(db); break;
      case SQLITE_INTERNAL: id = "SQLITE_INTERNAL"; break;
      case SQLITE_PERM: id = "SQLITE_PERM"; break;
      case SQLITE_ABORT: id = "SQLITE_ABORT"; break;
      case SQLITE_BUSY: id = "SQLITE_BUSY"; break;
      case SQLITE_LOCKED: id = "SQLITE_LOCKED"; break;
      case SQLITE_NOMEM: id = "SQLITE_NOMEM"; break;
      case SQLITE_READONLY: id = "SQLITE_READONLY"; break;
      case SQLITE_INTERRUPT: id = "SQLITE_INTERRUPT"; break;
      case SQLITE_IOERR: id = "SQLITE_IOERR"; break;
      case SQLITE_CORRUPT: id = "SQLITE_CORRUPT"; break;
      case SQLITE_NOTFOUND: id = "SQLITE_NOTFOUND"; break;
      case SQLITE_FULL: id = "SQLITE_FULL"; break;
      case SQLITE_CANTOPEN: id = "SQLITE_CANTOPEN"; break;
      case SQLITE_PROTOCOL: id = "SQLITE_PROTOCOL"; break;
      case SQLITE_EMPTY: id = "SQLITE_EMPTY"; break;
      case SQLITE_SCHEMA: id = "SQLITE_SCHEMA"; break;
      case SQLITE_TOOBIG: id = "SQLITE_TOOBIG"; break;
      case SQLITE_CONSTRAINT: id = "SQLITE_CONSTRAINT"; break;
      case SQLITE_MISMATCH: id = "SQLITE_MISMATCH"; break;
      case SQLITE_MISUSE: id = "SQLITE_MISUSE"; break;
      case SQLITE_NOLFS: id = "SQLITE_NOLFS"; break;
      case SQLITE_AUTH: id = "SQLITE_AUTH"; break;
      case SQLITE_FORMAT: id = "SQLITE_FORMAT"; break;
      case SQLITE_RANGE: id = "SQLITE_RANGE"; break;
      case SQLITE_NOTADB: id = "SQLITE_NOTADB"; break;
#if SQLITE_VERSION_NUMBER > 30080000
      case SQLITE_NOTICE: id = "SQLITE_NOTICE"; break;
      case SQLITE_WARNING: id = "SQLITE_WARNING"; break;
#endif // SQLITE_VERSION_NUMBER > 3008000
      default: {
          std::ostringstream oss;
          oss << "unrecognized return code, " << rc;
          id = oss.str();
        }
        break;
    }
 
    throw MyException(msg + ", " + id);
  }
}

void
MyDB::setFilename(const std::string& filename)
{
  defaultFilename = filename;
}

MyDB::MyDB()
  : mDB(0)
  , mFilename(defaultFilename)
{
}

MyDB::MyDB(const std::string filename)
  : mDB(0)
  , mFilename(filename)
{
}

MyDB::~MyDB()
{
  if (mDB) {
    tDBMap::iterator it(dbMap.find(mFilename));
    if ((it != dbMap.end()) && (--(it->second.first) <= 0)) {
      errorCheck(sqlite3_close(it->second.second), "closing " + mFilename);
      dbMap.erase(it);
    }
  }
}

void 
MyDB::open()
{
  if (!mDB) { // I don't have an open pointer yet
    tDBMap::iterator it(dbMap.find(mFilename));
    if (it == dbMap.end()) { // mFilename not opened yet
      sqlite3 *db;
      errorCheck(sqlite3_open(mFilename.c_str(), &db), "Opening " + mFilename);
      dbMap.insert(std::make_pair(mFilename, std::make_pair(1, db)));
      mDB = db;
    } else { // Already opened
      ++(it->second.first); // Increment refcount
      mDB = it->second.second;
    }
  } 
}

void
MyDB::checkPoint()
{
  errorCheck(sqlite3_wal_checkpoint_v2(mDB, 0, SQLITE_CHECKPOINT_RESTART, 0, 0), "checkPoint");
}

void 
MyDB::errorCheck(const int rc,
                 const std::string& msg)
{
  errChk(mDB, rc, msg);
}
 
size_t
MyDB::lastInsertRowid()
{
  return sqlite3_last_insert_rowid(mDB);
}

void
MyDB::query(const std::string& sql)
{
  Stmt s(*this, sql);
  int rc;

  while ((rc = s.step()) == SQLITE_ROW) {
    for (int i(0), e(s.columnCount()); i < e; ++i) {
      const int type(s.columnType(i));
      switch (type) {
        case SQLITE_INTEGER:
          std::cout << s.getInt(i) << std::endl;
          break;
        case SQLITE_FLOAT:
          std::cout << s.getDouble(i) << std::endl;
          break;
        case SQLITE_BLOB:
          std::cout << s.getBlob(i) << std::endl;
          break;
        case SQLITE_NULL:
          std::cout << "NULL" << std::endl;
          break;
        case SQLITE_TEXT:
          std::cout << s.getString(i) << std::endl;
          break;
        default:
          std::cerr << "Unknown column type " << type << std::endl;
          break;
      }
    }
  }

  if (rc != SQLITE_DONE) {
    errorCheck(rc, "query, " + s.str());
  }
}

MyDB::tStrings
MyDB::queryStrings(const std::string& sql)
{
  tStrings a;
  Stmt s(*this, sql);
  int rc;

  while ((rc = s.step()) == SQLITE_ROW) {
    for (int i(0), e(s.columnCount()); i < e; ++i) {
      a.push_back(s.getString(i));
    }
  }

  if (rc != SQLITE_DONE) {
    errorCheck(rc, "queryStrings, " + sql);
  }

  return a;
}

MyDB::tInts
MyDB::queryInts(const std::string& sql)
{
  tInts a;
  Stmt s(*this, sql);
  int rc;

  while ((rc = s.step()) == SQLITE_ROW) {
    for (int i(0), e(s.columnCount()); i < e; ++i) {
      a.push_back(s.getInt(i));
    }
  }

  if (rc != SQLITE_DONE) {
    errorCheck(rc, "queryInts, " + sql);
  }

  return a;
}

MyDB::tDoubles
MyDB::queryDoubles(const std::string& sql)
{
  tDoubles a;
  Stmt s(*this, sql);
  int rc;

  while ((rc = s.step()) == SQLITE_ROW) {
    for (int i(0), e(s.columnCount()); i < e; ++i) {
      a.push_back(s.getDouble(i));
    }
  }

  if (rc != SQLITE_DONE) {
    errorCheck(rc, "queryDoubles, " + sql);
  }
 
  return a;
}


MyDB::tStringInt
MyDB::queryStringInt(const std::string& sql)
{
  tStringInt a;
  Stmt s(*this, sql);
  int rc;

  while ((rc = s.step()) == SQLITE_ROW) {
    if (s.columnCount() != 2) {
      std::ostringstream oss;
      oss << "Error, '" << sql << "' produced " 
          << s.columnCount() << " columns, but should have produced 2.";
      MyException e(oss.str());
      throw e;
    }
    if (s.columnType(0) != SQLITE_TEXT) {
      std::ostringstream oss;
      oss << "Error, '" << sql << "' first column is not text, " << s.columnType(0);
      MyException e(oss.str());
      throw e;
    }
    if (s.columnType(1) != SQLITE_INTEGER) {
      std::ostringstream oss;
      oss << "Error, '" << sql << "' second column is not integer, " << s.columnType(1);
      MyException e(oss.str());
      throw e;
    }
    a.push_back(std::make_pair(s.getString(0), s.getInt(1)));
  }

  return a;
}

MyDB::Stmt::Stmt(const Stmt& a)
  : std::ostringstream(a.str())
  , mDB(a.mDB)
  , mStmt(a.mStmt)
  , mIndex(a.mIndex)
{
}

MyDB::Stmt::Stmt(MyDB& db)
  : mDB(db)
  , mStmt(0)
  , mIndex(0)
{
}
 
MyDB::Stmt::Stmt(MyDB& db, const std::string& sql)
  : std::ostringstream(sql)
  , mDB(db)
  , mStmt(0)
  , mIndex(0)
{
  prepare();
}

MyDB::Stmt::~Stmt()
{
  if (mStmt)
    errorCheck(sqlite3_finalize(mStmt), "in destructor");
}

void
MyDB::Stmt::prepare()
{
  const std::string sql(str());

  if (mStmt) { // already called
    throw MyException("Error, end() already called, " + sql);
  }

  const char *tail;

  mDB.open(); // Make sure our database is opened

  errorCheck(sqlite3_prepare_v2(mDB.mDB, sql.c_str(), -1, &mStmt, &tail), "preparing");

  if (mStmt == 0) {
    throw MyException("Error preparing '" + sql + "', null statement returned");
  }

  if (*tail != '\0') {
    throw MyException("Error preparing '" + sql + "', something was leftover, '" + tail + "'");
  }
}
  
int
MyDB::Stmt::columnCount()
{
  if (!mStmt) prepare();
  return sqlite3_column_count(mStmt);
}

int
MyDB::Stmt::columnType(const int col)
{
  if (!mStmt) prepare();
  return sqlite3_column_type(mStmt, col);
}

int
MyDB::Stmt::errorCheck(const int rc,
                       const std::string& s)
{
  if ((rc == SQLITE_OK) || (rc == SQLITE_DONE) || (rc == SQLITE_BUSY)) {
    return rc;
  }
  return errChk(mDB.mDB, rc, s + ", " + str());
}

void
MyDB::Stmt::bind(const int val)
{
  if (!mStmt) prepare();
  const int rc(sqlite3_bind_int(mStmt, ++mIndex, val));
  if (rc != SQLITE_OK) {
    std::ostringstream oss;
    oss << "bind<int> for column " << mIndex << " and value " << val;
    errorCheck(rc, oss.str());
  }
}

void
MyDB::Stmt::bind(const double val)
{
  if (!mStmt) prepare();
  const int rc(sqlite3_bind_double(mStmt, ++mIndex, val));
  if (rc != SQLITE_OK) {
    std::ostringstream oss;
    oss << "bind<double> for column " << mIndex << " and value " << val;
    errorCheck(rc, oss.str());
  }
}

void
MyDB::Stmt::bind(const std::string& val)
{
  if (!mStmt) prepare();
  const int rc(sqlite3_bind_text(mStmt, ++mIndex, val.c_str(), val.size(), 0));
  if (rc != SQLITE_OK) {
    std::ostringstream oss;
    oss << "bind<std::string> for column " << mIndex << " and value '" << val;
    errorCheck(rc, oss.str());
  }
}

int
MyDB::Stmt::step()
{
  mIndex = 0;
  if (!mStmt) prepare();
  return errorCheck(sqlite3_step(mStmt), "step");
}

void
MyDB::Stmt::reset()
{
  mIndex = 0;
  if (!mStmt) prepare();
  errorCheck(sqlite3_reset(mStmt), "reset");
}

int 
MyDB::Stmt::getInt(const int col)
{
  if (!mStmt) prepare();
  return sqlite3_column_int(mStmt, col);
}

double
MyDB::Stmt::getDouble(const int col)
{
  if (!mStmt) prepare();
  return columnType(col) == SQLITE_NULL ? NAN : sqlite3_column_double(mStmt, col);
}

std::string
MyDB::Stmt::getString(const int col)
{
  if (!mStmt) prepare();
  const char *ptr((const char *) sqlite3_column_text(mStmt, col));
  return ptr ? std::string(ptr) : std::string();
}

std::string
MyDB::Stmt::getBlob(const int col)
{
  if (!mStmt) prepare();
  const void *ptr(sqlite3_column_blob(mStmt, col));
  return ptr ? std::string((const char *) ptr, sqlite3_column_bytes(mStmt, col)) : std::string();
}

void
MyDB::dropRows(const std::string& table,
               const tRows& rowid)
{
  std::ostringstream oss;
  oss << "DELETE FROM " << table << " WHERE rowid == ?;";
  Stmt s(*this, oss.str());

  beginTransaction();

  for (tRows::const_reverse_iterator it(rowid.rbegin()), et(rowid.rend()); it != et; ++it) {
    s.bind(*it);
    s.step();
    s.reset();
  }
  endTransaction();
}

std::ostream&
operator << (std::ostream& os,
             const MyDB::tStrings& a)
{
  std::string delim;
  for (MyDB::tStrings::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    os << delim << *it;
    delim = ",";
  }
  return os;
}

std::ostream&
operator << (std::ostream& os,
             const MyDB::tInts& a)
{
  std::string delim;
  for (MyDB::tInts::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    os << delim << *it;
    delim = ",";
  }
  return os;
}

std::ostream&
operator << (std::ostream& os,
             const MyDB::tDoubles& a)
{
  std::string delim;
  for (MyDB::tDoubles::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    os << delim << *it;
    delim = ",";
  }
  return os;
}

std::ostream&
operator << (std::ostream& os,
             const MyDB::tRows& a)
{
  std::string delim;
  for (MyDB::tRows::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    os << delim << *it;
    delim = ",";
  }
  return os;
}
