// From the existing mySQL Master table and get all the gauges in db_name and merged_dbs
// From the existing mySQL data tables get all the gauges stored there
// Take the interesection of the two sets
// Ignore any data tables without any data
// Ignore any data tables without recent data
// Save only what remains into SQLite's data and gauges tables
// 
#include "MySQL.H"
#include "TimeIt.H"
#include "Tokens.H"
#include "Calc.H"
#include "Data.H"
#include "Gauges.H"
#include "String.H"
#include "Convert.H"
#include <iostream>
#include <set>
#include <cmath>

typedef std::set<std::string> tSet;

std::ostream& operator << (std::ostream& os, const tSet& a) {
  std::string delim;
  for (tSet::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    os << delim << *it;
    delim = std::string(",");
  }
  return os;
}

class DataTables {
public:
  struct DataTable {
    mutable bool qUsed;
    std::string table;
    std::string type;
    std::string gauge;
    DataTable(const std::string& tbl, const std::string& ty, const std::string& g)
      : qUsed(false), table(tbl), type(ty), gauge(g) {}
    explicit DataTable(const std::string& g) : qUsed(false), gauge(g) {}
    bool operator < (const DataTable& rhs) const {return gauge < rhs.gauge;}
  };
private:
  typedef std::multiset<DataTable> tDataTables;
  tDataTables mDataTables;
public:
  DataTables(MySQL& sql, int maxCount) {
    const time_t threshold(time(0) - 120 * 86400); // Data newer than this to be "current"
    const TimeIt stime;
    const MySQL::tTables& tables(sql.tables());
    std::cout << "Starting to get current data tables for " << tables.size() 
              << " tables" << std::endl;
    for (MySQL::tTables::const_iterator it(tables.begin()), et(tables.end()); 
         (it != et) && (--maxCount > 0); ++it) {
      const std::string::size_type i(it->find('_'));
      if (i == it->npos) continue;
      if (sql.maxDataTime(*it) < threshold) continue;
      mDataTables.insert(DataTable(*it, it->substr(0,i), it->substr(i+1)));
    }
    std::cout << mDataTables.size() << " current data tables out of " << tables.size() 
              << " in " << stime << " seconds" << std::endl;
  } // DataTables

  bool qKnown(const std::string& gauge) const {
    return mDataTables.find(DataTable(gauge)) != mDataTables.end();
  }

  typedef tDataTables::const_iterator const_iterator;
  const_iterator begin() const {return mDataTables.begin();}
  const_iterator end() const {return mDataTables.end();}

  typedef std::pair<const_iterator, const_iterator> iterators;
  iterators equal_range(const std::string& gauge) const {
    return mDataTables.equal_range(DataTable(gauge));
  }
}; // DataTables

class MasterRow {
private:
  typedef MySQL::tMaster tRow;
  tRow mRow;
  Calc mCalc;
  tSet mParsers;
  size_t mGaugeKey;

  std::string getString(const std::string& key, const std::string defVal=std::string()) const {
    tRow::const_iterator it(mRow.find(key));
    return it == mRow.end() ? defVal : it->second;
  }

  double getDouble(const std::string& key) const {
    tRow::const_iterator it(mRow.find(key));
    return it == mRow.end() ? NAN : Convert::strTo<double>(it->second);
  }

  void set(const std::string& key, const std::string& value) {
    mRow.insert(std::make_pair(key, value));
  }

  void addParsers(); 
  void fixIds(); 
  void fixCalc();
  void fixClass();
public:
  MasterRow(const MySQL::tMaster& row)
    : mRow(row)
    , mCalc(0, getString("calc_expr"), Data::decodeType(getString("calc_type")))
    , mGaugeKey(0)
  {
    addParsers();
    fixIds();
    fixCalc();
    fixClass();
  }

  void pruneParsers(const DataTables& tbls);

  const Calc& calc() const {return mCalc;}
  Calc& calc() {return mCalc;}
  const tSet& parsers() const {return mParsers;}

