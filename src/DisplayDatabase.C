#include "Display.H"
#include "DataGet.H"
#include "CGI.H"
#include "HTML.H"
#include "HTTP.H"
#include "Convert.H"
#include <iostream>
#include <cmath>

namespace {
  const CGI cgi;

  void mkTable(const Data::tAnObs& obs, HTML& html) {
    bool qFlow(false);
    bool qGauge(false);
    bool qTemp(false);
    bool qName(false);
    bool qURL(false);

    for (Data::tAnObs::const_iterator it(obs.begin()), et(obs.end()); 
         (it != et) && (!qFlow || !qGauge || !qTemp || !qName || !qURL); ++it) {
      qFlow |= !isnan(it->flow);
      qGauge |= !isnan(it->gauge);
      qTemp |= !isnan(it->temperature);
      qName |= !it->name.empty();
      qURL |= !it->url.empty();
    }

    std::string header("<tr><th>Date</th>");
    if (qFlow)  header += "<th>Flow<br>CFS</th>";
    if (qGauge) header += "<th>Gauge<br>Feet</th>";
    if (qTemp)  header += "<th>Temp<br>F</th>";
    if (qName)  header += "<th>Name</th>";
    if (qURL)   header += "<th>URL</th>";
    header += "</tr>";

    html << "<table>\n<thead>" << header << "</thead>\n<tbody>";

    for (Data::tAnObs::const_reverse_iterator it(obs.rbegin()), et(obs.rend()); it != et; ++it) {
      html << "<tr><td>" << Convert::toStr(it->time, "%m/%d/%Y %H:%M") << "</td>";
      if (qFlow) {
        html << "<td>";
        if (!isnan(it->flow)) {
          html << Convert::toComma(DataGet::roundY(it->flow, Data::FLOW));
        }
        html << "</td>";
      }
      if (qGauge) {
        html << "<td>";
        if (!isnan(it->gauge)) {
          html << Convert::toComma(DataGet::roundY(it->gauge, Data::GAUGE));
        }
        html << "</td>";
      }
      if (qTemp) {
        html << "<td>";
        if (!isnan(it->temperature)) {
          html << Convert::toComma(DataGet::roundY(it->temperature, Data::TEMPERATURE));
        }
        html << "</td>";
      }
      if (qName) {
        html << "<td>" << it->name << "</td>";
      }
      if (qURL) {
         html << "<td>";
         if (!it->url.empty()) {
           html << "<a href='" << it->url << "'>" << it->url << "</a>";
         }
        html << "</td>";
      }
      html << "</tr>\n";
    }
    html << "</tbody>\n<tfoot>" << header << "</tfoot>\n</table>\n";
  } // mkTable
} // anonymous

int
Display::database()
{
  const bool qScript(cgi.qJavaScript());
  const DataGet obs(false);
 
  if (!obs) {
    return HTTP(std::cout).errorPage(obs.errmsg());
  }

  HTML html;
  html << html.header()
       << html.myStyle()
       << "<title>" << obs.title() << "</title>\n";

  if (qScript) {
    html << html.myScript(); 
  }

  html << "</head>\n<body>\n"
       << dateForm(obs);

  if (qScript) {
    html << "<script>\n"
         << "tbl(" << obs.json() << ");\n"
         << "</script>\n"
         << "<noscript>\n";
  } else { // Spit out a table
    mkTable(obs.anObs(), html);
  }

  html << html.addOnStart()
       << html.addOnEnd();

  if (qScript) {
    html << "</noscript>";
  }
  
  html << "</body>\n</html>\n";
 
  return HTTP(std::cout).htmlPage(86400, html);
}
