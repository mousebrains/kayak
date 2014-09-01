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

  std::string format(const std::string& val, const std::string& hash, 
                     const std::string& type, const Levels::Level level) {
    const std::string color(level == Levels::LOW  ? " class='lo'" :
                           (level == Levels::OKAY ? " class='ok'" :
                           (level == Levels::HIGH ? " class='hi'" : "")));
    const bool qLink(!hash.empty() && !type.empty());
    std::string str("<td" + color + ">");
    if (qLink) str += "<a href='?o=p&amp;h=" + hash + "&amp;t=" + type + "'>";
    str += val;
    if (qLink) str += "</a>";
    str += "</td>";
    return str;
  }

  std::string formatFlow(const double val, const std::string& hash, const Levels::Level level) {
    return format(Convert::toComma(round(val)), hash, "f", level);
  }

  std::string formatGauge(const double val, const std::string& hash, const Levels::Level level) {
    return format(Convert::toComma(round(val * 10) / 10), hash, "g", level);
  }

  std::string formatTemp(const double val, const std::string& hash) {
    return format(Convert::toStr(round(val)), hash, "t", Levels::UNKNOWN);
  }

  std::string formatTime(const time_t t0, const time_t t1, const time_t t2) {
    const time_t t((t0 > t1) && (t0 > t2) ? t0 : (t1 > t2) ? t1 : t2);
    if (t == 0) {
      return "<td></td>";
    }
    std::string str("<td");
    if (t < tOld) str += " class=old";
    str += ">" + Convert::toStr(t, "%m/%d %H:%M") + "</td>";
    return str;
  }

  int mkTable(const Levels& levels) {
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
         << " with any comments about this page.</h3>\n"
         << "<h4><a href='?o=m'>For other states</a></h4>\n"
         << "<table>\n"
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
           << "<th><a href='?o=i&h=" << hash << "'>" << it->name;

      if (!it->location.empty()) html << "@" << it->location;

      html << "</a></th>";
     
      html << formatTime(qf ? it->flowTime : 0, qg ? it->gaugeTime : 0, 
                         qt ? it->temperatureTime: 0); 
  
      if (qf) {
        html << formatFlow(it->flow, hash, it->level);
      } else if (levels.qFlow()) {
        html << "<td></td>";
      }
      if (qg) {
        html << formatGauge(it->gauge, hash, it->level);
      } else if (levels.qGauge()) {
        html << "<td></td>";
      }
      if (qt) {
        html << formatTemp(it->temperature, hash);
      } else if (levels.qTemperature()) {
        html << "<td></td>";
      }

      if (levels.qClass()) html << "<td>" << it->grade << "</td>";

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
         << "</dl>\n"
         << "</body>\n"
         << "</html>\n";
    
    return HTTP(std::cout).htmlPage(86400, html);
  }

  int process(const Levels& levels) {
    // if (cgi.qJavaScript()) {
      // levels.json(std::cout) << std::endl;
    // } else {
    return mkTable(levels);
    // }
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
