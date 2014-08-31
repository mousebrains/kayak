// From the existing mySQL Master table and get all the gauges in db_name and merged_dbs
// From the existing mySQL data tables get all the gauges stored there
// Take the interesection of the two sets
// Ignore any data tables without any data
// Ignore any data tables without recent data
// Save only what remains into SQLite's data and gauges tables
// 
#include "MySQL.H"
#include "Convert.H"
#include "Tokens.H"
#include "Gauges.H"
#include "Calc.H"
#include "Data.H"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>

namespace {
  typedef std::set<std::string> tSet;

  std::ostream& operator << (std::ostream& os, const tSet& a) {
    std::string delim;
    for (tSet::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
      os << delim << *it;
      delim = std::string(",");
    }
    return os;
  }

  std::string masterString(const MySQL::tMaster& row, const std::string& key) {
    MySQL::tMaster::const_iterator it(row.find(key));
    return it == row.end() ? std::string() : it->second;
  }

  double masterDouble(const MySQL::tMaster& row, const std::string& key) {
    MySQL::tMaster::const_iterator it(row.find(key));
    if (it != row.end()) {
      bool flg;
      const double val(Convert::strTo<double>(it->second, &flg));
      if (flg) {
        return val;
      }
    }
    return -1e9;
  }

  struct GaugeInfo {
    int gaugeKey;
    double latitude, longitude;
    std::string location;
    std::string idUSGS;
    std::string idCBTT;
    std::string calcExpr;
    std::string calcTime;
    std::string calcType;
    tSet parsers;
    tSet hashes;
    GaugeInfo(const MySQL::tMaster& row)
      : gaugeKey(-1)
      , latitude(masterDouble(row, "latitude"))
      , longitude(masterDouble(row, "longitude"))
      , location(masterString(row, "gauge_location"))
      , idUSGS(masterString(row, "idUSGS"))
      , idCBTT(masterString(row, "idCBTT"))
      , calcExpr(masterString(row, "calc_expr"))
      , calcTime(masterString(row, "calc_time"))
      , calcType(masterString(row, "calc_type")) 
    {
      const std::string hash(masterString(row, "HashValue"));
      if (!hash.empty())
        hashes.insert(hash);

      const Tokens toks(masterString(row, "parserNames"), " ,\t\n");
      parsers.insert(toks.begin(), toks.end());
    }
    GaugeInfo& merge(const GaugeInfo& rhs) {
      latitude = latitude < -1000 ? rhs.latitude : latitude;
      longitude = longitude < -1000 ? rhs.longitude : longitude;
      idUSGS = idUSGS.size() < rhs.idUSGS.size() ? rhs.idUSGS : idUSGS;
      idCBTT = idCBTT.size() < rhs.idCBTT.size() ? rhs.idCBTT : idCBTT;
      location = location.size() < rhs.location.size() ? rhs.location : location;
      calcExpr = calcExpr.size() < rhs.calcExpr.size() ? rhs.calcExpr : calcExpr;
      calcTime = calcTime.size() < rhs.calcTime.size() ? rhs.calcTime : calcTime;
      calcType = calcType.size() < rhs.calcType.size() ? rhs.calcType : calcType;
      parsers.insert(rhs.parsers.begin(), rhs.parsers.end());
      hashes.insert(rhs.hashes.begin(), rhs.hashes.end());
      return *this;
    }
  };

  std::ostream& operator << (std::ostream& os, const GaugeInfo& a) {
    os << a.gaugeKey
       << "," << a.latitude 
       << "," << a.longitude 
       << "," << a.location 
       << "," << a.idUSGS 
       << "," << a.idCBTT
       << "," << a.parsers
       ;
    return os; 
  }

  typedef std::map<std::string, GaugeInfo> tGaugeInfo;

  std::ostream& operator << (std::ostream& os, const tGaugeInfo& a) {
    for (tGaugeInfo::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
      os << it->first << "=" << it->second << std::endl;
    }
    return os;
  }

