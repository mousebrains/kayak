#ifndef INC_GaugeTranslate_H_
#define INC_GaugeTranslate_H_

#include "MyDB.H"
#include <map>

class GaugeTranslate {
public:
  struct Info {
    std::string description;  
    std::string location;  
  };
private:
  MyDB mDB;
  typedef std::map<std::string, Info> tInfo;
  tInfo mInfo;
public:
  Info operator () (const std::string& str);
};

#endif // INC_GaugeTranslate_H_
