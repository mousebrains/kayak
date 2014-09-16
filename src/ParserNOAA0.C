// Parse a NOAA XML site data file, a gauge specific rss file

#include "ParserNOAA0.H"
#include "XMLParser.H"
#include "Gauges.H"
#include "GaugeTranslate.H"
#include "Convert.H"
#include "String.H"
#include <iostream>
#include <sstream>
#include <cmath>
#include <cstring>

namespace {
  std::string capitalize(const std::string& str) {
    if (str.empty()) return str; 
    std::string a(String::tolower(str));
    if ((a[0] >= 'a') && (a[0] <= 'z')) a[0] = a[0] - 'a' + 'A';
    return a;
  }
} // anonymous

ParserNOAA0::ParserNOAA0(const std::string& url,
                         const std::string& str,
                         const bool qVerbose)
  : mURL(url)
{
  const XMLParser xml(str);
  for (XMLParser::Node root(xml.first()); root; ++root) { // HydroMetData
    if (strcmp(root.name(), "HydroMetData")) continue;
    for (XMLParser::Node site(root.firstChild()); site; ++site) { // SiteData 
      if (strcmp(site.name(), "SiteData")) continue;
      collection(site);
    }
  }
}

void
ParserNOAA0::collection(const XMLParser::Node& site)
{
  Gauges::Info info;
  info.idCBTT = site.findAttr("id");

  std::string description;

  for (XMLParser::Node node(site.firstChild()); node; ++node) { // Walk through site information
    const std::string name(node.name());
    const std::string val(node.value());
    if (name == "description") { description = val;
    } else if (name == "county") { info.county = capitalize(val);
    } else if (name == "state") { info.state = capitalize(val);
    } else if (name == "elevation") { info.elevation = Convert::strTo<double>(val);
    } else if (name == "latitude") { info.latitude = Convert::strTo<double>(val);
    } else if (name == "longitude") { info.longitude = Convert::strTo<double>(val);
    } else if (name == "bankfullStage") { info.bankfullStage = Convert::strTo<double>(val);
    } else if (name == "floodStage") { info.floodStage = Convert::strTo<double>(val);
    } else if (name == "observedData") {
      observedData(node, info.idCBTT);
    } else if (name == "forecastData") {
      ; // Do nothing
    } else {
      std::cerr << "Unrecognized name(" << name << ") val(" << val << ")" << std::endl;
    }
  }

  info.gaugeKey = mGauges.name2key(info.idCBTT);
  if (info.gaugeKey <= 0) { // Didn't find
    info.gaugeKey = mGauges.idCBTT2gaugeKey(info.idCBTT);
  }

  if (info.gaugeKey > 0) { // Found a gauge key
    GaugeTranslate translate;
    const GaugeTranslate::Info a(translate(description));
    info.description = a.description;
    info.location = a.location;
    mGauges.putInfo(info);
  }
}

void
ParserNOAA0::observedData(const XMLParser::Node& root,
                          const std::string& name)
{
  for (XMLParser::Node node(root.firstChild()); node; ++node) {
    if (!strcmp(node.name(), "observedValue")) observedValue(node, name);
  }
}

void
ParserNOAA0::observedValue(const XMLParser::Node& root,
                          const std::string& name)
{
  double flow(NAN), gauge(NAN);
  time_t t(0);

  for (XMLParser::Node node(root.firstChild()); node; ++node) {
    const std::string name(node.name());
    const std::string val(node.value());
    if (name == "dataDateTime") {
      t = Convert::toTime(val);
    } else if (name == "stage") {
      gauge = Convert::strTo<double>(val);
    } else if (name == "discharge") {
      flow = Convert::strTo<double>(val);
    } else {
      std::cerr << "Unrecognized node name(" << name << ") value(" << val << ")" << std::endl;
    } 
  }

  if (t > 0) {
    if (~isnan(flow)) mData.add(name, t, flow, Data::FLOW, mURL);
    if (~isnan(gauge)) mData.add(name, t, gauge, Data::GAUGE, mURL);
  }
}
