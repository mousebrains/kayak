#include "CGI.H"
#include "Convert.H"
#include "Tokens.H"
#include "URL.H"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <set>

namespace {
  const std::string javaScriptKey("js");
  const std::string noCanvasKey("nc");

  bool qPathInitialized(false);
  Tokens path;

  bool qMapInitialized(false);
  CGI::tMap vars;

  void initPath() {
    if (qPathInitialized) {
      return;
    }
    const char *ptr(getenv("PATH_INFO"));
    if (ptr) { // PATH_INFO exists
      path = Tokens(ptr, "/");
    }
    std::for_each(path.begin(), path.end(), URL::decode);
    qPathInitialized = true;
  } // initPath

  void initMap() {
    if (qMapInitialized) {
      return;
    }
    const char *reqMethod(getenv("REQUEST_METHOD"));
    std::string input;
    if (reqMethod && !strcasecmp(reqMethod, "POST")) { // Use the post method
      const size_t n(Convert::strTo<size_t>(getenv("CONTENT_LENGTH")));
      char *buffer = new char[n+1];
      std::cin.get(buffer, n+1, '\0');
      input = buffer;
      delete [] buffer;
    } else { // Use the get method
      const char *ptr(getenv("QUERY_STRING"));
      input = ptr ? ptr : std::string(); 
    }

    const Tokens v(input, "&");
   
    for (Tokens::const_iterator it(v.begin()), et(v.end()); it != et; ++it) {
      const std::string::size_type n(it->find('='));
      if (n != it->npos) {
        vars.insert(std::make_pair(URL::decode(it->substr(0, n)), 
                                   URL::decode(it->substr(n + 1, it->npos))));
      } else {
        vars.insert(std::make_pair(URL::decode(*it), std::string()));
      }
    }
    qMapInitialized = true;
  } // initMap
} // anonymous

size_t
CGI::nPath() const
{
  initPath();
  return path.size();
}

const std::string&
CGI::operator [] (const size_t index) const
{
  initPath();
  static std::string dummy;
  return index < path.size() ? path[index] : dummy;
}

CGI::const_iterator
CGI::begin() const
{
  initMap();
  return vars.begin();
}

CGI::const_iterator
CGI::end() const
{
  initMap();
  return vars.end();
}

CGI::size_type
CGI::size() const
{
  initMap();
  return vars.size();
}

bool
CGI::empty() const
{
  initMap();
  return vars.empty();
}

bool
CGI::isSet(const std::string& key) const
{
  initMap();
  return vars.find(key) != vars.end();
}

const std::string&
CGI::get(const std::string& key) const
{
  static std::string dummy;
  return get(key, dummy);
}

const std::string&
CGI::get(const std::string& key,
         const std::string& dummy) const
{
  initMap();
  const_iterator it(vars.find(key));
  return it == vars.end() ? dummy : it->second;
}

std::string 
CGI::queryString(const bool qAddNoJS) const
{
  initMap();

  if (!qAddNoJS && vars.empty()) {
    return std::string();
  }

  std::string str;
  std::string delim("?");

  for (const_iterator it(vars.begin()), et(vars.end()); it != et; ++it) {
    if (!qAddNoJS || (it->first != javaScriptKey)) {
      str += delim + it->first + "=" + URL::encode(it->second);
    }
    delim = "&amp;";
  }

  if (qAddNoJS) {
    str += delim + javaScriptKey + "=0";
  }

  return str;
}

std::string 
CGI::queryString(const std::string& toSkip) const
{
  initMap();

  if (vars.empty()) {
    return std::string();
  }

  const Tokens toks(toSkip);
  std::set<std::string> skip;
  skip.insert(toks.begin(), toks.end());

  std::string str;
  std::string delim("?");

  for (const_iterator it(vars.begin()), et(vars.end()); it != et; ++it) {
    if (skip.find(it->first) == skip.end()) { // Not found
      str += delim + it->first + "=" + URL::encode(it->second);
      delim = "&amp;";
    }
  }

  return str;
}

std::string 
CGI::infoString(const std::string& name) const
{
  initMap();

  const std::vector<std::string> keys({"h", javaScriptKey});
  std::string str("?o=i");

  for (size_t i(0), e(keys.size()); i < e; ++i) {
    const std::string& key(keys[i]);
    const std::string& val(get(key));
    if (!val.empty()) {
      str += "&amp;" + key + "=" + URL::encode(val);
    }
  }

  return "<a href='" + str + "'>" + name + "</a>";
}

bool 
CGI::qJavaScript() const
{
  initMap();  
  return !isSet(javaScriptKey) || (get(javaScriptKey) != "0");
}

bool 
CGI::qNoCanvas() const
{
  initMap();  
  return isSet(noCanvasKey) && (get(noCanvasKey) == "1");
}
