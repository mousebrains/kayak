#ifndef INC_Fields_H_
#define INC_Fields_H_

#include <string>

class Fields {
private:
  const std::string mTable;
  const std::string mKey;
public:
  Fields() {}
  explicit Fields(const Fields& f) 
    : mTable(f.mTable), mKey(f.mKey) {}
  explicit Fields(const std::string& tbl) : mTable(tbl), mKey("key") {}
  Fields(const std::string& tbl, const std::string& key) : mTable(tbl), mKey(key) {}

  virtual ~Fields() {}

  virtual const std::string& table() const {return mTable;}
  virtual const std::string& key() const {return mKey;}
  virtual const char *name() const {return "name";}
  virtual const char *time() const {return "date";}
}; // Fields

class URLsFields : public Fields {
public:
  URLsFields() {}
  explicit URLsFields(const Fields& f) : Fields(f) {}
  explicit URLsFields(const std::string& tbl) : Fields(tbl, "urlKey") {}
  virtual ~URLsFields() {}
  virtual const char *parser() const {return "parser";}
  virtual const char *qRun() const {return "qRun";}
  virtual const char *monthOfYear() const {return "monthOfYear";}
  virtual const char *dayOfMonth() const {return "dayOfMonth";}
  virtual const char *hourOfDay() const {return "hourOfDay";}
}; // URLsFields

class SourceFields : public Fields {
public:
  SourceFields() {}
  explicit SourceFields(const SourceFields& f) : Fields(f) {}
  explicit SourceFields(const std::string& prefix) : Fields(prefix + "Source", prefix + "Key") {}
  virtual ~SourceFields() {}
  virtual const char *gaugeKey() const {return "gaugeKey";}
}; // SourceFields

class DataFields : public Fields {
public:
  DataFields() {}
  explicit DataFields(const DataFields& f) : Fields(f) {}
  explicit DataFields(const std::string& prefix) : Fields(prefix, prefix + "Key") {}
  virtual ~DataFields() {}
  virtual const char *urlKey() const {return "urlKey";}
  virtual const char *source() const {return "sourceKey";}
}; // DataFields

class UnknownFields : public Fields {
public:
  UnknownFields() {}
  explicit UnknownFields(const UnknownFields& f) : Fields(f) {}
  explicit UnknownFields(const std::string& prefix) : Fields(prefix + "Unknown", "urlKey") {}
  virtual ~UnknownFields() {}
  virtual const char *usage() const {return "usage";}
}; // UnknownFields

class MasterFields {
public:
  static const char *table() {return "master";}
  static const char *displayName() {return "displayName";}
  static const char *riverName() {return "riverName";}
  static const char *gaugeKey() {return "gaugeKey";}
  static const char *key() {return "key";}
  static const char *location() {return "location";}
  static const char *modified() {return "modified";}
  static const char *state() {return "state";}
  static const char *lowFlow() {return "lowFlow";}
  static const char *highFlow() {return "highFlow";}
  static const char *lowGauge() {return "lowGauge";}
  static const char *highGauge() {return "highGauge";}
  static const char *optimalLowFlow() {return "optimalLowFlow";}
  static const char *optimalHighFlow() {return "optimalHighFlow";}
  static const char *latitudePutin() {return "latitudePutin";}
  static const char *longitudePutin() {return "longitudePutin";}
  static const char *latitudeTakeout() {return "latitudeTakeout";}
  static const char *longitudeTakeout() {return "longitudeTakeout";}
  static const char *sortKey() {return "sortKey";}
  static const char *grade() {return "class";}
};

#endif // INC_Fields_H_
