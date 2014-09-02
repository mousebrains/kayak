#include "Display.H"
#include "Master.H"
#include "Gauges.H"
#include "CGI.H"
#include "HTML.H"
#include "HTTP.H"
#include "Convert.H"
#include "GuideBook.H"
#include <iostream>
#include <cmath>

namespace {
  bool maybe(HTML& html, const std::string& str, 
             const std::string& prefix = "", const std::string& suffix = "") {
    if (!str.empty()) {
      html << "<li>" << prefix << str << suffix << "</li>\n";
      return true;
    } 
    return false;
  }

  bool maybe(HTML& html, const double value, 
             const std::string& prefix = "", const std::string& suffix = "") {
    if (!isnan(value) && value != 0) {
      html << "<li>" << prefix << value << suffix << "</li>\n";
      return true;
    }
    return false;
  }

  bool maybe(HTML& html, const double val0, const double val1,
             const std::string& prefix = "", const std::string& suffix = "") {
    if ((!isnan(val0) && val0 != 0) || (!isnan(val1) && val1 != 0)) {
      html << "<li>" << prefix;
      if (val0 != 0 && val1 != 0) {
        html << val0 << " to " << val1;
      } else {
        html << (val0 != 0 ? val0 : val1);
      }
      html << suffix << "</li>";
      return true;
    }
    return false;
  }

  bool maybe(HTML& html, const time_t& t, 
             const std::string& prefix = "", const std::string& suffix = "") {
    if (t != 0) {
      html << "<li>" << prefix 
           << Convert::toStr(t, "%Y-%m-%d")
           << suffix << "</li>\n";
      return true;
    } 
    return false;
  }

  bool gmaybe(HTML& html, const double v0, const double v1,
              const std::string& prefix = "", const std::string& suffix = "") {
    return maybe(html, isnan(v0) || (v0 == 0) ? v1 : v0, prefix, suffix);
  }
   
  bool maybeLatLon(HTML& html, const double lat, const double lon, 
                   const std::string& prefix = "") { 
    const std::string wx(html.weatherURL(lat, lon));
    if (!wx.empty()) {
      const std::string map(html.mapURL(lat, lon));
      html << "<li>" << prefix << "Lat/Lon: "  
           << "<a href='" << map << "'>"
           << Convert::toLatLon(lat) << ", " 
           << Convert::toLatLon(lon) 
           << "</a>"
           << "<a href='" << wx << "'>Weather Forecast</a></li>\n";
      return true;
    }
    return false;
  }
} // anonymouse

