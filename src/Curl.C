#include "Curl.H"
#include "MyException.H"
#include "Convert.H"
#include "ReadFile.H"
#include <iostream>

Curl::Curl(const std::string& url,
           const bool qVerbose)
  : mOkay(false)
  , mURL(url)
  , mResponseCode(0)
{
  mErrorMsg[0] = '\0'; // Zero terminate the error message stream

  if (Convert::tolower(url.substr(0,9)).find("file:///") == 0) { // local file
    mText = ReadFile(url.substr(7));
    mOkay = true;
    mResponseCode = 200;
    return;
  }

  CURL *curl(curl_easy_init()); // Initialize a curl handle

  if (!curl) { // Failed
    throw MyException("Failed in curl_easy_init()");
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Set the URL to fetch
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow any redirections
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Skip any certificate verification
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // Skip hostname verification
  curl_easy_setopt(curl, CURLOPT_ENCODING, ""); // Enable compression
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, mErrorMsg); // Where to store the error message
  curl_easy_setopt(curl, CURLOPT_VERBOSE, qVerbose ? 1L : 0L); // Enable/disable verbose messages to stderr

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

  if (qVerbose) { // Dump diagnostic information, beyond CURLOPT_VERBOSE
    { // Get timestamp of file retrieved
      long a;
      errchk(curl_easy_getinfo(curl, CURLINFO_FILETIME, &a), "curl_easy_getinfo"); 
      std::cout << "Curl FileTime " << Convert::toStr((time_t) a, "%c") << std::endl;
    }
    { // Times to fetch the document
      double t;
      errchk(curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &t), "curl_easy_getinfo");
      std::cout << "Curl name lookup time " << t << std::endl;
      errchk(curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &t), "curl_easy_getinfo");
      std::cout << "Curl connect time " << t << std::endl;
      errchk(curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &t), "curl_easy_getinfo");
      std::cout << "Curl appconnect time " << t << std::endl;
      errchk(curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &t), "curl_easy_getinfo");
      std::cout << "Curl pre-transfer time " << t << std::endl;
      errchk(curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &t), "curl_easy_getinfo");
      std::cout << "Curl start transfer time " << t << std::endl;
      errchk(curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &t), "curl_easy_getinfo");
      std::cout << "Curl redirect time " << t << std::endl;
      errchk(curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &t), "curl_easy_getinfo");
      std::cout << "Curl total time " << t << std::endl;
    }
    { // size of the body retrieved
      double sz, sp;
      errchk(curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &sz), "curl_easy_getinfo");
      errchk(curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &sp), "curl_easy_getinfo");
      std::cout << "Curl size uploaded " << sz 
                << " at " << sp << " bytes/sec" << std::endl;
      errchk(curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &sz), "curl_easy_getinfo");
      errchk(curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &sp), "curl_easy_getinfo");
      std::cout << "Curl size downloaded " << sz 
                << " at " << sp << " bytes/sec " 
                << " expanded size " << mText.size()
                << " compression ratio " << ((double) sz / mText.size() * 100) << "%"
                << std::endl;
    }
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

