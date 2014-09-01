#include "Display.H"
#include "DataGet.H"
#include "HTTP.H"
#include "HTML.H"
#include <iostream>

int
Display::data()
{
  const time_t now(time(0));
  DataGet obs;

  HTML html;
  obs.json(html);

  HTTP http(std::cout);
  http.content("text/javascript")
      .modified(now)
      .expires(now + 3600)
      .length(html)
      .end();

  http << html;
  return 0;
}
