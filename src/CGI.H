#ifndef INC_CGI_H_
#define INC_CGI_H_

#include <string>
#include <map>

class CGI {
public:
  CGI() {}

  size_t nPath() const;
  const std::string& operator [] (const size_t i) const;

  typedef std::map<std::string, std::string> tMap;
  typedef tMap::const_iterator const_iterator;
  typedef tMap::size_type size_type;

  const_iterator begin() const;
  const_iterator end() const;
  size_type size() const;
  bool empty() const;

  bool isSet(const std::string& key) const;

  const std::string& get(const std::string& key) const;
  const std::string& get(const std::string& key, const std::string& def) const;

  std::string queryString(bool qAddNoJS = false) const;
  std::string queryString(const std::string& toSkip) const;
  std::string infoString(const std::string& name) const;

  bool qJavaScript() const;
  bool qNoCanvas() const;
};

#endif // INC_CGI_H_
