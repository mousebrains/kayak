#ifndef INC_ParserUSGS1_H_
#define INC_ParserUSGS1_H_

#include "XMLParser.H"
#include "Gauges.H"
#include <string>
#include <vector>

class ParserUSGS1 {
private:
  bool mqVerbose;
  Gauges mGauges;

  void collection(const XMLParser::Node& root);
public:
  ParserUSGS1(const std::string& url, const std::string& str, const bool qVerbose = false);
}; // ParserUSGS1

#endif // INC_ParserUSGS1_H_
