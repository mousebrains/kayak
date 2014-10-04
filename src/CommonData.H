#ifndef INC_CommonData_H_
#define INC_CommonData_H_

#include "MyDB.H"
#include "CommonSource.H"
#include "CommonURLs.H"
#include "CommonUnknown.H"
#include <iosfwd>
#include <string>
#include <set>

class CommonData {
public:
  enum Type {
    FIRSTTYPE = 1,
    FLOW = FIRSTTYPE,
    GAUGE,
    TEMPERATURE,
    INFLOW,
    LASTTYPE
  };
protected:
  MyDB mDB;
  CommonSource mSource;
  CommonURLs mURLs;
  CommonUnknown mUnknown;
  const DataFields mFields;
public:
  explicit CommonData(const DataFields& f)
    : mSource(f.table())
    , mURLs(f.table())
    , mUnknown(f.table())
    , mFields(f)
  {}

  virtual ~CommonData();

  static Type decodeType(const std::string& str);

  CommonSource& source() {return mSource;}
  CommonURLs& urls() {return mURLs;}

  Types::Keys allKeys();

  void dropRows(const Types::Keys& keys);
  size_t nRows(const int key);

  void checkPoint() {mDB.checkPoint();}
};

std::ostream& operator << (std::ostream& os, const CommonData::Type a);

#endif // INC_CommonData_H_
