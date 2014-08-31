#ifndef INC_CommonURLs_H_
#define INC_CommonURLs_H_

#include "Table.H"
#include "Fields.H"
#include <string>

class CommonURLs : public Table {
private:
  const URLsFields mFields;
public:
  explicit CommonURLs(const std::string& prefix);
  virtual ~CommonURLs();

  std::string parser(const int key);
  bool qOkayToRun(const int key, const time_t t);
};

#endif // INC_CommonURLs_H_
