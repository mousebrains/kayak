#include "Display.H"
#include "DataGet.H"
#include "HTTP.H"
#include "HTML.H"
#include "CGI.H"
#include "Convert.H"
#include <iostream>
#include <cmath>

namespace {
  const CGI cgi;

  void mkTable(const DataGet& obs, HTML& html) {
    const std::string head("<tr><th>Date</th><th>" + obs.ylabel() + "<br>" + 
                           obs.units() + "</th></tr>\n");
    const std::string link(cgi.infoString(obs.title()) + "\n");
   
    html << html.addOnStart()
         << link 
         << "<table>\n"
         << "<thead>\n"
         << head
         << "</thead>\n<tbody>\n";

    bool qClass(false);

   for (DataGet::const_reverse_iterator it(obs.rbegin()), et(obs.rend()); it != et; ++it) {
      html << "<tr><td>" << Convert::toStr(it->first, "%m/%d/%Y %H:%M") << "</td><td";
      if ((it->second < obs.low()) && (obs.low() != 0)) {
        html << " class=\"lo\"";
        qClass = true;
      } else if ((it->second > obs.high()) && (obs.high() != 0)) {
        html << " class=\"hi\"";
        qClass = true;
      } else if ((obs.low() !=0) && (obs.high() != 0)) {
        html << " class=\"ok\"";
        qClass = true;
      }
      html << ">" << Convert::toComma(obs.roundY(it->second)) << "</td></tr>\n";
    }
  
    html << "<tfoot>\n"
         << head;

    if (qClass) {
      html << "<tr><th class=\"hi\" colspan=2>High</th></tr>\n"
           << "<tr><th class=\"ok\" colspan=2>Okay</th></tr>\n"
           << "<tr><th class=\"lo\" colspan=2>Low</th></tr>\n";
    }

    html << "</tfoot>\n"
         << "</table>\n"
         << link
         << Display::dateForm(obs, "frm2")
         << html.addOnEnd()
         << "</body>\n</html>\n";
  }
} // anonymous

int
Display::table()
{
  DataGet obs;

  if (!obs) {
    return HTTP(std::cout).errorPage(obs.errmsg());
  }

  const bool qScript(cgi.qJavaScript());

  HTML html;
  html << html.header()
       << html.myStyle()
       << "<title>" << obs.title() << "</title>\n";

  if (qScript) { // We may have java script available, so treat it as such
    // If noscript succeds we come back here but with js=0, which sets qScript to false
    html << "<noscript>\n"
         << "<meta http-equiv='refresh' content='0; url="
         << cgi.queryString(true)
         << "'>\n"
         << "</noscript>\n"
         << html.myScript();
  }

  html << "</head>\n<body>\n"
       << dateForm(obs);

  if (qScript) { // We may have java script available, so treat it as such
    html << "<script>\n"
         << "tbl(";
    obs.json(html);
    html << ");\n"
         << "</script>\n";
  } else { // We don't have java script available, so send an HTML table
    mkTable(obs, html);
  }

  html << "</body>\n</html>\n";

  return HTTP(std::cout).htmlPage(3600, html);
}
