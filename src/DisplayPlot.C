#include "Display.H"
#include "DataGet.H"
#include "HTTP.H"
#include "HTML.H"
#include "CGI.H"
#include <iostream>
#include <cmath>

namespace {
  const CGI cgi;
} // anonymous

int
Display::plot()
{
  DataGet obs;

  if (!obs) {
    return HTTP(std::cout).errorPage(obs.errmsg());
  }

  const bool qScript(cgi.qJavaScript()); // Is javascript support enabled?
  const bool qCanvas(!cgi.isSet("nc") || (cgi.get("nc") != "1")); // Canvas may be supported

  HTML html;
  html << html.header()
       << html.myStyle()
       << "<title>" << obs.title() << "</title>\n";
 
  if (qScript) {
    html << html.myScript();
  }

  html << "</head>\n<body>\n"
       << dateForm(obs);

  if (qScript && qCanvas) { // scripting in a browser that may support a canvas
    html << "<script>\n"
         << "plt(";
    obs.json(html);
    html << ");\n"
         << "</script>\n"
         << "<noscript>\n";
  }
  
  html << html.addOnStart()
       << "<img src='/kayaking2/cgi/png"
       << cgi.queryString(false)
       << "' alt='Plot goes here' width='800' height='600'>\n"
       << html.addOnEnd();

  if (qScript && qCanvas) {
    html << "</noscript>\n";
  }

  html << "</body>\n</html>\n";

  return HTTP(std::cout).htmlPage(3600, html);
}
