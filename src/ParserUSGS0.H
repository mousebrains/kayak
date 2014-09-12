#ifndef INC_ParserUSGS0_H_
#define INC_ParserUSGS0_H_

#include "XMLParser.H"
#include "Gauges.H"
#include "Data.H"
#include <string>
#include <vector>

class ParserUSGS0 {
private:
  bool mqVerbose;
  std::string mURL;
  Data mData;
  Gauges mGauges;

  void collection(const XMLParser::Node& root);
  void obsMember(const XMLParser::Node& root, Gauges::Info& gauge);
  void omResult(const XMLParser::Node& root, Gauges::Info& gauge, const Data::Type type);
  void featureOfInterest(const XMLParser::Node& root, Gauges::Info& gauge);
  void point(const XMLParser::Node& root, Gauges::Info& gauge, const Data::Type type);
public:
  ParserUSGS0(const std::string& url, const std::string& str, const bool qVerbose = false);
}; // ParserUSGS0

#endif // INC_ParserUSGS0_H_
