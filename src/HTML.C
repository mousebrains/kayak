#include "HTML.H"
#include "Convert.H"
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
      << "<base href='" << Paths::webBase() << "'>\n"
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

  return "http://forecast.weather.gov/MapClick.php?lat=" +
         Convert::toStr(lat) + "&amp;lon=" + Convert::toStr(lon);
}

std::string
HTML::weatherForm(const double lat,
                  const double lon)
{
  if (fabs(lat) > 90 || fabs(lon) > 180) return std::string();
  std::string str("<form action='http://forecast.weather.gov/MapClick.php'>\n");

  str += "<input type='submit' value='Weather'>\n";
  str += "<input type='hidden' name='lat' value='" + Convert::toStr(lat) + "'>\n";
  str += "<input type='hidden' name='lon' value='" + Convert::toStr(lon) + "'>\n";
  str += "</form>\n";

  return str;
}


std::string
HTML::mapURL(const double lat,
             const double lon)
{
  if (fabs(lat) > 90 || fabs(lon) > 180) return std::string();

  return "http://maps.google.com/?q=" + Convert::toStr(lat) + "," + Convert::toStr(lon);
}

std::string
HTML::mapForm(const double lat,
              const double lon)
{
  if (fabs(lat) > 90 || fabs(lon) > 180) return std::string();
  std::string str("<form action='http://maps.google.com'>\n");

  str += "<input type='submit' value='Map'>\n";
  str += "<input type='hidden' name='q' value='";
  str += Convert::toStr(lat) + "," + Convert::toStr(lon) + "'>\n";
  str += "</form>\n";

  return str;
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

std::ostream& 
operator << (std::ostream& os, 
             const HTML& h)
{
  os << h.str();
  return os;
}
