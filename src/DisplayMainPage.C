#include "Display.H"
#include "Levels.H"
#include "ReadFile.H"
#include "Paths.H"
#include "HTML.H"
#include "HTTP.H"
#include <iostream>
#include <cmath>

int
Display::mainPage()
{
  const Levels::tStates states(Levels().states());
  HTML html;
  html << ReadFile(Paths::mainPageHead());

  for (Levels::tStates::const_iterator it(states.begin()), et(states.end()); it != et; ++it) {
    html << "<tr><th>" 
         << "<a href='?o=l&s=" << *it << "'>" << *it << "</a></th></tr>\n";
  }

  html << ReadFile(Paths::mainPageTail());

  return HTTP(std::cout).htmlPage(86400, html);
}