  void fixMasterIDs(MySQL::tMaster& row) {
    std::string usgs(masterString(row, "usgs_id"));
    std::string cbtt(masterString(row, "cbtt_id"));
    std::string stationNumber(masterString(row, "StationNumber"));
    std::string nws(masterString(row, "nws_id"));
    std::string nwsli(masterString(row, "nwsli_id"));
    if (!stationNumber.empty() && (stationNumber != usgs)) {
      cbtt = stationNumber;
      stationNumber.clear();
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
      const Tokens toks(masterString(row, "parserNames"), " ,\t\n");
      for (Tokens::const_iterator it(toks.begin()), et(toks.end()); it != et; ++it) {
         const std::string& name(*it);
         if ((name.size() >= 8) && (name.find_first_not_of("0123456789") == name.npos)) {
           usgs = name;
         } else if ((name.size() == 5) && (name.substr(3) == "O3")) {
           cbtt = name;
         } else if ((cbtt.size() < 4) && (name.size() == 4) && (name[3] == 'O')) {
           cbtt = name;
         } else if (cbtt.empty() && (name.size() == 3) && 
                    (name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVXYZabcdefgnamefghijklmnopqrstuvwxyz") == name.npos)) {
           cbtt = name;
         }
      }
    }
    if (!usgs.empty()) {
      row.insert(std::make_pair("idUSGS", usgs));
    }
    if (!cbtt.empty()) {
      row.insert(std::make_pair("idCBTT", cbtt));
    }
  } // fixMasterIDs

  tGaugeInfo::iterator dropDuplicate(tGaugeInfo& a, tGaugeInfo::iterator& it, 
                                      tGaugeInfo::iterator jt) {
    if (jt == a.end()) {
      return it;
    }

    const std::string name(it->first);
    it->second.merge(jt->second);
    a.erase(jt);
    return a.find(name); 
  }
      
  tGaugeInfo::iterator dropDuplicateParsers(tGaugeInfo& a, tGaugeInfo::iterator& it) {
    tSet names(it->second.parsers); // local copy
    for (tSet::const_iterator jt(names.begin()), jet(names.end()); jt != jet;) {
      if (*jt != it->first) {
        it = dropDuplicate(a, it, a.find(*jt));
        if (it->second.parsers.size() > names.size()) {
          names = it->second.parsers;
          jt = names.begin();
          jet = names.end();
          continue;
        }
      }
      ++jt;
    }
    return it;
  }

  tSet parsersInMaster(const tGaugeInfo& a) {
    tSet b;
    for (tGaugeInfo::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
      b.insert(it->second.parsers.begin(), it->second.parsers.end());
    }
    return b;
  }

  void parsersInMaster(MySQL::tMaster& row) {
    if (row.find("calc_expr") == row.end()) { // not a calculate
      MySQL::tMaster::const_iterator it(row.find("merged_dbs"));
      if ((it != row.end()) || ((it = row.find("db_name")) != row.end())) {
        const Tokens toks(it->second, " ,\t\n");
        tSet b;
        for (Tokens::const_iterator kt(toks.begin()), ket(toks.end()); kt != ket; ++kt) {
          if (kt->find("_calc") == kt->npos) {
            b.insert(*kt);
          }
        }
        std::ostringstream oss;
        oss << b;
        row.insert(std::make_pair("parserNames", oss.str()));
      }
    }
  }

  tGaugeInfo gaugesInMaster(MySQL::tRows& a) {
    tGaugeInfo b;

    for (MySQL::tRows::iterator it(a.begin()), et(a.end()); it != et; ++it) {
      MySQL::tMaster& row(*it);
      parsersInMaster(row); // create parserNames
      fixMasterIDs(row); // Fix up USGS and CBTT ids
      GaugeInfo info(row); // Get gauge information
      std::string name(!info.idUSGS.empty() ? info.idUSGS : info.idCBTT);
      if (name.empty() && !info.parsers.empty()) { // Fall back to first parser
        name = *(info.parsers.begin());
      }
      if (!info.calcExpr.empty() && !info.calcTime.empty() && !info.calcType.empty()) { //
        if (name.empty()) { // USGS and CBTT not known
          MySQL::tMaster::const_iterator jt(row.find("db_name"));
          name = jt->second;
          const std::string::size_type i(name.find("_calc"));
          if (i != name.npos) {
            name = name.substr(0, i);
          }
        }
        name += ".calc";
      }

      if (!name.empty()) { // We have a name, so save it
        if (b.find(name) != b.end()) { // This gauge has already been seen
          GaugeInfo& oinfo(b.find(name)->second);
          oinfo.merge(info);
        } else {
          b.insert(std::make_pair(name, info));
        }
      }
    }

    // First check that anything with a USGS id does not have a duplicate CBTT or parsers entry
    for (tGaugeInfo::iterator it(b.begin()); it != b.end(); ++it) {
      if (it->first != it->second.idUSGS) continue;
      it = dropDuplicate(b, it, b.find(it->second.idCBTT));
      it = dropDuplicateParsers(b, it);
    }
    // Check that anything with a CBTT entry does not have a duplicate parsers entry
    for (tGaugeInfo::iterator it(b.begin()); it != b.end(); ++it) {
      if (it->first != it->second.idCBTT) continue;
      it = dropDuplicateParsers(b, it);
    }
    // Now make sure parsers do not duplicate each other
    for (tGaugeInfo::iterator it(b.begin()); it != b.end(); ++it) {
      if (!it->second.parsers.empty() && (it->first == *(it->second.parsers.begin()))) {
        it = dropDuplicateParsers(b, it);
      }
    }

    return b;
  }

  tSet tablesToKeep(MySQL& sql, const tSet& pim) {
    const MySQL::tTables tables(sql.tables());
    tSet a;
    const time_t now(time(0));
    const time_t maxAge(120 * 86400); // 120 days

    for (MySQL::tTables::const_iterator it(tables.begin()), et(tables.end()); it != et; ++it) {
      const std::string& tbl(*it);
      const std::string::size_type offset(tbl.find('_'));
      if (offset == tbl.npos) {
        continue;
      }
      const std::string type(tbl.substr(0,offset));
      const std::string name(tbl.substr(offset+1));
      if (pim.find(name) == pim.end()) { // Not know in master, so skip
        continue;
      }
      if (name.find("_calc") != name.npos) { // skip
        continue;
      }
      if (!sql.nData(tbl)) {
        continue;
      }
      if ((now - sql.maxDataTime(tbl)) > maxAge) { 
        continue;
      }
      a.insert(tbl);
    }

    return a;
  }

  void pruneParsers(tGaugeInfo& gim, const tSet& p) {
    tSet a;

    for (tSet::const_iterator it(p.begin()), et(p.end()); it != et; ++it) {
      a.insert(it->substr(it->find('_')+1));
    }

    size_t nTotal(0), nDropped(0);

    for (tGaugeInfo::iterator it(gim.begin()), et(gim.end()); it != et; ++it) {
      tSet& aa(it->second.parsers); // reference copy
      const tSet bb(aa); // local copy
      nTotal += bb.size();
      for (tSet::const_iterator jt(bb.begin()), jet(bb.end()); jt != jet; ++jt) {
        if (a.find(*jt) == a.end()) {
          aa.erase(*jt);
          ++nDropped;
        }
      }
    }
  }

  void saveGauges(MyDB& db, tGaugeInfo& info) {
    Calc::tHash2Key hash2gauge;
    tSet calcs;

    // insert all gauges into gauges table
    MyDB::Stmt s(db, "INSERT INTO gauges (name,latitude,longitude,location,idUSGS,idCBTT) VALUES(?,?,?,?,?,?);");

    db.beginTransaction();
    db.query("DELETE FROM gauges;");
    for (tGaugeInfo::iterator it(info.begin()), et(info.end()); it != et; ++it) {
      GaugeInfo& a(it->second);
      s.bind(it->first);
      s.bind(a.latitude);
      s.bind(a.longitude);
      s.bind(a.location);
      s.bind(a.idUSGS);
      s.bind(a.idCBTT);
      s.step();
      s.reset();

      if (!a.calcExpr.empty()) {
        calcs.insert(it->first);
      }
      for (tSet::const_iterator jt(a.hashes.begin()), jet(a.hashes.end()); jt != jet; ++jt) {
        hash2gauge.insert(std::make_pair(*jt, it->second.gaugeKey));
      }
    }
    db.endTransaction();

    // Get gaugeKey's from what was just inserted

    MyDB::tInts keys(db.queryInts("SELECT gaugeKey FROM gauges;"));

    // insert into dataSource table with gaugeKey 

    MyDB::Stmt s1(db, "INSERT INTO dataSource (name,gaugeKey) VALUES(?,?);");
    size_t cnt(0);
    db.beginTransaction();
    db.query("DELETE FROM dataSource;");
    for (tGaugeInfo::iterator it(info.begin()), et(info.end()); it != et; ++it) {
      GaugeInfo& a(it->second);
      a.gaugeKey = keys[cnt++]; 
      const tSet& parsers(a.parsers);
      for (tSet::const_iterator jt(parsers.begin()), jet(parsers.end()); jt != jet; ++jt) {
        s1.bind(*jt);
        s1.bind(a.gaugeKey);
        s1.step();
        s1.reset();
      }
    }

    for (tSet::const_iterator it(calcs.begin()), et(calcs.end()); it != et; ++it) {
      const GaugeInfo& a(info.find(*it)->second);
      Calc calc(a.calcExpr, a.calcTime);
      if (!calc.remap(hash2gauge)) {
        std::cout << "Failed to remap calculation " << a.calcExpr << " " << a.calcTime << std::endl;
        continue;
      }
      std::ostringstream oss; // Individual commands since calcs are expressions
      oss << "UPDATE gauges SET ";
      if (a.calcType == "flow") {
        oss << "calcFlow='" << calc.expr() << "', calcFlowTime='" << calc.time();
      } else if (a.calcType == "gauge") {
        oss << "calcGauge='" << calc.expr() << "', calcGaugeTime='" << calc.time();
      } else {
        std::cerr << "Unsupported calculation type '" << a.calcType << "'" << std::endl;
        continue;
      }
      oss << "' WHERE gaugeKey=" << a.gaugeKey << ";";
      db.query(oss.str());
    }
    db.endTransaction();
    std::cout << "Saved " << calcs.size() << " calculations" << std::endl;
  } // saveGauges

  void saveMaster(MyDB& db, MySQL::tRows& master, const tGaugeInfo& gim) {
    typedef std::map<std::string, std::string> tMap;
    tMap hash2gauge;

    for (tGaugeInfo::const_iterator it(gim.begin()), et(gim.end()); it != et; ++it) {
      const GaugeInfo& a(it->second);
      for (tSet::const_iterator jt(a.hashes.begin()), jet(a.hashes.end()); jt != jet; ++jt) {
        hash2gauge.insert(std::make_pair(*jt, it->first));
      }
    }

    tSet toIgnore;
    tMap toCopy;
    tSet cols;

    toIgnore.insert("HashValue");
    toIgnore.insert("StationNumber");
    toIgnore.insert("approved");
    toIgnore.insert("db_source");
    toIgnore.insert("calc_expr");
    toIgnore.insert("calc_time");
    toIgnore.insert("calc_type");
    toIgnore.insert("cbtt_id");
    toIgnore.insert("data_source");
    toIgnore.insert("db_name");
    toIgnore.insert("db_rating");
    toIgnore.insert("idCBTT");
    toIgnore.insert("idUSGS");
    toIgnore.insert("map_name");
    toIgnore.insert("merged_dbs");
    toIgnore.insert("nws_id");
    toIgnore.insert("nwsli_id");
    toIgnore.insert("snotel_id");
    toIgnore.insert("source_name");
    toIgnore.insert("usgs_id");
    toIgnore.insert("parserNames");
    toIgnore.insert("cfs_to_gauge_converter");
    toIgnore.insert("cfs_to_gauge_data");
    toIgnore.insert("guide_book");
    toIgnore.insert("PageNumber");
    toIgnore.insert("RunNumber");

    toCopy["drainage"] = "drainage";
    toCopy["drainage_area"] = "drainageArea";
    toCopy["Nature"] = "nature";
    toCopy["difficulties"] = "difficulties";
    toCopy["display_name"] = "displayName";
    toCopy["drainage"] = "drainage";
    toCopy["elevation"] = "elevation";
    toCopy["elevation_lost"] = "elevationLost";
    toCopy["features"] = "features";
    toCopy["gradient"] = "gradient";
    toCopy["high_flow"] = "highFlow";
    toCopy["low_flow"] = "lowFlow";
    toCopy["optimal_flow"] = "optimalLowFlow";
    toCopy["length"] = "length";
    toCopy["no_show"] = "qNoShow";
    toCopy["notes"] = "notes";
    toCopy["region"] = "region";
    toCopy["remoteness"] = "remoteness";
    toCopy["river_name"] = "riverName";
    toCopy["scenery"] = "scenery";
    toCopy["season"] = "season";
    toCopy["section"] = "section";
    toCopy["sort_key"] = "sortKey";
    toCopy["state"] = "state";
    toCopy["watershed_type"] = "watershedType";

    db.beginTransaction();
    db.query("DELETE FROM master;");

    for (MySQL::tRows::iterator it(master.begin()), et(master.end()); it != et; ++it) {
      MySQL::tMaster& row(*it);
      tMap contents;
      const GaugeInfo *ptr(0);
      if (row.find("HashValue") != row.end()) {
        const std::string& hash(row.find("HashValue")->second);
        if (hash2gauge.find(hash) != hash2gauge.end()) {
          const std::string& name(hash2gauge.find(hash)->second);
          ptr = &(gim.find(name)->second);
          contents.insert(std::make_pair("gaugeName", name));
          contents.insert(std::make_pair("gaugeKey", Convert::toStr(ptr->gaugeKey)));
        }
      }
      for (MySQL::tMaster::const_iterator jt(row.begin()), jet(row.end()); jt != jet; ++jt) {
        if (jt->second.empty() || (toIgnore.find(jt->first) != toIgnore.end())) {
          continue;
        }
        if (toCopy.find(jt->first) != toCopy.end()) {
          contents.insert(std::make_pair(toCopy[jt->first], jt->second));
          continue;
        }
        if (jt->first == "gauge_location") {
          if (!ptr || (ptr->location != jt->second)) {
            contents.insert(std::make_pair("location", jt->second));
          }
          continue;
        }
        if (jt->first == "latitude") {
          if (!ptr || (ptr->latitude != Convert::strTo<double>(jt->second))) {
            contents.insert(std::make_pair("latitudePutin", jt->second));
          }
          continue;
        }
        if (jt->first == "longitude") {
          if (!ptr || (ptr->longitude != Convert::strTo<double>(jt->second))) {
            contents.insert(std::make_pair("longitudePutin", jt->second));
          }
          continue;
        }
        if (jt->first == "class") {
          if (contents.find("class") == contents.end()) {
            contents.insert(std::make_pair("class", jt->second));
          }
          continue;
        }
        if (jt->first == "class_flow") {
          if (contents.find("class") != contents.end()) {
            contents.erase(contents.find("class"));
          }
          contents.insert(std::make_pair("class", jt->second));
          continue;
        }
        if (jt->first == "date") {
          struct tm tt;
          strptime(jt->second.c_str(), "%Y-%m-%d %H:%M:%S", &tt);
          const std::string t(Convert::toStr(mktime(&tt)));
          contents.insert(std::make_pair("created", t));
          contents.insert(std::make_pair("modified", t));
          continue;
        }
        cols.insert(jt->first);
        continue;
      }
      if (!contents.empty()) {
        std::ostringstream oss;
        oss << "INSERT INTO master (";
        std::string delim;
        for (tMap::const_iterator jt(contents.begin()), jet(contents.end()); jt != jet; ++jt) {
          oss << delim << jt->first;
          delim = ",";
        }
        delim = ") VALUES ('";
        for (tMap::const_iterator jt(contents.begin()), jet(contents.end()); jt != jet; ++jt) {
          oss << delim << jt->second;
          delim = "','";
        }
        oss << "');";
        db.query(oss.str());
        row.insert(std::make_pair("key", Convert::toStr(db.lastInsertRowid())));
      }
    }

    db.endTransaction();

    for (tSet:: const_iterator it(cols.begin()), et(cols.end()); it != et; ++it) {
       std::cout << *it << std::endl;
    }
  } // saveMaster

  std::string renameGuideBook(const std::string& name) {
    if (name.find("Soggy Sneakers") != name.npos) {
      return "Soggy Sneakers";
    }
    if (name.find("Whitewater Rivers ") != name.npos) {
      return "Whitewater Rivers Of Washington";
    }
    return name;
  }

  void saveGuideBook(MyDB& db, MySQL::tRows& master) {
    // Now build guide book information
    typedef std::map<std::string, size_t> tBook2key;
    tBook2key book2key;

    // get all the book names

    for (MySQL::tRows::const_iterator it(master.begin()), et(master.end()); it != et; ++it) {
      const MySQL::tMaster& row(*it);
      if (row.find("key") == row.end()) continue; // no master key, so skip
      if (row.find("guide_book") == row.end()) continue; // no master guide book, so skip
      const std::string name(renameGuideBook(row.find("guide_book")->second));
      book2key.insert(std::make_pair(name, 0));
    }

    db.beginTransaction();
    db.query("DELETE FROM GuideBooks;");
    db.query("DELETE FROM Guides;");

    for (tBook2key::iterator it(book2key.begin()), et(book2key.end()); it != et; ++it) {
      const bool qSoggy(it->first == "Soggy Sneakers");
      const std::string url(qSoggy ? "http://www.wkcc.org/content/soggy-sneakers" : "");
      std::ostringstream oss;
      oss << "INSERT INTO GuideBooks (name,url) VALUES('" << it->first << "','" << url << "');";
      db.query(oss.str());
      it->second = db.lastInsertRowid();
    }

    // Now insert into Guides

    for (MySQL::tRows::const_iterator it(master.begin()), et(master.end()); it != et; ++it) {
      const MySQL::tMaster& row(*it);
      if (row.find("key") == row.end()) continue; // no master key, so skip
      if (row.find("guide_book") == row.end()) continue; // no master guide book, so skip
      const std::string key(row.find("key")->second);
      const std::string name(renameGuideBook(row.find("guide_book")->second));
      const std::string page(row.find("PageNumber") == row.end() ? "" : row.find("PageNumber")->second);
      const std::string run(row.find("RunNumber") == row.end() ? "" : row.find("RunNumber")->second);
      const size_t bookKey(book2key[name]);
      std::ostringstream oss;
      oss << "INSERT INTO Guides (key,bookKey,pageNumber,runNumber) VALUES("
          << key << "," << bookKey << ",'" << page << "','" << run << "');";
      db.query(oss.str());
    }

    db.endTransaction();
  } // saveGuideBook

  void saveURLs(const MySQL::tRows& urls, MyDB& db) {
    db.query("DELETE FROM dataURLs;");

    for (MySQL::tRows::const_iterator it(urls.begin()), et(urls.end()); it != et; ++it) {
       const MySQL::tMaster& row(*it);
       if ((row.find("parser") != row.end()) && (row.find("URL") != row.end())) {
         const std::string& url(row.find("URL")->second);
         const std::string& parser(row.find("parser")->second);
         db.query("INSERT INTO dataURLs (url,parser) VALUES('" + url + "','" + parser + "');");
      }
    }
  }

  void saveData(MySQL& sql, MyDB& db, const tSet& names, int maxCount) {
    db.query("DELETE FROM data;");

    Data data;

    for (tSet::const_iterator it(names.begin()), et(names.end()); it != et; ++it) {
      const std::string& name(*it);
      const std::string::size_type offset(name.find('_'));
      if (offset == name.npos) continue;
      const std::string type(name.substr(0,offset));
      const std::string src(name.substr(offset+1));
      Data::Type tNum;
      if (type == "flow") {
        tNum = Data::FLOW;
      } else if (type == "gauge") {
        tNum = Data::GAUGE;
      } else if (type == "temperature") {
        tNum = Data::TEMPERATURE;
      } else {
        std::cerr << "Unrecognized type '" << type << "'" << std::endl;
        exit(1);
      } 
      MySQL::tData rows(sql.data(name));
      std::cout << name << " " << tNum << " " << rows.size() << std::endl;
      for (MySQL::tData::const_iterator jt(rows.begin()), jet(rows.end()); jt != jet; ++jt) {
        bool flg;
        const double value(Convert::strTo<double>(jt->second, &flg));
        if (flg) {
          data.add(src, jt->first, value, tNum, std::string());
        }
      }
      if (--maxCount < 0) break;
    }
  }
} // Anonymouse namespace

int
main (int argc,
      char **argv)
{
  int maxCount(argc > 1 ? atoi(argv[1]) : 1000000000);
  MyDB db;
  MySQL msql("levels_information");
  saveURLs(msql.master("URLparse"), db);

  MySQL::tRows master(msql.master("Master")); // Get mysql master tbl
  tGaugeInfo gim(gaugesInMaster(master));
  tSet pim(parsersInMaster(gim));
 
  MySQL sql("levels_data");
  const tSet tblKeep(tablesToKeep(sql, pim)); // All data tables we want to transfer
  pruneParsers(gim, tblKeep); // Prune parsers in gim

  std::cout << "Master " << master.size() 
            << " parsers in master " << pim.size() 
            << " keep " << tblKeep.size()
            << " gim " << gim.size()
            << std::endl;

  saveGauges(db, gim);
  saveMaster(db, master, gim);
  saveGuideBook(db, master);

  saveData(sql, db, tblKeep, maxCount); // Store data in sqlite database

  db.checkPoint(); // Force a checkpoint in the database
}
