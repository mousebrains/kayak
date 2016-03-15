#include "Display.H"
#include "Master.H"
#include "Gauges.H"
#include "Levels.H"
#include "Calc.H"
#include "CGI.H"
#include "HTML.H"
#include "HTTP.H"
#include "Convert.H"
#include "GuideBook.H"
#include "Tokens.H"
#include <iostream>
#include <cmath>

namespace {
  bool gmaybe(HTML& html, const double v0, const double v1,
              const std::string& prefix = "", const std::string& suffix = "") {
    return Display::maybe(html, isnan(v0) || (v0 == 0) ? v1 : v0, prefix, suffix);
  }
} // anonymous

int
Display::info()
{
  const CGI cgi;
  const Types::Keys keys(dehashKeys(cgi, "h"));
  Master master;
  const Master::tInfo info(master.getInfo(keys));
  Gauges gauges;
  Data data;
  Levels levels(keys); // current level information

  typedef std::map<int, Levels::size_type> tKey2Level; // Master key to level index
  tKey2Level key2level;

  for (Levels::size_type i(0), e(levels.size()); i < e; ++i) {
    key2level.insert(std::make_pair(levels[i].key, i));
  }

  typedef std::multimap<std::string, size_t> tSortKey2Key; // Sortkey to master key
  tSortKey2Key sortedKeys;
  { // Build sortkey to index
    int cnt(0);
    for (Master::tInfo::const_iterator it(info.begin()), et(info.end()); it != et; ++it, ++cnt) {
      sortedKeys.insert(std::make_pair(it->sortKey, cnt));
    }
  }

  HTML html;
  html << html.header()
       << html.baseURL()
       << html.myStyle()
       << "<title>Run Information</title>\n"
       << "</head>\n<body>\n";

  bool qHR(false);

  for (tSortKey2Key::const_iterator it(sortedKeys.begin()), et(sortedKeys.end()); it != et; ++it) {
    const Master::Info& a(info[it->second]);
    const Gauges::Info ginfo(a.gaugeKey > 0 ? gauges.getInfo(a.gaugeKey) : Gauges::Info());
    const std::string location(a.location.empty() ? ginfo.location : a.location);
    const std::string title(a.displayName + (location.empty() ? "" : (" " + location)));
    const GuideBook guides(a.key);

    if (qHR) html << "<hr>\n";
    qHR = true;

    html << "<h1>" << title << "</h1>\n";
    
    { // Display current information
      tKey2Level::const_iterator jt(key2level.find(a.key));
      if (jt != key2level.end()) levels[jt->second].table(html);
    }

    html << "<ul>\n";

    maybe(html, location, "Location: ");
    maybe(html, a.riverName, "River Name: ");

    maybeLatLon(html, 
                a.latitudePutin, a.longitudePutin,
                a.latitudeTakeout, a.longitudeTakeout,
                "Lat/Lon: ");

    maybe(html, a.classString, "Class: ");

    for (GuideBook::const_iterator jt(guides.begin()), jet(guides.end()); jt != jet; ++jt) {
      html << "<li>Guide: " << jt->mkHTML() << "</li>\n";
    }

    maybe(html, a.length, "Length: ", " miles");
    gmaybe(html, round(a.elevation), round(ginfo.elevation), "Elevation: ", " feet");
    maybe(html, a.elevationLost, "Elevation Lost: ", " feet");
    if (!maybe(html, a.gradient, "Gradient: ", " feet/mile")) {
      if ((a.length > 0) && (a.elevationLost > 0)) {
        maybe(html, round(a.elevationLost / a.length), "Gradient: ", " feet/mile");
      } 
    }

    maybe(html, a.lowFlow, "Low flow: ", " CFS");
    maybe(html, a.optimalLowFlow, a.optimalHighFlow, "Optimal flow: ", " CFS");
    maybe(html, a.highFlow, "High flow: ", " CFS");
    maybe(html, a.state, "State: ");
    maybe(html, a.section, "Section: ");
    maybe(html, a.features, "Features: ");
    maybe(html, a.region, "Region: ");
    maybe(html, a.remoteness, "Remoteness: ");
    maybe(html, a.scenery, "Scenery: ");
    maybe(html, a.nature, "Nature: ");
    maybe(html, a.difficulties, "Difficulties: ");
    maybe(html, a.watershedType, "Type of watershed: ");
    maybe(html, a.notes, "Notes: ");
    maybe(html, a.drainage, "Drainage: ");
    maybe(html, a.drainageArea, "Drainage Area: ", " Square Miles");

    if (a.gaugeKey > 0) {
      maybeLatLon(html, ginfo.latitude, ginfo.longitude, "Gauge Lat/Lon: ");
      maybe(html, ginfo.date, "Gauge last updated: ");
      maybe(html, ginfo.description, "Gauge Description: ");
      maybe(html, ginfo.location, "Gauge Location: ");
      maybeHRef(html, ginfo.idUSGS, html.usgsIdURL(ginfo.idUSGS), "Gauge USGS ID: ");
      maybeHRef(html, ginfo.idCBTT, html.cbttIdURL(ginfo.idCBTT), "Gauge CBTT ID: ");
      maybe(html, ginfo.idUnit, "Gauge Hydrologic Unit ID: ");
      maybe(html, ginfo.state, "Gauge State: ");
      maybe(html, round(ginfo.elevation), "Gauge Elevation: ", " feet");
      maybe(html, ginfo.drainageArea, "Gauge Drainage Area: ", " Square Miles");
      maybeCalc(html, ginfo.calcFlow, master, "Gauge Flow Calculation: ");
      maybeCalc(html, ginfo.calcGauge, master, "Gauge Height Calculation: ");
    }

    maybe(html, a.sortKey, "Sort Key: ");
 

    if (a.qNoShow) html << "<li>Do not display</li>\n";

    maybe(html, a.created, "Entry created on ");
    maybe(html, a.modified, "Entry modified on ");
  
    html << "</ul>\n</div>\n";
  }
  
  html <<"</body>\n</html>\n";

  return HTTP(std::cout).htmlPage(86400, html);
}
