#ifndef INC_MyDB_H_
#define INC_MyDB_H_

#include <sqlite3.h>
#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

class MyDB {
public:
  class Stmt : public std::ostringstream {
  private:
    MyDB& mDB;
    sqlite3_stmt *mStmt;
    size_t mIndex;

    void prepare();
  public:
    explicit Stmt(MyDB& db);
    Stmt(MyDB& db, const std::string& sql);
    Stmt(const Stmt& a); // Copy constructor
    ~Stmt();

    int errorCheck(const int rc, const std::string& str);

    void bind(); // Bind a null
    void bind(const int val); //bind an integer
    void bind(const size_t val) {bind((int) val);} 
    void bind(const time_t val) {bind((int) val);}
    void bind(const double val); // bind a double
    void bind(const std::string& val); // bind a string

    int step();
    void reset();

    int columnCount();
    int columnType(const int col);

    int getInt(const int col);
    double getDouble(const int col);
    std::string getString(const int col);
    std::string getBlob(const int col);

    int getInt() {return getInt(mIndex++);}
    double getDouble() {return getDouble(mIndex++);}
    std::string getString() {return getString(mIndex++);}
    std::string getBlob() {return getBlob(mIndex++);}

    typedef std::vector<std::string> tStrings;
    typedef std::vector<int> tInts;
    typedef std::vector<double> tDoubles;
    typedef std::vector<std::pair<std::string, int> > tStringInt;

    void query();
    tStrings queryStrings();
    tInts queryInts();
    tDoubles queryDoubles();
    tStringInt queryStringInt();

    void quoteString(const std::string& str);
    static std::string quotedString(const std::string& str);
  }; // Stmt

  typedef std::set<size_t> tRows;
  typedef std::multimap<time_t, size_t> tTimeKeys;

  typedef std::pair<int, time_t> tKeyTime;
  typedef std::map<std::string, tKeyTime> tNameKeyTime;
  typedef std::map<int, std::string> tKeyName;
private:
  friend class Stmt;

  sqlite3 *mDB;
  const std::string mFilename;

  void open();

  typedef std::set<std::string> tSet;
public:
  static void setFilename(const std::string& filename);

  MyDB();
  explicit MyDB(const std::string filename);
  ~MyDB();

  void errorCheck(const int rc, const std::string& msg);

  void beginTransaction() {Stmt(*this, "BEGIN;").query();}
  void endTransaction() {Stmt(*this, "COMMIT;").query();}

  void query(const std::string& sql) {Stmt(*this, sql).query();}

  int lastInsertRowid();

  void dropRows(const std::string& table, const tRows& rowids);

  void checkPoint();
};

std::ostream& operator << (std::ostream& os, const MyDB::Stmt::tStrings& a);
std::ostream& operator << (std::ostream& os, const MyDB::Stmt::tInts& a);
std::ostream& operator << (std::ostream& os, const MyDB::Stmt::tDoubles& a);
std::ostream& operator << (std::ostream& os, const MyDB::tRows& a);

#endif // INC_MyDB_H_