  std::string hash() const {return getString("HashValue");}
  std::string nature() const {return getString("Nature");}
  std::string pageNumber() const {return getString("PageNumber");}
  std::string runNumber() const {return getString("RunNumber");}
  std::string stationNumber() const {return getString("StationNumber");}
  std::string calcExpr() const {return getString("calc_expr");}
  std::string calcTime() const {return getString("calc_time");}
  std::string calcType() const {return getString("calc_type");}
  std::string cbtt_id() const {return getString("cbtt_id");}
  std::string grade() const {return getString("class");}
  std::string gradeFlow() const {return getString("class_flow");}
  time_t date() const {return Convert::toTime(getString("date"));}
  std::string dbName() const {return getString("db_name");}
  std::string dbRating() const {return getString("db_rating");}
  std::string dbSource() const {return getString("db_source");}
  std::string difficulties() const {return getString("difficulties");}
  std::string displayName() const {return getString("display_name");}
  std::string drainage() const {return getString("drainage");}
  double drainageArea() const {return getDouble("drainage_area");}
  double elevation() const {return getDouble("elevation");}
  double elevationLost() const {return getDouble("elevation_lost");}
  std::string features() const {return getString("features");}
  std::string location() const {return getString("gauge_location");}
  double gradient() const {return getDouble("gradient");}
  std::string guideBook() const {return getString("guide_book");}
  double highFlow() const {return getDouble("high_flow");}
  std::string idCBTT() const {return getString("idCBTT");}
  std::string idUSGS() const {return getString("idUSGS");}
  double latitude() const {return getDouble("latitude");}
  double length() const {return getDouble("length");}
  double longitude() const {return getDouble("longitude");}
  double lowFlow() const {return getDouble("low_flow");}
  std::string mergedDBs() const {return getString("merged_dbs");}
  bool qNoShow() const {return !getString("no_show").empty();}
  std::string notes() const {return getString("notes");}
  std::string nwsID() const {return getString("nws_id");}
  std::string nwsliID() const {return getString("nwsli_id");}
  double optimalFlow() const {return getDouble("optimal_flow");}
  std::string region() const {return getString("region");}
  std::string remoteness() const {return getString("remoteness");}
  std::string riverName() const {return getString("river_name");}
  std::string scenery() const {return getString("scenery");}
  std::string season() const {return getString("season");}
  std::string section() const {return getString("section");}
  std::string sortKey() const {return getString("sortKey");}
  std::string state() const {return getString("state");}
  std::string watershedType() const {return getString("watershed_type");}
  std::string usgs_id() const {return getString("usgs_id");}
  std::string gaugeName() const {return getString("gaugeName");}
  void gaugeName(const std::string& name) {return set("gaugeName", name);}
  size_t gaugeKey() const {return mGaugeKey;}
  void gaugeKey(const size_t key) {mGaugeKey = key;}

  typedef tRow::const_iterator const_iterator;
  const_iterator begin() const {return mRow.begin();}
  const_iterator end() const {return mRow.end();}
}; // MasterRow

std::ostream&
operator << (std::ostream& os,
             const MasterRow& row)
{
  for (MasterRow::const_iterator it(row.begin()), et(row.end()); it != et; ++it) {
    os << it->first << "='" << it->second << "'" << std::endl;
  }
  if (!row.calc().empty()) {
    os << "CALC: " << row.calc() << std::endl;
  }
  if (!row.parsers().empty()) {
    os << "PARSERS: " << row.parsers() << std::endl;
  }
  return os;
}

void
MasterRow::pruneParsers(const DataTables& tbls)
{
  const size_t n(mParsers.size());
  if (n == 0) return;

  for (tSet::iterator it(mParsers.begin()), et(mParsers.end()); it != et;) {
    if (!tbls.qKnown(*it)) {
      if (it == mParsers.begin()) {
        mParsers.erase(it);
        it = mParsers.begin();
      } else {
        tSet::iterator jt(it);
        ++it;
        mParsers.erase(jt);
      }
    } else {
      ++it;
    }
  }
}

