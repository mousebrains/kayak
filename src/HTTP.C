#include "HTTP.H"
#include "HTML.H"
#include <fstream>
#include <cstdlib>
#include <cstring>

HTTP::~HTTP()
{
}

std::string
HTTP::date(const time_t& when) 
{
  struct tm *gmt(gmtime(&when));
  char buffer[256];
  strftime(buffer, sizeof(buffer), "%A, %d %b %Y %H:%M:%S GMT", gmt);
  return buffer;
}

HTTP&
HTTP::content (const std::string& type) 
{
  *this << "Content-Type: " << type << "\n";
  return *this;
}

HTTP&
HTTP::expires (const time_t& when) 
{
  *this << "Expires: " << date(when) << "\n";
  return *this;
}

HTTP&
HTTP::modified (const time_t& when) 
{
  *this << "Last-Modified: " << date(when) << "\n";
  return *this;
}

HTTP&
HTTP::encoding(const std::string& encoding) 
{
  *this <<  "Content-Encoding: " << encoding << "\n";
  return *this;
}

HTTP&
HTTP::length(const size_t length) 
{
  *this << "Content-Length: " << length << '\n';
  return *this;
}

HTTP&
HTTP::length(HTML& html) 
{
  return length(html.tellp());
}

HTTP&
HTTP::setCookie(const std::string& cookie) 
{
  *this << "Set-Cookie: " << cookie << "\n";
  return *this;
}

HTTP&
HTTP::location(const std::string& url) 
{
  *this << "Location: " << url << "\n";
  return *this;
}

HTTP&
HTTP::noCache()
{
  *this << "Pragma: no-cache\nCache-control: no-cache\n";
  expires(time_t(0));
  return *this;
}

HTTP&
HTTP::end()
{
  *this << "\n";
  return *this;
}

HTTP&
HTTP::refresh(const size_t seconds) 
{
  *this << "Refresh: " << seconds << '\n';
  return *this;
}

HTTP&
HTTP::status(const int code,
             const std::string& str)
{
  *this << "status: " << code;

  if (!str.empty()) {
    *this << " " << str << "\n";
    return *this;
  }

  switch (code) {
    // case 100: oss << " Continue\n"; break;
    // case 101: oss << " Switching Protocols\n"; break;
    case 200: *this << " Ok\n"; break;
    // case 201: oss << " Created\n"; break;
    // case 202: oss << " Accepted\n"; break;
    // case 203: oss << " Non-Authoritative Information\n"; break;
    // case 204: oss << " No Content\n"; break;
    // case 205: oss << " Reset Content\n"; break;
    // case 206: oss << " Partial Content\n"; break;
    // case 300: oss << " Multiple Choices\n"; break;
    // case 301: oss << " Moved Permanently\n"; break;
    // case 302: oss << " Found\n"; break;
    // case 303: oss << " See Other\n"; break;
    // case 304: oss << " Not Modified\n"; break;
    // case 305: oss << " Use Proxy\n"; break;
    // case 307: oss << " Temporary Redirect\n"; break;
    case 400: *this << " Bad Request\n"; break;
    // case 401: oss << " Unauthorized\n"; break;
    // case 402: oss << " Payment Required\n"; break;
    // case 403: oss << " Forbidden\n"; break;
    // case 404: oss << " Not Found\n"; break;
    // case 405: oss << " Method Not Allowed\n"; break;
    // case 406: oss << " Not Acceptable\n"; break;
    // case 407: oss << " Proxy Authentication Required\n"; break;
    // case 408: oss << " Request Time-out\n"; break;
    // case 409: oss << " Conflict\n"; break;
    // case 410: oss << " Gone\n"; break;
    // case 411: oss << " Length Required\n"; break;
    // case 412: oss << " Precondition Failed\n"; break;
    // case 413: oss << " Request Entity Too Large\n"; break;
    // case 414: oss << " Request-URI Too Large\n"; break;
    // case 415: oss << " Unsupported Media Type\n"; break;
    // case 416: oss << " Requested range not satistifed\n"; break;
    // case 417: oss << " Expectation Failed\n"; break;
    default: *this << " Unknown reason\n"; break;
  }

  return *this;
}

int
HTTP::errorPage(const std::string& msg)
{
  const time_t now(time(0));

  HTML html;
  html << html.header()
       << "<title>" << msg << "</title>\n"
       << "</head>\n<body>\n"
       << "<h1>" << msg << "</h1>\n"
       << "</body>\n</html>\n";
  content() ;
  modified(now);
  expires(now);
  length(html);
  status(400, msg);
  end();
  *this << html;

  return 1;
}

int
HTTP::htmlPage(const time_t duration,
               HTML& html)
{
  const time_t now(time(0));
  content();
  modified(now);
  expires(now + duration);
  length(html.tellp());
  end();
  *this << html;
  return 0;
}
