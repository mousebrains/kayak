#ifndef INC_Curl_H_
#define INC_Curl_H_

#include <curl/curl.h>
#include <string>

class Curl {
private:
  bool mOkay;
  const std::string mURL;
  long mResponseCode;
  std::string mContentType;
  std::string mText;
  char mErrorMsg[CURL_ERROR_SIZE + 1024];

  void errchk(CURLcode code, const std::string& reason);
  static size_t callback(void *ptr, size_t size, size_t nmemb, void *data);
public:
  explicit Curl(const std::string& url, const bool qVerbose = false);

  operator bool () const {return mOkay;}
  const char *errmsg() const {return mErrorMsg;}

  const std::string& url() const {return mURL;}
  const std::string& str() const {return mText;}
  const std::string& type() const {return mContentType;}
  long responseCode() const {return mResponseCode;}
}; // Curl

#endif // INC_Curl_H_
