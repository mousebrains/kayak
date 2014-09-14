#ifndef INC_StateCounty_H_
#define INC_StateCounty_H_

#include "MyDB.H"

class StateCounty {
private:
  MyDB mDB;
  typedef std::map<int, std::string> tMap;
  tMap mStates;
  tMap mCounties;
public:
  std::string state(const int code); 
  std::string county(const int stateCode, const int countyCode); 
}; // StateCounty

#endif // INC_StateCounty_H_
