#ifndef INC_File_H_
#define INC_File_H_

#include <sys/stat.h>
#include <string>
#include <ctime>

class File {
private:
  bool mOkay;
  struct stat mStat;
  const std::string mFilename;
public:
  explicit File(const std::string& filename);

  operator bool () const {return mOkay;}

  off_t size() const;
  static off_t size(const std::string& fn) {return File(fn).size();}

  time_t mtime() const;
  static time_t mtime(const std::string& fn) {return File(fn).mtime();}

  static std::string dirname(const std::string& fn);
  std::string dirname() const {return dirname(mFilename);}

  static std::string tail(const std::string& fn);
  std::string tail() const {return dirname(mFilename);}
}; // File

#endif // INC_File_H_
