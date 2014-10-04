#include "Display.H"
#include "DataGet.H"
#include "CGI.H"
#include "HTML.H"
#include "Convert.H"
#include "Calc.H"
#include "Tokens.H"
#include "Master.H"
#include <cmath>
#include <sstream>

namespace {
  const CGI cgi;

  std::string dateInput(const std::string& name, const time_t t) {
    std::string str("<input type='date' name='" + name);
    if (t > 0) {
      str += "' value='" + Convert::toStr(t, "%Y-%m-%d");
    }
    str += "' />\n";
    return str;
  }

  std::string submitInput(const std::string& value) {
    return "<input type='submit' name='o' value='" + value + "' />\n";
  }

  std::string hiddenInput(const std::string& key, const std::string& defVal) {
    const std::string& value(cgi.get(key));
    return defVal.empty() && value.empty() ? 
           std::string() : 
           ("<input type='hidden' name='" + key + "' value='" + 
            (value.empty() ? defVal : value) + "' />\n"); 
  }

  std::string selectType(const DataGet& obs) {
    const Data::Type& t(obs.type());
    const MyDB::tInts& types(obs.types());
    std::ostringstream oss;
    oss << "<select name='t'>\n";
    for (MyDB::tInts::const_iterator it(types.begin()), et(types.end()); it != et; ++it) {
      const Data::Type type((Data::Type) *it);
      oss << "<option value='" << type << "'"
          << (t == type ? " selected" : "")
          << ">";
      switch (type) {
        case Data::FLOW: oss << "Flow"; break;
        case Data::GAUGE: oss << "Gauge"; break;
        case Data::TEMPERATURE: oss << "Temp"; break;
        case Data::INFLOW:
        case Data::LASTTYPE: oss << "???"; break;
      }
      oss << "</option>\n";
    }
    oss << "</select>\n";
    return oss.str();
  }
} // anonymous

std::string
Display::dateForm(const DataGet& obs,
                  const std::string& id)
{
  const size_t nTypes(obs.types().size());

  if (nTypes == 0) { // No data to select dates for
    return std::string();
  }

  const bool qObs(!obs.empty());
  const bool qAnObs(!qObs && !obs.anObs().empty());
  const time_t stime(qObs ? obs.begin()->first  : (qAnObs ? obs.anObs().begin()->time  : 0));
  const time_t etime(qObs ? obs.rbegin()->first : (qAnObs ? obs.anObs().rbegin()->time : 0)); 
  std::ostringstream oss;
  oss << "<div id='dateForm'>\n<form id='" << id << "'>\n"
      << dateInput("sdate", stime)
      << dateInput("edate", etime);

  if (nTypes > 1) {
    oss << selectType(obs);
  }

  if (qAnObs) {
    oss << submitInput("Database");
  }

  oss << submitInput("Plot")
      << submitInput("Table")
      << submitInput("Info")
      << "<input type='hidden' name='h' value='" << obs.hash() << "'>\n";

  if (nTypes == 1) {
    oss << "<input type='hidden' name='t' value='" << obs.type() << "'>\n";
  }

  oss << hiddenInput("nc", "0")
      << "<noscript>\n"
      << "<input type='hidden' name='js' value='0' />\n"
      << "</noscript>\n"
      << "</form>\n";

  const double lat(obs.latitude());
  const double lon(obs.longitude());

  if (fabs(lat) <= 90 && fabs(lon) <= 180) {
    oss << HTML::mapForm(lat, lon) << HTML::weatherForm(lat, lon);
  }

  oss << "</div>\n";

  return oss.str();
}

bool
Display::maybeLatLon(std::ostream& os,
                     const double lat,
                     const double lon,
                     const std::string& prefix,
                     const std::string& suffix)
{
  if (isnan(lat) || isnan(lon) || (fabs(lat) > 90) || (fabs(lon) > 180)) return false;

  os << "<li>" 
     << prefix
     << "<a href='" << HTML::mapURL(lat, lon) << "'>"
     << Convert::toLatLon(lat) << ", " << Convert::toLatLon(lon)
     << "</a>"
     << " "
     << "<a href='" << HTML::weatherURL(lat, lon) << "'> Weather Forecast</a>"
     << suffix 
     << "</li>\n";

  return true;
}