void
MasterRow::addParsers()
{
  const Tokens names(getString("merged_dbs", getString("db_name")), " ,\t\n");

  for (Tokens::const_iterator it(names.begin()), et(names.end()); it != et; ++it) {
    // if (it->find("_calc") == (it->size() - 5)) continue; // Skip _cal parsers
    mParsers.insert(*it);
  }
}

void
MasterRow::fixIds()
{
    std::string usgs(usgs_id());
    std::string cbtt(cbtt_id());
    std::string sNumber(stationNumber());
    std::string nws(nwsID());
    std::string nwsli(nwsliID());

    if (!sNumber.empty() && (sNumber != usgs)) {
      cbtt = sNumber;
      sNumber.clear();
    }

    if (usgs.find(nws) == 0) { // nws is usgs
      nws.clear();
    }

    if (usgs.empty() && (nws.size() == 8) && (nws.find_first_not_of("0123456789") == nws.npos)) {
      usgs = nws;
      nws.clear();
    }

    if (!nwsli.empty()) {
      if (cbtt.empty() ||
          (nwsli.find(cbtt) == 0) ||
          ((cbtt[cbtt.size()-1] == 'X') && (nwsli.find(cbtt.substr(0,cbtt.size()-1)) == 0)) ||
          ((cbtt[0] == 'U') && (nwsli.find(cbtt.substr(1)) == 0)) ||
          ((nwsli == "FRNO3") && (cbtt == "NOTO")) ||
          ((nwsli == "MKLO3") && (cbtt == "MZLO")) ||
          ((nwsli == "OCUO3") && (cbtt == "ORC")) ||
          ((nwsli == "VENO3") && (cbtt == "NRVO")) ||
          ((nwsli == "BTYO3") && (cbtt == "SPBO")) ||
          ((nwsli == "CWMO3") && (cbtt == "GLNO"))) {
        cbtt = nwsli;
        nwsli.clear();
      }
    }

    if (usgs.empty() && cbtt.empty()) { // Look at the parsers
      for (tSet::const_iterator it(mParsers.begin()), et(mParsers.end()); it != et; ++it) {
         const std::string& name(*it);
         if ((name.size() >= 8) && (name.find_first_not_of("0123456789") == name.npos)) {
           usgs = name;
         } else if ((name.size() == 5) && (String::tolower(name.substr(3)) == "o3")) {
           cbtt = name;
         } else if ((cbtt.size() < 4) && (name.size() == 4) && 
                    ((name[3] == 'O') || (name[3] == 'o'))) {
           cbtt = name;
         } else if (cbtt.empty() && (name.size() == 3) && 
                    (name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVXYZabcdefgnamefghijklmnopqrstuvwxyz") == name.npos)) {
           cbtt = name;
         }
      }
    }

    if (!usgs.empty()) {
      set("idUSGS", usgs);
    }
    if (!cbtt.empty()) {
      set("idCBTT", cbtt);
    }
} // MasterRow::fixIds

void
MasterRow::fixCalc()
{
  typedef std::map<std::string, std::string> tf2f;
  static const tf2f f2f = { {"greatest", "max"}, {"least", "min"} };
  for (Calc::size_type i(0), e(mCalc.size()); i < e; ++i) {
    const std::string field(String::tolower(mCalc[i].str()));
    tf2f::const_iterator it(f2f.find(field));
    if (it != f2f.end()) mCalc[i] = Calc::Field(it->second);
  }
}

void
MasterRow::fixClass()
{
  const std::string b(gradeFlow());
  if (!b.empty()) set("class", b);
}

class Master {
private:
  typedef std::map<std::string, MasterRow> tRows;
  tRows mRows;

  typedef std::map<std::string, Gauges::Info> tGauges;
  tGauges mGauges;

  std::string addGauge(const MasterRow& row);
public:
  Master(MySQL& sql) {
    const TimeIt stime;
    const MySQL::tRows master(sql.master("Master")); // Get mysql master tbl
    std::cout << "load master " << stime << std::endl;
    tSet names;
    for (MySQL::tRows::const_iterator it(master.begin()), et(master.end()); it != et; ++it) {
      MasterRow row(*it);
      tRows::iterator jt(mRows.insert(std::make_pair(row.hash(), row)).first);
      const std::string gaugeName(addGauge(jt->second));
      if (!gaugeName.empty()) jt->second.gaugeName(gaugeName);
      for (MasterRow::const_iterator kt(row.begin()), ket(row.end()); kt != ket; ++kt) {
        names.insert(kt->first);
      }
    }
    std::cout << "prepared " << mRows.size() << " master rows in " 
              << stime << " seconds" << std::endl;
    // for (tSet::const_iterator it(names.begin()), et(names.end()); it != et; ++it) {
      // std::cout << *it << std::endl;
    // }
  }

  void pruneParsers(const DataTables& tbls) {
    for (tRows::iterator it(mRows.begin()), et(mRows.end()); it != et; ++it) {
      it->second.pruneParsers(tbls);
    }
  }

  struct GuideBook {
    int bookKey;
    std::string name;
    std::string author;
    std::string edition;
    std::string url;
  };

  void saveGuides(MyDB& db) {
    typedef std::map<std::string, GuideBook> tBooks;
    tBooks mBooks;

    int cnt(1);
    for (tRows::const_iterator it(mRows.begin()), et(mRows.end()); it != et; ++it, ++cnt) {
      const MasterRow& row(it->second); 
      const std::string book(row.guideBook());
      const std::string page(row.pageNumber());
      const std::string run(row.runNumber());
      if (book.empty()) continue;
      if (page.empty() && run.empty()) continue;
      GuideBook bk;
      if (book.find("Soggy Sneakers") == 0) {
        bk.name = "Soggy Sneakers";
        bk.author = "WKCC";
        bk.edition = "4th";
        bk.url = "http://www.wkcc.org/content/soggy-sneakers";
      } else if (book.find("Idaho") == 0) {
        bk.name = "Idaho - The Whitewater State";
        bk.author = "Grant Amaral";
        bk.url = "http://www.amazon.com/Idaho-Whitewater-State-Grant-Amaral/dp/0962234400";
      } else if (book.find("Whitewater Rivers") != book.npos) {
        bk.name = "Whitewater Rivers of Washington";
        bk.author = "Jeff Bennett";
        bk.edition = "2nd";
        bk.url = "http://www.amazon.com/Guide-Whitewater-Rivers-Washington-Trips/dp/096298437X/ref=la_B001ILHHOM_1_3?s=books&ie=UTF8&qid=1410888675&sr=1-3";
      } else {
        std::cout << "Unrecognized guidebook " << book << std::endl;
        continue;
      }
      const std::string bkkey(bk.name + "::" + bk.author + "::" + bk.edition);
      tBooks::const_iterator jt(mBooks.find(bkkey));
      if (jt == mBooks.end()) {
        bk.bookKey = mBooks.size() + 1;
        mBooks.insert(std::make_pair(bkkey, bk));
      } else {
        bk = jt->second;
      }

      MyDB::Stmt s(db);
      s << "INSERT INTO Guides (key,bookKey";
      if (!page.empty()) s << ",pageNumber";
      if (!run.empty()) s << ",runNumber";
      s << ") VALUES (" << cnt << "," << bk.bookKey;
      if (!page.empty()) s << "," << s.quotedString(page);
      if (!run.empty()) s << "," << s.quotedString(run);
      s << ");";
      s.query();
    }

    for (tBooks::const_iterator it(mBooks.begin()), et(mBooks.end()); it != et; ++it) {
      const GuideBook& bk(it->second);
      MyDB::Stmt s(db);
      s << "INSERT INTO GuideBooks (bookKey";
      if (!bk.name.empty()) s << ",name";
      if (!bk.author.empty()) s << ",author";
      if (!bk.edition.empty()) s << ",edition";
      if (!bk.url.empty()) s << ",url";
      s << ") VALUES (" << bk.bookKey;
      if (!bk.name.empty()) s << "," << s.quotedString(bk.name);
      if (!bk.author.empty()) s << "," << s.quotedString(bk.author);
      if (!bk.edition.empty()) s << "," << s.quotedString(bk.edition);
      if (!bk.url.empty()) s << "," << s.quotedString(bk.url);
      s << ");";
      s.query();
    }
  }

  void saveGauges(MyDB& db) {
    TimeIt stime;
    MyDB::Stmt s(db);
    s << "INSERT OR REPLACE INTO gauges (gaugeKey,name,latitude,longitude,description,location,idUSGS,idCBTT"
      << ",state,elevation,drainageArea) VALUES(?,?,?,?,?,?,?,?,?,?,?);";
    db.beginTransaction();
    int cnt(1);
    for (tGauges::iterator it(mGauges.begin()), et(mGauges.end()); it != et; ++it, ++cnt) {
      Gauges::Info& info(it->second);
      info.gaugeKey = cnt;
      s.bind(info.gaugeKey);
      s.bind(info.name);
      s.bind(info.latitude);
      s.bind(info.longitude);
      s.bind(info.description);
      s.bind(info.location);
      s.bind(info.idUSGS);
      s.bind(info.idCBTT);
      s.bind(info.state);
      s.bind(info.elevation);
      s.bind(info.drainageArea);
      s.step();
      s.reset();
    }
    db.endTransaction();

    std::cout << stime << " seconds to save " << mGauges.size() << " " << cnt 
              << " gauges" << std::endl;
    stime.reset();

    for (tRows::iterator it(mRows.begin()), et(mRows.end()); it != et; ++it) {
      MasterRow& row(it->second);
      if (row.gaugeName().empty()) continue;
      tGauges::const_iterator jt(mGauges.find(row.gaugeName()));
      if (jt == mGauges.end()) continue;
      row.gaugeKey(jt->second.gaugeKey);
    }
    std::cout << stime << " seconds to set gaugeKeys" << std::endl;
  }

  void saveCalcs(MyDB& db) {
    TimeIt stime;
    db.beginTransaction();
    for (tRows::iterator it(mRows.begin()), et(mRows.end()); it != et; ++it) {
      MasterRow& row(it->second);
      Calc& calc(row.calc());
      if (calc.empty()) continue;
      if (row.gaugeKey() <= 0) {
        std::cout << "Gaugekey(" << row.gaugeKey() << ") for a calculation is invalide,\n"
                  << row << std::endl;
        continue;
      }
      calc.gaugeKey(row.gaugeKey());
      const std::string name(calc.mkName());
      row.gaugeName(name);
      for (Calc::size_type i(0), e(calc.size()); i < e; ++i) {
        if (!calc[i].qRef()) continue; // not a reference
        const std::string hash(calc[i].keyString());
        tRows::const_iterator jt(mRows.find(hash));
        if (jt == et) {
          std::cout << "problem with hash(" << hash << ") in " << calc << std::endl;
          continue;
        }
        const int key(jt->second.gaugeKey());
        if (key <= 0) {
          std::cout << "problem with hash(" << hash << ") in " << calc << std::endl;
          std::cout << "gaugeKey is " << key << " in\n" << jt->second;
          continue;
        }
        const std::string body(calc[i].comment());
        const Data::Type type(calc[i].type());
        calc[i].encode((int) jt->second.gaugeKey(), body, type);
      }
      MyDB::Stmt s(db);
      s << "UPDATE gauges SET name='" << name << "',"
        << (calc.type() == Data::FLOW ? "calcFlow" : "calcGauge")
        << "='" << calc << "' WHERE gaugeKey=" << row.gaugeKey() << ";";
      s.query();
    }
    db.endTransaction();
    std::cout << stime << " seconds to save calcs" << std::endl;
  }

  void saveMaster(MyDB& db) {
    TimeIt stime;
    MyDB::Stmt s(db);
    s << "INSERT INTO master VALUES("
      << "?,?,?,?,?,?,?,?,?,?" // 10
      << ",?,?,?,?,?,?,?,?,?,?" // 20
      << ",?,?,?,?,?,?,?,?,?,?" // 30
      << ",?,?,?,?,?,?,?,?" // 38
      << ");";
    db.beginTransaction();
    int cnt(1);
    for (tRows::iterator it(mRows.begin()), et(mRows.end()); it != et; ++it, ++cnt) {
      MasterRow& row(it->second);
      s.bind(cnt);
      s.bind(row.qNoShow());
      s.bind(row.date());
      s.bind(row.date());
      s.bind(row.nature());
      s.bind(row.grade());
      s.bind(row.difficulties());
      s.bind(row.displayName());
      s.bind(row.drainage());
      s.bind(row.drainageArea());
      s.bind(row.elevation());
      s.bind(row.elevationLost());
      s.bind(row.features());
      s.bind(row.gradient());
      s.bind(row.latitude());
      s.bind(row.longitude());
      s.bind(NAN); // latitude
      s.bind(NAN); // longitude
      s.bind(row.location());
      s.bind(row.length());
      s.bind(row.lowFlow());
      s.bind(row.highFlow());
      s.bind(); // lowGauge
      s.bind(); // highGauge
      s.bind(row.optimalFlow());
      s.bind(); // optimal high
      s.bind(row.notes());
      s.bind(row.region());
      s.bind(row.remoteness());
      s.bind(row.riverName());
      s.bind(row.scenery());
      s.bind(row.season());
      s.bind(row.section());
      s.bind(row.sortKey());
      s.bind(row.state());
      s.bind(row.watershedType());
      s.bind(row.gaugeKey());
      s.bind(row.gaugeName());
      s.step();
      s.reset();
    }
    db.endTransaction();
    std::cout << stime << " seconds to save " << mRows.size() << " master records" << std::endl;
  }

  typedef std::map<std::string, int> tSources;
  tSources saveSources(MyDB& db) {
    TimeIt stime;
    tSources s2g;
    for (tRows::const_iterator it(mRows.begin()), et(mRows.end()); it != et; ++it) {
      const MasterRow& row(it->second);
      const int gaugeKey(row.gaugeKey());
      if (gaugeKey <= 0) continue;
      const tSet& p(row.parsers());
      if (p.empty()) continue;
      for (tSet::const_iterator jt(p.begin()), jet(p.end()); jt != jet; ++jt) {
        s2g.insert(std::make_pair(*jt, gaugeKey));
      } 
    }
    MyDB::Stmt s(db, "INSERT INTO dataSource VALUES (?,?,0,?);");
    db.beginTransaction();
    tSources a;
    for (tSources::const_iterator it(s2g.begin()), et(s2g.end()); it != et; ++it) {
      a.insert(std::make_pair(it->first, a.size() + 1));
      s.bind(a.size());
      s.bind(it->first);
      s.bind(it->second);
      s.step(); 
      s.reset(); 
    }
    db.endTransaction();
    std::cout << stime << " seconds to save " << a.size() << " sources" << std::endl;
    return a;
  }
}; // Master

std::string
Master::addGauge(const MasterRow& row)
{
  Gauges::Info info;
  info.latitude = row.latitude();
  info.longitude = row.longitude();
  info.description = row.riverName();
  info.location = row.location();
  info.idUSGS = row.idUSGS();
  info.idCBTT = row.idCBTT();
  // info.idUSBR;
  // info.idUnit;
  info.state = row.state();
  // info.county;
  info.elevation = row.elevation();
  info.drainageArea = row.drainageArea();
  // info.bankfullStage;
  // info.floodStage;
  // info.calcFlow;
  // info.calcGauge;
  info.name = !info.idUSGS.empty() ? info.idUSGS :
              !info.idCBTT.empty() ? info.idCBTT : std::string();

  const tSet& parsers(row.parsers());

  if (!parsers.empty() && info.name.empty()) { // Use shortest parser name
    info.name = *(parsers.begin());
    if (parsers.size() > 1) { // Check all names
      for (tSet::const_iterator jt(parsers.begin()), jet(parsers.end()); jt != jet; ++jt) {
        if (info.name.size() > jt->size()) info.name = *jt;
      }
    }
  }

  if (info.name.empty()) {
    return std::string(); // Don't know what to call this one
  }

  tGauges::iterator it(mGauges.find(info.name));
  if ((it == mGauges.end()) && (parsers.size() > 1)) { // check all the parsers
    for (tSet::const_iterator jt(parsers.begin()), jet(parsers.end()); 
         (it == mGauges.end()) && (jt != jet); ++jt) {
      it = mGauges.find(*jt); 
    }
  }

  if (it == mGauges.end()) { // Not known yet
    mGauges.insert(std::make_pair(info.name, info));
  } else {
    Gauges::Info& a(it->second);
    if (isnan(a.latitude)) a.latitude = info.latitude;
    if (isnan(a.longitude)) a.longitude = info.longitude;
    if (isnan(a.elevation)) a.elevation = info.elevation;
    if (isnan(a.drainageArea)) a.drainageArea = info.drainageArea;
    if (isnan(a.bankfullStage)) a.bankfullStage = info.bankfullStage;
    if (isnan(a.floodStage)) a.floodStage = info.floodStage;
    if (a.description.empty()) a.description = info.description;
    if (a.location.empty()) a.location = info.location;
    if (a.idUSGS.empty()) a.idUSGS = info.idUSGS;
    if (a.idCBTT.empty()) a.idCBTT = info.idCBTT;
    if (a.idUSBR.empty()) a.idUSBR = info.idUSBR;
    if (a.idUnit.empty()) a.idUnit = info.idUnit;
    if (a.state.empty()) a.state = info.state;
    if (a.county.empty()) a.county = info.county;
    if (a.calcFlow.empty()) a.calcFlow = info.calcFlow;
    if (a.calcGauge.empty()) a.calcGauge = info.calcGauge;
  }

  return info.name;
}

void
saveData(MySQL& sql,
         MyDB& db, 
         const Master::tSources& sources,
         const DataTables& tbls)
{
  TimeIt stime;
  Data d;
  for (DataTables::const_iterator it(tbls.begin()), et(tbls.end()); it != et; ++it) {
    Master::tSources::const_iterator jt(sources.find(it->gauge));
    if (jt == sources.end()) continue; // Not known
    const Data::Type type(Data::decodeType(it->type));
    if (type == Data::LASTTYPE) continue; // unknown type
    MySQL::tData data(sql.data(it->table));
    std::cout << "Loaded " << data.size() << " points from " << it->table << std::endl;
    for (MySQL::tData::const_iterator kt(data.begin()), ket(data.end()); kt != ket; ++kt) {
      d.add(it->gauge, kt->first, Convert::strTo<double>(kt->second), type, std::string());
    }
  }
  std::cout << stime << " seconds to save data" << std::endl;
}

int
main (int argc,
    char **argv)
{
  int maxCount(argc > 1 ? atoi(argv[1]) : 1000000000);
  MySQL msql("levels_information");
  Master master(msql);
  MySQL dsql("levels_data");
  DataTables dTables(dsql, maxCount);
  master.pruneParsers(dTables);
  MyDB db;
  master.saveGauges(db);
  master.saveCalcs(db);
  master.saveMaster(db);
  master.saveGuides(db);
  Master::tSources sources(master.saveSources(db));
  saveData(dsql, db, sources, dTables);
  // MySQL::tRows urls(msql.master("URLparse"));
}
