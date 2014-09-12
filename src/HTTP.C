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

  *this << " " << statusMsg(code) << "\n";
  return *this;
}

const char *
HTTP::statusMsg(const int code)
{
  switch (code) {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 200: return "Ok";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Time-out";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Request Entity Too Large";
    case 414: return "Request-URI Too Large";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested range not satistifed";
    case 417: return "Expectation Failed";
    default:  return "Unknown reason";
  }
  return "Unknown reason";
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
