#ifndef INC_HTML_H_
#define INC_HTML_H_

#include <string>
#include <sstream>

class HTML : public std::ostringstream {
public:
  HTML();
  ~HTML();

  static std::string header();

  static std::string baseURL();
  static std::string myStyle();
  static std::string myScript();

  static std::string addOnStart();
  static std::string addOnEnd();

  static std::string usgsIdURL(const std::string& id);
  static std::string cbttIdURL(const std::string& id);

  static std::string weatherURL(const double lat, const double lon);
  static std::string weatherForm(const double lat, const double lon);

  static std::string mapURL(const double lat, const double lon);
  static std::string mapForm(const double lat, const double lon);
};

std::ostream& operator << (std::ostream& os, const HTML& h);

#endif // INC_HTML_H_
