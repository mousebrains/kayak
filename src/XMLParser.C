#include "XMLParser.H"

XMLParser::XMLParser(const std::string& str)
  : mContent(str.begin(), str.end())
{
  mDoc.parse<0>(&mContent[0]); 
}
