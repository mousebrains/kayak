#include "Calc.H"
#include "Tokens.H"
#include "Gauges.H"
#include <sstream>
#include <iostream>
#include <cmath>

Calc::Calc(const int gaugeKey)
  : mGaugeKey(gaugeKey)
  , mType(Data::LASTTYPE)
{
  MyDB::Stmt s(mDB);
  s << "SELECT calcFlow,calcGauge FROM gauges WHERE gaugeKey=" << gaugeKey << ";";
  MyDB::tStrings result(s.queryStrings());
  const MyDB::tStrings::size_type n(result.size());
  if (n > 0 && !result[0].empty()) { // A flow calculation
    split(result[0], Data::FLOW);
  } else if (n > 1 && !result[1].empty()) { // A gauge calculation
    split(result[0], Data::GAUGE);
  }
}

Calc::Calc(const int gaugeKey,
           const std::string& expr,
           const Data::Type type)
  : mGaugeKey(gaugeKey)
  , mType(type)
{
  if (!expr.empty() && (type != Data::LASTTYPE)) {
    split(expr, type);
  }
}

void
Calc::split(const std::string& expr,
            const Data::Type type)
{
  mType = type;

  const Tokens a(expr, " \t\n"); // Split on whitespace

  for (Tokens::const_iterator it(a.begin()), et(a.end()); it != et; ++it) {
    const std::string uniOps("(),*/+-"); // Unary operators
    std::string str(*it); // Local copy
    while (!str.empty()) {
      const std::string::size_type j(str.find_first_of(uniOps));
      if (j == str.npos) { // No ops left
        mExpr.push_back(Field(str));
        break;
      }
      if (j != 0) { // Not first char, so pull off what is in front
        mExpr.push_back(Field(str.substr(0, j)));
      }
      mExpr.push_back(str.substr(j, 1)); // Push the unary operator
      str = str.substr(j+1); // Drop past the operator
    } // while
  } // for

  for (size_type i(0), e(mExpr.size()); i < e; ++i) { // Find reference fields a::b::c
    if (mExpr[i].qRef()) {
      mTime.insert(std::make_pair(mExpr[i].key(), mExpr[i].type()));
    }
  }
} // Calc::load

void
Calc::update(Data& data,
             time_t t0, 
             time_t dt)
{
  const int daysBack(1); 
  const int windowSeconds(2 * 3600);

  t0 = t0 <= 0 ? (time(0) - daysBack * 86400) : t0; // As of now
  dt = dt <= 0 ? windowSeconds : dt; // +- 2 hours

  MyDB::Stmt s(mDB); // Time query
  s << "SELECT ";
  { // Build observation query
    for (tFields::size_type i(0), e(mExpr.size()); i < e; ++i) {
      const Field& f(mExpr[i]);
      if (f.qRef()) {
        mkQuery(s, "AVG(obs)", f.key(), f.type(), t0, dt);
      } else {
        s << f.str();
      }
    }
  }
  s << ",";
  { // Build query for time
    std::string delim;
    s << "(SELECT MAX(";
    for (tTimes::const_iterator it(mTime.begin()), et(mTime.end()); it != et; ++it) {
      s << delim;
      mkQuery(s, "date", it->first, it->second, t0, dt);
      delim = ",";
    }
    s << "))";
  }
  s << ";";

  const std::string& name(mkName());
  data.source().setGauge(name, mGaugeKey); // Make sure this calc source is know and points to gkey

  int rc;

  while ((rc = s.step()) == SQLITE_ROW) {
    const double v(s.getDouble());
    const time_t t(s.getInt());
    if (t > 0 && !isnan(v)) {
      data.add(name, t, v, mType, std::string());
    }
  }

  if (rc != SQLITE_DONE) {
    s.errorCheck(rc, std::string(__FILE__) + " at line " + Convert::toStr(__LINE__));
  }
}

int
Calc::mkQuery(MyDB::Stmt& s,
              const std::string& field,
              const int gaugeKey,
              const Data::Type type,
              const time_t t0,
              const time_t dt) const
{
  s << "(SELECT " << field
    << " FROM data WHERE dataKey in (SELECT dataKey FROM dataSource WHERE gaugeKey=" << gaugeKey
    << ") AND type=" << (int) type
    << " AND abs(date-" << t0 << ")<=" << dt
    << " GROUP BY abs(date-" << t0 << ") ORDER BY abs(date-" << t0 << ") LIMIT 1)";
  return 3; // Number of times t0 needs to bound to 
}

const std::string&
Calc::mkName()
{
  if (mName.empty()) {
    mName = "calc." + Convert::toStr(mGaugeKey);
  }
  return mName;
}

unsigned int
Calc::Field::key() const
{
  const std::string a(keyString());
  int result(0);

  for (std::string::size_type i(a.size()); i > 0; --i) {
    const char c(a[i-1]);
    result *= 36; // % Shift current result
    if ((c >= '0') && (c <= '9')) {
      result += c - '0';
    } else if ((c >= 'A') && (c <= 'Z')) {
      result += (c - 'A') + 10;
    } else if ((c >= 'a') && (c <= 'z')) {
      result += (c - 'a') + 10;
    } else {
     std::cerr << "Unsupported character(" << c << ") in key(" << a << ") at position " << i << std::endl;
     exit (1);
    }
  }

  return result;
}

void
Calc::Field::encode(unsigned int key, 
                    const std::string& comment, 
                    const Data::Type type)
{
  std::string keyValue(key ? "" : "0");

  while (key) {
   const char a(key % 36); // 
   if (a < 10) {
    keyValue.append(1, a + '0');
   } else { // >= 10
    keyValue.append(1, (a - 10) + 'a');
   }
   key /= 36;
  } 
  
  mField = keyValue + "::" + comment + "::" + Convert::toStr(type);
}

std::ostream&
operator << (std::ostream& os,
             const Calc& calc)
{
  for (Calc::size_type i(0), e(calc.size()); i < e; ++i) {
    os << calc[i].str();
  }
  return os;
}
