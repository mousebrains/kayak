#include "HTML.H"
#include "Paths.H"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

HTML::HTML()
{
}

HTML::~HTML()
{
}

std::string
HTML::header() 
{
  std::ostringstream oss;

  oss << "<!DOCTYPE html >\n"
      << "<html lang='en'>\n"
      << "<head>\n"
      << "<meta charset='utf-8'>\n"
      ;

  return oss.str();
}

std::string 
HTML::myStyle()
{
  return "<link rel='stylesheet' type='text/css' href='css/kayak.css'>\n";
}

std::string 
HTML::myScript()
{
  return "<script src='scripts/kayak.js'></script>\n";
}

std::string
HTML::baseURL()
{
  return "<base href='/kayaking2'>\n";
}

std::string 
HTML::addOnStart()
{
  return "<div id='addon'>\n";
}

std::string 
HTML::addOnEnd()
{
  return "</div>\n";
}

std::string
HTML::weatherURL(const double lat,
                 const double lon)
{
  if (isnan(lat) || isnan(lon) || (fabs(lat) > 90) || (fabs(lon) > 180)) return std::string();
 
  std::ostringstream oss;

  oss << "http://forecast.weather.gov/MapClick.php?lat=" 
      << lat << "&amp;lon=" << lon;

  return oss.str();
}

std::string
HTML::weatherForm(const double lat,
                  const double lon)
{
  if (fabs(lat) > 90 || fabs(lon) > 180) return std::string();

  std::ostringstream oss;

  oss << "<form action='http://forecast.weather.gov/MapClick.php'>\n"
      << "<input type='submit' value='Weather'>\n"
      << "<input type='hidden' name='lat' value='" << lat << "'>\n"
      << "<input type='hidden' name='lon' value='" << lon << "'>\n"
      << "</form>\n" 
      ;

  return oss.str();
}


std::string
HTML::mapURL(const double lat,
             const double lon)
{
  if (fabs(lat) > 90 || fabs(lon) > 180) return std::string();

  std::ostringstream oss;
  oss << "http://maps.google.com/?q=" 
      << lat << "," << lon
      ;
  return oss.str();
}

std::string
HTML::mapURL(const double slat,
             const double slon,
             const double elat,
             const double elon)
{
  if (fabs(slat) > 90 || fabs(slon) > 180) return mapURL(elat, elon);
  if (fabs(elat) > 90 || fabs(elon) > 180) return mapURL(slat, slon);

  std::ostringstream oss;
  oss << "http://maps.google.com/?saddr=" 
      << slat << "," << slon
      << "&amp;daddr="
      << elat << "," << elon
      ;
  return oss.str();
}

std::string
HTML::mapForm(const double lat,
              const double lon)
{
  if (fabs(lat) > 90 || fabs(lon) > 180) return std::string();
  std::ostringstream oss;
  oss << "<form action='http://maps.google.com'>\n"
      << "<input type='submit' value='Map'>\n"
      << "<input type='hidden' name='q' value='"
      << lat << "," << lon << "'>\n"
      << "</form>\n"
      ;

  return oss.str();
}

std::string
HTML::usgsIdURL(const std::string& id)
{
  return id.empty() ? std::string() :
         "http://waterdata.usgs.gov/nwis/inventory?site_no=" + id;
}

std::string 
HTML::cbttIdURL(const std::string& id)
{
  return id.empty() ? std::string() :
         "http://www.nwrfc.noaa.gov/river/station/flowplot/flowplot.cgi?lid=" + id;
}

std::string 
HTML::awIdURL(const std::string& id)
{
  return id.empty() ? std::string() :
         "https://www.americanwhitewater.org/content/River/detail/id/" + id;
}

std::ostream& 
operator << (std::ostream& os, 
             const HTML& h)
{
  os << h.str();
  return os;
}
