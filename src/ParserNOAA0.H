#ifndef INC_ParserNOAA0_H_
#define INC_ParserNOAA0_H_

#include "XMLParser.H"
#include "Gauges.H"
#include "Data.H"
#include <string>
#include <vector>

class ParserNOAA0 {
private:
  Data mData;
  Gauges mGauges;

  std::string mURL;

  void collection(const XMLParser::Node& root);
  void observedData(const XMLParser::Node& root, const std::string& name);
  void observedValue(const XMLParser::Node& root, const std::string& name);
public:
  ParserNOAA0(const std::string& url, const std::string& str, const bool qVerbose = false);
}; // ParserNOAA0

#endif // INC_ParserNOAA0_H_
