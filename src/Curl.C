#include "Curl.H"
#include "MyException.H"
#include <iostream>

Curl::Curl(const std::string& url)
  : mOkay(false)
  , mURL(url)
{
  CURL *curl(curl_easy_init()); // Initialize a curl handle

  if (!curl) { // Failed
    throw MyException("Failed in curl_easy_init()");
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Set the URL to fetch
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow any redirections
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Skip any certificate verification
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // Skip hostname verification
  curl_easy_setopt(curl, CURLOPT_ENCODING, ""); // Enable compression

  // Now setup the callback for receiving the data
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) this);
  
  errchk(curl_easy_perform(curl), "curl_easy_perform()"); // Fetch the page

  errchk(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &mResponseCode), 
         "curl_easy_getinfo(CURLINFO_RESPONSE_CODE");

  {
    char *ptr;
    errchk(curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ptr), 
           "curl_easy_getinfo(CURLINFO_CONTENT_TYPE");
    mContentType = ptr;
  }

  curl_easy_cleanup(curl); // Clean everything up
  mOkay = true;
}

size_t
Curl::callback(void *ptr,
               size_t size,
               size_t nmemb,
               void *data)
{
  const size_t nbytes(size * nmemb);
  ((Curl *) data)->mText.append((const char *) ptr, nbytes);
  return nbytes;
}

void 
Curl::errchk(CURLcode code, 
             const std::string& reason)
{
  if (code != CURLE_OK) {
    throw MyException("ERROR: " + reason + " for '" + mURL + "', " + curl_easy_strerror(code));
  }
}

