// Parse a USGS XML Rest file

#include "ParserUSGS0.H"
#include "XMLParser.H"
#include "Gauges.H"
#include "GaugeTranslate.H"
#include "Convert.H"
#include <iostream>
#include <sstream>

namespace {
  std::string mkName(const std::string& str) {
    const std::string::size_type n(str.rfind('.'));
    return n == str.npos ? str : str.substr(n+1);
  }

  void mkLatLon(const XMLParser::Node& root, Gauges::Info& gauge) {
    const XMLParser::Node node(root.firstChild().findNode("gml:pos"));
    if (node) {
      std::istringstream iss(node.value());
      double lat, lon;
      if ((iss >> lat >> lon)) {
        gauge.latitude = lat;
        gauge.longitude = lon;
      }
    }
  }

  Data::Type getType(const XMLParser::Node& root) {
    const std::string title(root.findNode("om:observedProperty").findAttr("xlink:title"));
    if (title.find("Discharge") != title.npos) return Data::FLOW;
    if (title.find("Gage") != title.npos) return Data::GAUGE;
    if (title.find("Temperature") != title.npos) return Data::TEMPERATURE;
    std::cerr << "Unrecognized type, '" << title << "'" << std::endl;
    return Data::LASTTYPE;
  }
} // anonymous

ParserUSGS0::ParserUSGS0(const std::string& url,
                         const std::string& str,
                         const bool qVerbose)
  : mqVerbose(qVerbose)
  , mURL(url)
  , mData(qVerbose)
{
  const XMLParser xml(str);
  const char *colName("wml2:Collection");

  for (XMLParser::Node root(xml.first().findNode(colName)); root;) {
    collection(root);

    if (!root.parent()) break; // No parent to try and traverse across

    XMLParser::Node node(root.next(colName));

    if (node) { // Found another colName at this level
      root = node;
    } else { // Look at parent level
      root = root.parent();
      if (root && root.parent()) { // There is a parent with a parent level to look in
        root = (++root).findNode(colName); // Find the next collection
      }
    }
  }
}

void
ParserUSGS0::collection(const XMLParser::Node& root)
{
  Gauges::Info gauge;
  gauge.name = mkName(root.findAttr("gml:id"));
  gauge.idUSGS = gauge.name;

  if (gauge.name.empty()) return;

  if (mqVerbose) {
    std::cout << "processCollection NAME(" << gauge.name << ")" << std::endl;
  }

  const char *key("wml2:observationMember");
  for (XMLParser::Node node(root.findNode(key)); node; node = node.next(key)) {
    obsMember(node.firstChild(), gauge);
  }

  gauge.gaugeKey = mGauges.name2key(gauge.name);
  if (gauge.gaugeKey <= 0) { // Look up by idUSGS
    gauge.gaugeKey = mGauges.idUSGS2gaugeKey(gauge.name);
  }

  if (gauge.gaugeKey > 0) {
    const GaugeTranslate::Info xlat(GaugeTranslate()(gauge.description));
    gauge.description = xlat.description;
    gauge.location = xlat.location;
  
    mGauges.putInfo(gauge);
  }
}

void 
ParserUSGS0::obsMember(const XMLParser::Node& root,
                       Gauges::Info& gauge) 
{
  const Data::Type type(getType(root));
  if (type == Data::LASTTYPE) return;

  if (mqVerbose) {
    std::cout << "Type(" << type << ")" << std::endl;
  }

  featureOfInterest(root, gauge);
  omResult(root, gauge, type);
}

void 
ParserUSGS0::omResult(const XMLParser::Node& root,
                      Gauges::Info& gauge,
                      const Data::Type type) 
{
  const char *key("om:result");
  for (XMLParser::Node node(root.findNode(key)); node; node = node.next(key)) {
    point(node.firstChild(), gauge, type);
  }
}

void 
ParserUSGS0::featureOfInterest(const XMLParser::Node& root,
                               Gauges::Info& gauge) 
{
  XMLParser::Node node(root.findNode("om:featureOfInterest"));
  gauge.description = node.findAttr("xlink:title");
  mkLatLon(node, gauge);

  if (mqVerbose) {
    std::cout << "title(" << gauge.description << ")" << std::endl;
    printf("Lat %.15g Lon %.15g\n", gauge.latitude, gauge.longitude);
  }
}

void 
ParserUSGS0::point(const XMLParser::Node& root,
                   Gauges::Info& gauge,
                   const Data::Type type) 
{
  const char *key("wml2:point");
  for (XMLParser::Node node(root.findNode(key)); node; node = node.next(key)) {
    const XMLParser::Node child(node.firstChild());
    const std::string time(child.findValue("wml2:time"));
    const std::string value(child.findValue("wml2:value"));
    if (!time.empty() && !value.empty()) {
      const time_t t(Convert::toTime(time));
      const double obs(Convert::strTo<double>(value));
      mData.add(gauge.name, t, type == Data::TEMPERATURE ? (obs * 1.8 + 32) : obs, type, mURL);
      gauge.date = gauge.date > t ? gauge.date : t;
      if (mqVerbose) {
        std::cout << gauge.name << " " << type << " TIME (" << time << ") VALUE(" << value << ")" << std::endl;
        std::cout << gauge.name << " " << type << " TIME (" << t << ") VALUE(" << obs << ")" << std::endl;
      }
    }
  }
}
