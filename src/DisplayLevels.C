#include "Display.H"
#include "Levels.H"
#include "Master.H"
#include "Tokens.H"
#include "CGI.H"
#include "HTML.H"
#include "HTTP.H"
#include "Convert.H"
#include <iostream>
#include <cmath>

namespace {
  const CGI cgi;
  const time_t tOld(time(0) - 2 * 86400);

  void formatVal(std::ostream& os, const bool q, const double val, const std::string& hash, 
                 const Levels::Level level, const double delta, 
                 const std::string& type, const double roundTo) {
    if (!q) {
      os << "<td></td>";
      return;
    }

    const std::string color(level == Levels::LOW  ? " class='lo'" :
                           (level == Levels::OKAY ? " class='ok'" :
                           (level == Levels::HIGH ? " class='hi'" : "")));
    const bool qLink(!hash.empty() && !type.empty());

    os << "<td" << color << ">";

    if (qLink) os << "<a href='?o=p&amp;h=" << hash << "&amp;t=" << type << "'>";
    { // Arrow
      const int cnt(fmin(11,round(fabs(delta))));
      if (cnt) {
        os << "<span class='lev" << cnt << "'>&"
           << (delta < 0 ? "d" : "u") << "arr;</span>";
      }
    } // Arrow
    os << Convert::toComma(round(val * roundTo) / roundTo);
    if (qLink) os << "</a>";
    os << "</td>";
  }

  void formatTime(std::ostream& os, const time_t t0, const time_t t1, const time_t t2) {
    const time_t t((t0 > t1) && (t0 > t2) ? t0 : (t1 > t2) ? t1 : t2);
    if (t == 0) {
      os << "<td></td>";
      return;
    }
    os << "<td";
    if (t < tOld) os << " class=old";
    os << ">" << Convert::toStr(t, "%m/%d %H:%M") << "</td>";
  }

  void mkTable(const Levels& levels, HTML& html) {
    html << "<table>\n"
         << "<thead>\n";

    std::string hdr("<tr><th>Name</th><th>Date</th>");

    if (levels.qFlow()) hdr += "<th>Flow</br>CFS</th>";
    if (levels.qGauge()) hdr += "<th>Height</br>Feet</th>";
    if (levels.qTemperature()) hdr += "<th>Temp</br>F</th>";
    if (levels.qClass()) hdr += "<th>Class</th>";
    hdr += "</tr>\n";

    html << hdr << "<tbody>\n";
 
    for (Levels::const_iterator it(levels.begin()), et(levels.end()); it != et; ++it) {
      const bool qf(levels.qFlow() && !isnan(it->flow));
      const bool qg(levels.qGauge() && !isnan(it->gauge));
      const bool qt(levels.qTemperature() && !isnan(it->temperature));
      const std::string& hash(Master::mkHash(it->key));
 
      html << "<tr>"
           << "<th><a href='?o=i&h=" << hash << "'>" 
           << it->name;

      if (!it->location.empty()) html << "@" << it->location;
      html << "</a>";
      if (it->qCalc) html << "<span class='red'>(est)</span>";
      html << "</th>";
     
      formatTime(html, 
                 qf ? it->flowTime : 0, 
                 qg ? it->gaugeTime : 0, 
                 qt ? it->temperatureTime: 0); 
 
      formatVal(html, qf, it->flow, hash, it->level, it->flowDelta, "f", 1); 
      formatVal(html, qg, it->gauge, hash, it->level, it->gaugeDelta, "g", 10); 
      formatVal(html, qt, it->temperature, hash, Levels::UNKNOWN, it->temperatureDelta, "t", 1); 

      if (levels.qClass()) html << "<th>" << it->grade << "</th>";

      html << "\n";
    }
    html << "</tbody>\n<tfoot>\n" 
         << hdr;
        
    if (levels.qFlow() || levels.qGauge()) { // Low/Okay/High
      const int n(2 + levels.qFlow() + levels.qGauge() + levels.qTemperature() + levels.qClass());
      html << "<tr><th class='lo' colspan=" << n << ">Low</th></tr>\n" 
           << "<tr><th class='ok' colspan=" << n << ">Okay</th></tr>\n" 
           << "<tr><th class='hi' colspan=" << n << ">High</th></tr>\n";
    }
 
    html << "</foot>\n"
         << "</table>\n"
         << "<h3>Generated " << Convert::toStr(time(0), "%c") << "</h3>\n"
         << "<dl>\n"
         << "<dd><span class='lev1'>&uarr;&darr;</span> Changing between 0.5% and 1.5% per hour</dd>\n"
         << "<dd><span class='lev2'>&uarr;&darr;</span> Changing between 1.5% and 2.5% per hour</dd>\n"
         << "<dd><span class='lev3'>&uarr;&darr;</span> Changing between 2.5% and 3.5% per hour</dd>\n"
         << "<dd><span class='lev4'>&uarr;&darr;</span> Changing between 3.5% and 4.5% per hour</dd>\n"
         << "<dd><span class='lev5'>&uarr;&darr;</span> Changing between 4.5% and 5.5% per hour</dd>\n"
         << "<dd><span class='lev6'>&uarr;&darr;</span> Changing between 5.5% and 6.5% per hour</dd>\n"
         << "<dd><span class='lev7'>&uarr;&darr;</span> Changing between 6.5% and 7.5% per hour</dd>\n"
         << "<dd><span class='lev8'>&uarr;&darr;</span> Changing between 7.5% and 8.5% per hour</dd>\n"
         << "<dd><span class='lev9'>&uarr;&darr;</span> Changing between 8.5% and 9.5% per hour</dd>\n"
         << "<dd><span class='lev10'>&uarr;&darr;</span> Changing between 9.5% and 10.5% per hour</dd>\n"
         << "<dd><span class='lev11'>&uarr;&darr;</span> Changing more than  10.5% per hour</dd>\n"
         << "</dl>\n";
  }

  int process(const Levels& levels) {
    const std::string title(levels.state().empty() ? "WKCC Water Levels" : levels.state());

    HTML html;
    html << html.header()
         << "<title>" << title << "</title>\n"
         << html.myStyle()
         << html.myScript()
         << "</head>\n<body>\n"
         << "<h1>" << title << "</h1>\n"
         << "<h3>Courtesy of <a href='http://www.wkcc.org'>WKCC</a>, "
         << "please contact "
         << "<a href='mailto:pat.kayak@gmail.com'>Pat Welch</a>"
         << " with any comments about this page.\n"
         << "<a href='?o=m'>For other states</a></h3>\n";

    if (!cgi.qJavaScript()) {
      mkTable(levels, html);
    } else {
      html << "<script>\n"
           << "lev(";
      levels.json(html);
      html << ");\n"
           << "</script>\n";
    }

    html << "</body>\n"
         << "</html>\n";
    
    return HTTP(std::cout).htmlPage(86400, html);
  }
}

int
Display::levels()
{
  const std::string state(cgi.get("s", std::string()));
  if (!state.empty()) {
    const Levels levels(state);
    return process(Levels(state));
  }
  const Tokens tokens(cgi.get("h", std::string()));
  MyDB::tInts keys;
  for (Tokens::const_iterator it(tokens.begin()), et(tokens.end()); it != et; ++it) {
    keys.push_back(Convert::strTo<int>(*it));
  }
  return process(Levels(keys));
}
