#include "Calc.H"
#include "Tokens.H"
#include <sstream>
#include <iostream>
#include <cmath>

Calc::Calc(const int gaugeKey)
  : mGaugeKey(gaugeKey)
  , mDataKey(-1)
{
  MyDB::Stmt s(mDB);
  s << "SELECT calcFlow,calcGauge FROM gauges WHERE gaugeKey=" << gaugeKey << ";";
  MyDB::Stmt::tStrings result(s.queryStrings());
  const MyDB::Stmt::tStrings::size_type n(result.size());
  if (n > 0 && !result[0].empty()) {
    load(result[0], Data::FLOW);
  } else if (n > 1 && !result[1].empty()) {
    load(result[0], Data::GAUGE);
  }
}

Calc::Calc(const int gaugeKey,
           const std::string& expr,
           const Data::Type type)
  : mGaugeKey(gaugeKey)
  , mDataKey(-1)
{
  load(expr, type);
}

Calc::tStrings
Calc::split(const std::string& expr)
{
  const Tokens a(expr, " \t\n"); // Split on whitespace
  tStrings toks;

  for (Tokens::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    const std::string uniOps("(),*/+-"); // Unary operators
    std::string str(*it); // Local copy
    while (!str.empty()) {
      const std::string::size_type j(str.find_first_of(uniOps));
      if (j == str.npos) { // No ops left
        toks.push_back(str);
        break;
      }
      if (j != 0) { // Not first char, so pull off what is in front
        toks.push_back(str.substr(0, j));
      }
      toks.push_back(str.substr(j, 1));
      str = str.substr(j+1); // Drop past operator
    } // while
  } // for

  return toks;
}

void
Calc::load(const std::string& expr,
           const Data::Type type)
{
  mType = type;
  tStrings toks(split(expr));

  for (tStrings::iterator it(toks.begin()), et(toks.end()); it != et; ++it) {
    const std::string::size_type i(it->find("::"));
    if (i == it->npos) {
      mExpr += *it;
      continue; 
    }
    const std::string::size_type j(it->rfind("::"));
    if (i == j) {
      mExpr += *it;
      continue; 
    }
    const std::string key(it->substr(0, i));
    const std::string type(it->substr(j+2));
    mExpr += buildQuery("AVG(obs)", key, type);
    mTime.push_back(buildQuery("date", key, type));
  } // for
} // Calc::load

std::string
Calc::buildQuery(const std::string& name,
                 const std::string& gaugeKey,
                 const std::string& type) const
{
  std::ostringstream oss;
  oss << "(SELECT " << name 
      << " FROM data WHERE dataKey in (SELECT dataKey FROM dataSource WHERE gaugeKey="
      << gaugeKey
      << ") AND type="
      << (int) Data::decodeType(type)
      << " GROUP BY date ORDER BY date DESC LIMIT 1)";
  return oss.str();
}

int
Calc::dataKey()
{
  if (mDataKey > 0) return mDataKey;

  { // Get an existing dataKey
    MyDB::Stmt s(mDB);
    s << "SELECT dataKey FROM dataSource WHERE gaugeKey=" << mGaugeKey << ";";
    MyDB::Stmt::tInts result(s.queryInts());
    if (!result.empty()) {
      mDataKey = result[0];
      return result[0];
    }
  }
  MyDB::Stmt s(mDB);
  s << "INSERT INTO dataSource (name,gaugeKey) VALUES('calcFor" 
    << mGaugeKey << "'," << mGaugeKey << ");";
  s.query();
  mDataKey = mDB.lastInsertRowid();
  return mDataKey;
}

time_t
Calc::time()
{
  MyDB::Stmt s(mDB);
  s << "SELECT MIN(";
  for (tStrings::size_type i(0), e(mTime.size()); i < e; ++i) {
    s << (i != 0 ? "," : "") << mTime[i];
  }
  s << ");";
  MyDB::Stmt::tInts a(s.queryInts());
  return (time_t) (a.empty() ? 0 : a[0]);
}

double
Calc::calc()
{
  MyDB::Stmt s(mDB);
  s << "SELECT AVG(" << mExpr << ");";
  MyDB::Stmt::tDoubles a(s.queryDoubles());
  return a.empty() ? NAN : a[0];
}

void
Calc::update()
{
  if ((mType != Data::FLOW) && (mType != Data::GAUGE)) return;
  const time_t t(time());
  if (t <= 0) return;
  const double x(calc());
  if (isnan(x)) return;
  const int key(dataKey());
  if (key <= 0) return;

  MyDB::Stmt s(mDB);
  s << "INSERT OR REPLACE INTO data (dataKey,type,date,urlKey,obs) VALUES(" 
    << key
    << "," << (int) mType
    << "," << t
    << ",0"
    << "," << x
    << ");";
  s.query();
}

std::ostream&
operator << (std::ostream& os,
             Calc& calc)
{
  os << "Calc gaugeKey " << calc.mGaugeKey << " type " << calc.mType 
     << " dataKey " << calc.dataKey()
     << " time " << calc.time()
     << " calc " << calc.calc()
     << std::endl;
  // os << calc.mExpr << std::endl;
  // for (Calc::tStrings::size_type i(0), e(calc.mTime.size()); i < e; ++i) {
    // os << "time[" << i << "] = '" << calc.mTime[i] << "'" << std::endl;
  // }
  return os;
}
