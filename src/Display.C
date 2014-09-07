#include "Display.H"
#include "DataGet.H"
#include "CGI.H"
#include "HTML.H"
#include "Convert.H"
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
           value : 
           ("<input type='hidden' name='" + key + "' value='" + 
            (value.empty() ? defVal : value) + "' />\n"); 
  }

  std::string selectType(const DataGet& obs) {
    const Data::Type& t(obs.type());
    const MyDB::Stmt::tInts& types(obs.types());
    std::ostringstream oss;
    oss << "<select name='t'>\n";
    for (MyDB::Stmt::tInts::const_iterator it(types.begin()), et(types.end()); it != et; ++it) {
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
    oss << "<input type='hidden' name='t' value='"; // << obs.type() << "'>\n";
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
