#include "HTML.H"
#include "HTTP.H"
#include "CGI.H"
#include <iostream>
#include <unistd.h>

int 
main (int argc,
      char **argv)
{
  CGI cgi;

  HTML html;

  html << html.header()
       << "<title>Print Environment</title>"
       << "</head>\n<body>\n";

  html << "<H1>Login</H1>\n"
       << getlogin() << "\n";

  html << "<H1>Args</H1>\n"
       << "<OL>\n";
  for (int i = 0; i < argc; ++i) {
    html << "<LI>" << argv[i] << "\n";
  }
  html << "</OL>\n";

  {
    extern char **environ;

    html << "<H1>Environ</H1>\n"
         << "<OL>\n";
    for (int i = 0; environ[i]; ++i) {
      html << "<LI>" << environ[i] << "\n";
    }
    html << "</OL>\n";
  }
 
  html << "<H1>CGI Path</H1>\n"
       << "<OL>\n";
  for (CGI::size_type i = 0; i < cgi.nPath(); ++i) {
    html << "<LI>" << cgi[i] << "\n";
  }
  html << "</OL>\n";

  html << "<H1>CGI Vars</H1>\n"
       << "<UL>\n";
  for (CGI::const_iterator it = cgi.begin(); it != cgi.end(); ++it) {
    html << "<LI>" << it->first << " -- " << it->second << "\n";
  }
  html << "</UL>\n";
  html << "</body>\n</html>\n";

  HTTP(std::cout).htmlPage(0, html);

  return 0;
}
