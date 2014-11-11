#include "Display.H"
#include "Gauges.H"
#include "Master.H"
#include "Levels.H"
#include "Tokens.H"
#include "CGI.H"
#include "HTML.H"
#include "HTTP.H"
#include "Convert.H"
#include <iostream>
#include <cmath>

namespace {
  int mkTable(const Gauges::tInfo& info, 
              const Master::tKey2KeyNameLocation& keyMap, 
              Master& master,
              const Levels& levels) {
    typedef std::map<int, size_t> tMap2Level;
    tMap2Level map2Level;

    for (Levels::size_type i(0), e(levels.size()); i < e; ++i) {
      map2Level.insert(std::make_pair(levels[i].gaugeKey, i));
    }

    HTML html;
    html << html.header()
         << html.myStyle()
         << "<title>Gauges</title>\n"
         << "</head>\n<body>\n";

    bool qHR(false);
    for (Gauges::tInfo::const_iterator it(info.begin()), et(info.end()); it != et; ++it) {
      const Gauges::Info& a(*it);
      if (qHR) html << "<hr>\n";
      qHR = true;
      html << "<div>\n"
           << "<h2>" << a.description << " " << a.location << "</h2>\n";

      { // Display level information, if known
        tMap2Level::const_iterator jt(map2Level.find(a.gaugeKey));
        if (jt != map2Level.end()) {
          levels[jt->second].table(html);
        }
      }

      html << "<ul>\n";
      Display::maybe(html, a.name, "Name: ");
      { // Get river segments that use this gauge
        typedef Master::tKey2KeyNameLocation::const_iterator const_iterator;
        std::pair<const_iterator, const_iterator> iters(keyMap.equal_range(a.gaugeKey));
        for (const_iterator jt(iters.first), jet(iters.second); jt != jet; ++jt) {
          const Master::KeyNameLocation& b(jt->second);
          html << "<li>Run: <a href='?o=i&amp;h=" << Master::mkHash(b.key)
               << "'>" << b.name
               << " " 
               << (b.location.empty() ? a.location : b.location) 
               << "</a></li>\n";
        }
      }
      Display::maybeLatLon(html, a.latitude, a.longitude, "Lat/Lon: ");
      Display::maybe(html, a.state, "State: ");
      Display::maybe(html, a.county, "County: ");
      Display::maybe(html, a.elevation, "Elevation: ", " Feet");
      Display::maybe(html, a.drainageArea, "Drainage Area: ", " Square Miles");
      Display::maybe(html, a.bankfullStage, "Bank Full Stage: ", " Feet");
      Display::maybe(html, a.date, "Last updated: ");
      Display::maybeCalc(html, a.calcFlow, master, "Flow Calculation: ");
      Display::maybeCalc(html, a.calcGauge, master, "Gauge Calculation: ");
      Display::maybe(html, a.gaugeKey, "Gauge Key: ");
      Display::maybeHRef(html, a.idUSGS, HTML::usgsIdURL(a.idUSGS), "USGS ID: ");
      Display::maybeHRef(html, a.idCBTT, HTML::cbttIdURL(a.idCBTT), "CBTT ID: ");
      Display::maybe(html, a.idUSBR, "USBR ID: ");
      Display::maybe(html, a.idUnit, "Hydrologic Unit: ");
      Display::maybeMinMax(html, a.minFlow, a.maxFlow, "Flow Limits: ", " CFS");
      Display::maybeMinMax(html, a.minGauge, a.maxGauge, "Gauge Limits: ", " Feet");
      Display::maybeMinMax(html, a.minTemperature, a.maxTemperature, "Temperature Limits: ", " F");
      html << "</ul>\n</div>\n";
    }

    html << "</body>\n</html>\n";
    return HTTP(std::cout).htmlPage(86400, html); 
  }
} // Anonymous

int
Display::gauges()
{
  const CGI cgi;
  Types::Keys keys(dehashKeys(cgi, "h"));
  Gauges gauges;
  const Gauges::tInfo info(gauges.getInfo(keys));

  Types::Keys gkeys;

  for (Gauges::tInfo::const_iterator it(info.begin()), et(info.end()); it != et; ++it) {
    gkeys.insert(it->gaugeKey);
  }

  Master master;
  const Master::tKey2KeyNameLocation keyMap(master.gaugeKey2KeyNameLocation(gkeys));

  return mkTable(info, keyMap, master, Levels(gkeys));
}