int
Display::info()
{
  const CGI cgi;
  const std::string hash(cgi.get("h", std::string()));
  
  if (hash.empty()) { // No hash
    return HTTP(std::cout).errorPage("Hash not set");
  }

  const size_t key(Master::deHash(hash));  
  Master master;
  Gauges gauges;
  Data data;
  const Master::Info info(master.getInfo(key));
  const Gauges::Info ginfo(info.gaugeKey > 0 ? gauges.getInfo(info.gaugeKey) : Gauges::Info());
  const std::string location(info.location.empty() ? ginfo.location : info.location);
  const std::string title(info.displayName + (location.empty() ? "" : ("@" + location)));
  const double lat(fabs(info.latitudePutin) <= 90 ? info.latitudePutin : ginfo.latitude);
  const double lon(fabs(info.longitudePutin) <= 180 ? info.longitudePutin : ginfo.longitude);
  const GuideBook guides(key);
  const MyDB::tInts types(data.types(data.source().gaugeKey2Keys(info.gaugeKey)));

  HTML html;
  html << html.header()
       << html.myStyle()
       << "<title>" << title << "</title>\n"
       << "</head>\n<body>\n"
       << "<h1>" << title << "</h1>\n";

  if (!types.empty()) {
    html << "<form>\n";
    if (types.size() == 1) {
       html << "<input type='hidden' name='t' value='" 
            << ((Data::Type) *(types.begin())) << "'>\n";
    } else {
      bool qFirst(true);
      html << "<select name='t'>\n";
      
      for (MyDB::tInts::const_iterator it(types.begin()), et(types.end()); it != et; ++it) {
        html << "<option";
        if (qFirst) {
          html << " selected";
          qFirst = false;
        }
        html << " value='";

        switch ((Data::Type) *it) {
          case Data::INFLOW:
          case Data::FLOW: html << "flow'>Flow</option>\n"; break;
          case Data::GAUGE: html << "gauge'>Gauge</option>\n"; break;
          case Data::TEMPERATURE: html << "temp'>Temp</option>\n"; break;
          case Data::LASTTYPE: html << "GotMe'>GotMe</option>\n"; break;
        }
      } 
      html << "</select>\n";
    }
    html << "<input type='submit' name='o' value='Plot'>\n"
         << "<input type='submit' name='o' value='Table'>\n"
         << "<input type='submit' name='o' value='Database'>\n"
         << "<input type='hidden' name='h' value='" << hash << "'>\n"
         << "<noscript><input type='hidden' name='js' value='0'></noscript>\n"
         << "</form>\n";
  }

  html << "<ul>\n";

  maybe(html, location, "Location: ");
  maybe(html, info.riverName, "River Name: ");

  maybeLatLon(html, lat, lon);
  maybeLatLon(html, info.latitudeTakeout, info.longitudeTakeout);

  maybe(html, info.classString, "Class: ");

  for (GuideBook::const_iterator it(guides.begin()), et(guides.end()); it != et; ++it) {
    html << "<li>Guide: " << it->mkHTML() << "</li>\n";
  }

  maybe(html, info.length, "Length: ", " miles");
  gmaybe(html, info.elevation, ginfo.elevation, "Elevation: ", " feet");
  maybe(html, info.elevationLost, "Elevation Lost: ", " feet");
  if (!maybe(html, info.gradient, "Gradient: ", " feet/mile")) {
    if ((info.length > 0) && (info.elevationLost > 0)) {
      maybe(html, round(info.elevationLost / info.length), "Gradient: ", " feet/mile");
    } 
  }

  maybe(html, info.lowFlow, "Low flow: ", " CFS");
  maybe(html, info.optimalLowFlow, info.optimalHighFlow, "Optimal flow: ", " CFS");
  maybe(html, info.highFlow, "High flow: ", " CFS");
  maybe(html, info.state, "State: ");
  maybe(html, info.section, "Section: ");
  maybe(html, info.features, "Features: ");
  maybe(html, info.region, "Region: ");
  maybe(html, info.remoteness, "Remoteness: ");
  maybe(html, info.scenery, "Scenery: ");
  maybe(html, info.nature, "Nature: ");
  maybe(html, info.difficulties, "Difficulties: ");
  maybe(html, info.watershedType, "Type of watershed: ");
  maybe(html, info.notes, "Notes: ");
  maybe(html, info.drainage, "Drainage: ");
  maybe(html, info.drainageArea, "Drainage Area: ", " Square Miles");

  if (info.gaugeKey > 0) {
    // maybe(html, info.gaugeKey, "Gauge Key: ");
    // maybe(html, ginfo.name, "Gauge Name: ");
    maybeLatLon(html, ginfo.latitude, ginfo.longitude, "Gauge ");
    maybe(html, ginfo.date, "Gauge last updated: ");
    maybe(html, ginfo.description, "Gauge Description: ");
    maybe(html, ginfo.location, "Gauge Location: ");
    if (!ginfo.idUSGS.empty()) {
      html << "<li>Gauge USGS ID: <a href='" << html.usgsIdURL(ginfo.idUSGS)
           << "'>" << ginfo.idUSGS << "</a></li>\n";
    }
    if (!ginfo.idCBTT.empty()) {
      html << "<li>Gauge CBTT ID: <a href='" << html.cbttIdURL(ginfo.idCBTT)
           << "'>" << ginfo.idCBTT << "</a></li>\n";
    }
    maybe(html, ginfo.idUnit, "Gauge Hydrologic Unit ID: ");
    maybe(html, ginfo.state, "Gauge State: ");
    maybe(html, ginfo.elevation, "Gauge Elevation: ", " feet");
    maybe(html, ginfo.drainageArea, "Gauge Drainage Area: ", " Square Miles");
    // maybe(html, ginfo.minFlow, "Gauge Minimum Flow: ", " CFS");
    // maybe(html, ginfo.maxFlow, "Gauge Maximum Flow: ", " CFS");
    // maybe(html, ginfo.minGauge, "Gauge Minimum Height: ", " Feet");
    // maybe(html, ginfo.maxGauge, "Gauge Maximum Height: ", " Feet");
    // maybe(html, ginfo.minTemperature, "Gauge Minimum Temperature: ", " F");
    // maybe(html, ginfo.maxTemperature, "Gauge Maximum Temperature: ", " F");
    maybe(html, ginfo.calcFlow, "Gauge Flow Calculation: ");
    maybe(html, ginfo.calcGauge, "Gauge Gauge Height Calculation: ");
  }

  maybe(html, info.sortKey, "Sort Key: ");
 

  if (info.qNoShow) html << "<li>Do not display</li>\n";

  maybe(html, info.created, "Entry created on ");
  maybe(html, info.modified, "Entry modified on ");
  
  html << "</ul>\n"
       <<"</body>\n</html>\n";

  return HTTP(std::cout).htmlPage(86400, html);
}
