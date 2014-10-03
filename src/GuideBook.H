#ifndef INC_GuideBook_H_
#define INC_GuideBook_H_

#include "MyDB.H"
#include <string>
#include <set>

class GuideBook {
public:
  struct Info {
    int masterKey; // Key into Master table
    int bookKey; // Key into GuideBooks table
    std::string name; // Guide book name
    std::string author; // Guide book author
    std::string edition; // Guide book edition
    std::string urlGB; // Guide book URL
    std::string urlBase; // URL base to use for guide URL
    std::string url; // Run's URL
    std::string page; // Run's page number
    std::string run; // Run's name

    Info(const int key) : masterKey(key), bookKey(0) {};

    bool operator < (const Info& rhs) const {
      return (masterKey < rhs.masterKey) || 
             ((masterKey == rhs.masterKey) && 
              ((name < rhs.name) || 
               ((name == rhs.name) && 
                ((page < rhs.page) || 
                 ((page == rhs.page) && (run < rhs.run))))));
    }

    std::string mkHTML() const;
  };

  typedef std::multiset<Info> tInfo;
  typedef tInfo::const_iterator const_iterator;
  typedef tInfo::size_type size_type;
private:
  tInfo mInfo;
  MyDB mDB;

  void loadInfo(const int key); // Load information for a master key into mInfo
public:
  explicit GuideBook(const int key); // Master key to get information for

  typedef std::set<int> tKeys;
  explicit GuideBook(const tKeys& keys); // Master keys to get information for

  const_iterator begin() const {return mInfo.begin();}
  const_iterator end() const {return mInfo.end();}
  size_type size() const {return mInfo.size();}
  bool empty() const {return mInfo.empty();}
}; // GuideBook

#endif // INC_GuideBook_H_
