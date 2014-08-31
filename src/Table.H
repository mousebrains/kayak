#ifndef INC_Table_H_
#define INC_Table_H_

#include "MyDB.H"
#include "Fields.H"
#include <string>
#include <map>

class Table : public Fields {
private:
  struct Info {
    int key;
    time_t time;
    bool qModified;
  };

  typedef std::map<std::string, Info> tNameInfo;
  typedef std::map<int, std::string> tKeyName;

  tNameInfo mName2Info;
  tKeyName mKey2Name;

  bool mqModified;
  const Fields mFields;

  void updateTimes();
protected:
  MyDB mDB;

  Table(const Fields& fields);
  virtual ~Table();
public:
  int name2key(const std::string& name, const time_t t, const bool qInsert=true); 
  int name2key(const std::string& name) {return name2key(name, 0, false);} // Get without inserting
  std::string key2name(const int key); // Get without inserting

  typedef std::map<std::string, int> tNameKey;
  tNameKey allNameKeys();
};

#endif // INC_Table_H_
