#ifndef INC_CommonUnknown_H_
#define INC_CommonUnknown_H_

#include "Table.H"
#include "Fields.H"
#include <string>

class CommonUnknown : public Table {
public:
  enum Usage {
    SEEN = 0,
    NOTIFIED,
    IGNORE,
    LASTUSAGE
  };
private:
  const UnknownFields mFields;
public:
  explicit CommonUnknown(const std::string& prefix);
  virtual ~CommonUnknown();

  Usage usage(const int key);
};

std::ostream& operator << (std::ostream& os, const CommonUnknown::Usage u);

#endif // INC_CommonUnknown_H_
