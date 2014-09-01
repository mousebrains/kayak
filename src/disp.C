// Main display engine for all pages
// The logic is dynamic information is gotten and spit out here
// so the client does not have to make a second request for dynamic
// information. Hopefully, when combined with caching of the 
// static information, this will reduce a fetch cycle and the associated
// latency.

#include "Display.H"
#include "HTTP.H"
#include "CGI.H"
#include <iostream>

int 
main (int argc,
      char **argv)
{
  const std::string key("o");
  const CGI cgi;

  const std::string page(cgi.get(key, std::string()));

  if (page.empty() || (page == "m")) { // Main page
    return Display::mainPage();
  }
  if (page == "l") { // levels page
    return Display::levels();
  }
  if (page == "d") { // data json
    return Display::data();
  }
  if ((page == "t") || (page == "Table")) { // Table
    return Display::table();
  }
  if ((page == "p") || (page == "Plot")) { // Plot
    return Display::plot();
  }
  if ((page == "i") || (page == "Info")) { // Master table info
    return Display::info();
  }
  if (page == "db" || (page == "Database")) { // Database 
    return Display::database();
  }
 
  return HTTP(std::cout).errorPage("Unknown page '" + page + "' to display");
}
