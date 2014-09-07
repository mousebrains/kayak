#ifndef INC_DATAGET_H_
#define INC_DATAGET_H_

// Harvest data from the SQLite3 database

#include "Data.H"
#include <string>
#include <map>

class DataGet {
private:
  typedef Data::tObs tObs;
  tObs mObs;

  typedef Data::tAnObs tAnObs;
  tAnObs mAnObs;

  const std::string mHash;
  const size_t mKey;
  Data::Type mType;
  
  size_t mGaugeKey;
  MyDB::Stmt::tInts mSourceKeys;

  double mLow;
  double mHigh;
  double mLowOptimal;
  double mHighOptimal;

  double mLatitude;
  double mLongitude;

  std::string mMsg;
  std::string mTitle;
  std::string mYLabel;
  std::string mUnits;

  MyDB::Stmt::tInts mTypes;

  void fetch(time_t stime, time_t etime, bool qOne); // Popuplate DataGet from the master table key

  std::ostream& jsonObs(std::ostream& os) const;
  std::ostream& jsonAnObs(std::ostream& os) const;

  std::string mkLabel(const Data::Type type) const;
  std::string mkUnits(const Data::Type type) const;
public:
  explicit DataGet(const bool qOne = true); // Use CGI interface for one data type

  operator bool () const {return mMsg.empty();}

  const std::string& errmsg() const {return mMsg;}
  const std::string& title() const {return mTitle;}
  const std::string& ylabel() const {return mYLabel;}
  const std::string& units() const {return mUnits;}

  const std::string& hash() const {return mHash;}
  Data::Type type() const {return mType;}

  double low() const {return mLow;}
  double high() const {return mHigh;}
  double lowOptimal() const {return mLowOptimal;}
  double highOptimal() const {return mHighOptimal;}
 
  double latitude() const {return mLatitude;}
  double longitude() const {return mLongitude;}
 
  typedef tObs::const_iterator const_iterator;
  typedef tObs::const_reverse_iterator const_reverse_iterator;
  typedef tObs::size_type size_type;

  bool empty() const {return mObs.empty();}
  size_type size() const {return mObs.size();}
  const_iterator begin() const {return mObs.begin();}
  const_iterator end() const {return mObs.end();}
  const_reverse_iterator rbegin() const {return mObs.rbegin();}
  const_reverse_iterator rend() const {return mObs.rend();}

  std::string json() const;
  std::ostream& json(std::ostream& os) const;

  const MyDB::Stmt::tInts& types() const {return mTypes;}

  double roundY(const double y) const;
  static double roundY(const double y, const Data::Type type);

  const tAnObs& anObs() const {return mAnObs;}
};

#endif // INC_DATAGET_H_