bool
Display::maybeLatLon(std::ostream& os,
                     const double lat0,
                     const double lon0,
                     const double lat1,
                     const double lon1,
                     const std::string& prefix,
                     const std::string& suffix)
{
  const bool q0(isnan(lat0) || isnan(lon0) || (fabs(lat0) > 90) || (fabs(lon0) > 180));
  const bool q1(isnan(lat1) || isnan(lon1) || (fabs(lat1) > 90) || (fabs(lon1) > 180));

  if (q0 && q1) return false;
  if (q0) return maybeLatLon(os, lat1, lon1, prefix, suffix);
  if (q1) return maybeLatLon(os, lat0, lon0, prefix, suffix);

  os << "<li>"
     << prefix
     << "<a href='" << HTML::mapURL(lat0, lon0, lat1, lon1) << "'>"
     << Convert::toLatLon(lat0) << ", " << Convert::toLatLon(lon0)
     << " to "
     << Convert::toLatLon(lat1) << ", " << Convert::toLatLon(lon1)
     << "</a>"
     << " "
     << "<a href='" << HTML::weatherURL((lat0 + lat1) / 2, (lon0 + lon1) / 2) 
     << "'> Weather Forecast</a>"
     << suffix 
     << "</li>\n";
    
  return true;
}

bool
Display::maybe(std::ostream& os,
               const double min,
               const double max,
               const std::string& prefix,
               const std::string& suffix)
{
  const bool qMin(!isnan(min) && min != 0);
  const bool qMax(!isnan(max) && max != 0);

  if (!qMin && !qMax) return false;

  os << "<li>" << prefix;

  if (qMin) {
    if (qMax) {
      os << min << " to " << max;
    } else {
      os << min;
    }
  } else {
    os << max;
  }

  os << suffix << "</li>\n";

  return true;
}

bool
Display::maybeMinMax(std::ostream& os,
                     const double min,
                     const double max,
                     const std::string& prefix,
                     const std::string& suffix)
{
  const bool qMin(!isnan(min));
  const bool qMax(!isnan(max));

  if (!qMin && !qMax) return false;

  os << "<li>" << prefix;

  if (qMin) {
    if (qMax) {
      os << "[" << min << ", " << max << "]";
    } else {
      os << ">= " << min;
    }
  } else {
    os << "<= " << max;
  }

  os << suffix << "</li>\n";

  return true;
}

bool 
Display::maybeCalc(std::ostream& os, 
                   const std::string& str,
                   Master& master,
                   const std::string& prefix,
                   const std::string& suffix) 
{
  if (str.empty()) return false;
  Calc calc(0, str, Data::FLOW);

  os << "<li>" 
     << prefix;
 
  for (Calc::size_type i(0), e(calc.size()); i < e; ++i) {
    if (calc[i].qRef()) {
      const int gaugeKey(calc[i].key());
      os << "<a href='?o=g&amp;h=" << Master::mkHash(gaugeKey) << "'>"
         << calc[i].str()
         << "</a>";
    } else {
      os << calc[i].str();
    }
  } // for
  os << suffix 
     << "</li>\n";
  return true;
} // maybeCalc

namespace Display {

template<>
bool
maybe(std::ostream& os,
      const int& val,
      const std::string& prefix,
      const std::string& suffix)
{
  if (val == 0) return false;

  os << "<li>"
     << prefix
     << val
     << suffix
     << "</li>\n";

  return true;
}

template<>
bool
maybe(std::ostream& os,
      const size_t& val,
      const std::string& prefix,
      const std::string& suffix)
{
  if (val == 0) return false;

  os << "<li>"
     << prefix
     << val
     << suffix
     << "</li>\n";

  return true;
}

template<>
bool
maybe(std::ostream& os,
      const double& val,
      const std::string& prefix,
      const std::string& suffix)
{
  if (isnan(val)) return false;

  os << "<li>"
     << prefix
     << val
     << suffix
     << "</li>\n";

  return true;
}

template<>
bool
maybe(std::ostream& os,
      const std::string& str,
      const std::string& prefix,
      const std::string& suffix)
{
  if (str.empty()) return false;

  os << "<li>"
     << prefix
     << str
     << suffix
     << "</li>\n";

  return true;
}

template<>
bool
maybe(std::ostream& os,
      const time_t& t,
      const std::string& prefix,
      const std::string& suffix)
{
  if (t <= 0) return false;

  os << "<li>"
     << prefix
     << Convert::toStr(t, "%Y-%m-%d %H:%M")
     << suffix
     << "</li>\n";

  return true;
}
} // namespace Display

bool
Display::maybeHRef(std::ostream& os,
                   const std::string& str,
                   const std::string& url,
                   const std::string& prefix,
                   const std::string& suffix)
{
  if (str.empty() || url.empty()) return maybe(os, str, prefix, suffix);

  os << "<li>"
     << prefix
     << "<a href='" << url << "'>" << str << "</a>"
     << suffix
     << "</li>\n";

  return true;
}

Types::Keys
Display::dehashKeys(const CGI& cgi,
                    const std::string& key)
{
  const Tokens toks(cgi.get(key), " ,\n\t");
  Types::Keys a;
  
  for (Tokens::const_iterator it(toks.begin()), et(toks.end()); it != et; ++it) {
    a.insert(Master::deHash(*it));
  }
  return a;
}
