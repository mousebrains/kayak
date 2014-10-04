#ifndef INC_Display_H_
#define INC_Display_H_

#include "Types.H"
#include <string>
#include <set>
#include <iosfwd>

class DataGet;
class Master;
class CGI;

namespace Display {
  std::string dateForm(const DataGet& obs,
                       const std::string& id = "frm");

  int mainPage();
  int levels();
  int gauges();
  int table();
  int plot();
  int info();
  int database();
  int data();

  Types::Keys dehashKeys(const CGI& cgi, const std::string& key);

  template <typename T>
  bool maybe(std::ostream& os, const T& val, 
             const std::string& prefix=std::string(), 
             const std::string& suffix=std::string());

  bool maybe(std::ostream& os, const double lat, const double lon,
             const std::string& prefix=std::string(), 
             const std::string& suffix=std::string());

  bool maybeLatLon(std::ostream& os, const double lat, const double lon,
                   const std::string& prefix=std::string(), 
                   const std::string& suffix=std::string());

  bool maybeLatLon(std::ostream& os, 
                   const double lat0, const double lon0,
                   const double lat1, const double lon1,
                   const std::string& prefix=std::string(), 
                   const std::string& suffix=std::string());

  bool maybeMinMax(std::ostream& os, const double min, const double max,
                   const std::string& prefix=std::string(), 
                   const std::string& suffix=std::string());

  bool maybeCalc(std::ostream& os, const std::string& expr,
                 Master& master,
                 const std::string& prefix=std::string(), 
                 const std::string& suffix=std::string());

  bool maybeHRef(std::ostream& os, const std::string& str, const std::string& url,
                 const std::string& prefix=std::string(), 
                 const std::string& suffix=std::string());
} // Display

#endif // INC_Display_H_
