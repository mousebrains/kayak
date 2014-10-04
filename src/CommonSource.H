#ifndef INC_CommonSource_H_
#define INC_CommonSource_H_

#include "Table.H"
#include "Fields.H"
#include <string>
#include <map>

class CommonSource : public Table {
protected:
  const SourceFields mFields;
public:
  explicit CommonSource(const std::string& prefix);

  virtual ~CommonSource() {}

  const SourceFields& fields() const {return mFields;}

  void setGauge(const std::string& name, const int gaugeKey);

  MyDB::tInts gaugeKey2Keys(const size_t gaugeKey);

  typedef std::map<int, int> tSourceKey2GaugeKey;
  tSourceKey2GaugeKey dataKeys(const MyDB::tInts& gaugeKeys);
};

#endif // INC_CommonSource_H_
