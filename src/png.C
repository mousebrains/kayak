// CGI program to get data and generate a png image

#include "DataGet.H"
#include "MakePlot.H"
#include "HTTP.H"
#include "HTML.H"
#include "PNGCanvas.H"
#include <iostream>

int main (int argc,
          char **argv)
{
  const time_t now(time(0));
  DataGet obs; // Fetch information from CGI parameters

  if (!obs) {
    HTML html;
    html << html.header()
         << "<title>Data get Error</title>\n"
         << "</head>\n<body>\n<h2>"
         << obs.errmsg()
         << "</h2>\n</body>\n</html>\n";
    HTTP(std::cout).content().modified(now).expires(now)
                   .length(html).status(400, obs.errmsg()).end() << html;
    return 1;
  }

  PNGCanvas canvas(800, 600);
  MakePlot(canvas, obs);
  const std::string str(canvas.toStr());

  HTTP(std::cout).content("image/png").modified(now).expires(now + 3600)
                 .length(str.size()).end() << str;
  return 0;
}
