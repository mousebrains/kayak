#ifndef INC_HTTP_H_
#define INC_HTTP_H_

#include <string>
#include <iostream>

class HTML;

class HTTP : public std::ostream {
public:
  explicit HTTP(std::ostream& os) : std::ostream(os.rdbuf()) {}

  ~HTTP();

  int errorPage(const std::string& msg);

  int htmlPage(const time_t duration, HTML& html);

  static std::string date(const time_t& when);
  HTTP& expires(const time_t& when);
  HTTP& modified(const time_t& when);
  HTTP& content(const std::string& type="text/html");
  HTTP& encoding(const std::string& encoding);
  HTTP& location(const std::string& location);
  HTTP& setCookie(const std::string& cookie);
  HTTP& length(const size_t length);
  HTTP& length(HTML& html);
  HTTP& refresh(const size_t seconds);
  HTTP& noCache();
  HTTP& status(const int code=200, const std::string& reason=std::string());

  HTTP& end();
};

#endif // INC_HTTP_H_
