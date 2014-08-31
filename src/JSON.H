#ifndef INC_JSON_H_
#define INC_JSON_H_

#include <sstream>
#include <vector>

namespace JSON {
  template <typename T>
  class Array {
  private:
    std::string mName;
    std::vector<T> mValues;
    std::ostream& dumpSmall(std::ostream& os) const;
  public:
    explicit Array(const std::string& name) : mName(name) {}
    void operator () (const T& val) {mValues.push_back(val);}
    std::ostream& dump(std::ostream& os) const;
  };  // Array
} // JSON

template<typename T>
std::ostream& operator << (std::ostream& os, const JSON::Array<T>& a) {
  return a.dump(os);
}

#endif // INC_JSON_H_
