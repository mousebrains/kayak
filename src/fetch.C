// Fetch a set of URLs and then parse them

#include "ParserNOAA0.H"
#include "ParserUSGS0.H"
#include "ParserUSGS1.H"
#include "Curl.H"
#include "HTTP.H"
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <getopt.h>

namespace {
  int usage(const char *argv0, const char *options) {
    std::cerr << "Usage: " << argv0 << " -{" << options << "}\n"
              << "\n"
              << "-h            show this message\n"
              << "-s filename   save information retrieved from URL to filename\n"
              << "-u parser:URL fetch URL and parse with parser\n"
              << "-v            verbose mode" << std::endl;
    return 1;
  }
} // anonymous

int
main (int argc,
      char **argv)
{
  const char *options("hs:u:v");
  bool qVerbose(false);
  std::string saveFilename;

  typedef std::vector<std::string> tStrings;
  tStrings urls;

  for (int c; (c = getopt(argc, argv, options)) != EOF;) {
    switch (c) {
      case 's': saveFilename = optarg; break;
      case 'u': urls.push_back(optarg); break;
      case 'v': qVerbose = !qVerbose; break;
      default:
        std::cerr << "Unrecognized option (" << ((char) optopt) << ")" << std::endl;
      case 'h':
        return usage(argv[0], options);
    }
  }

  if (optind < argc) {
    std::cerr << "Unprocessed command line arguments:";
    for (int i(optind); i < argc; ++i) {
      std::cerr << " '" << argv[i] << "'";
    }
    std::cerr << std::endl;
    return usage(argv[0], options);
  }

  if (urls.empty()) { // Build from database
    std::cerr << "urls from database not currently implemented!" << std::endl;
    return 1;
  }

  for (tStrings::const_iterator it(urls.begin()), et(urls.end()); it != et; ++it) {
    const std::string::size_type j(it->find(':'));
    if (j == it->npos) {
      std::cerr << "Malformed parser:URL field(" << *it << ")" << std::endl;
      return 1;
    }
    const std::string parser(it->substr(0,j));
    const std::string url(it->substr(j+1));
    Curl curl(url, qVerbose); // Load the file
    if (!curl) {
      std::cerr << "Error fetching '" << url << "', " << curl.errmsg() << std::endl;
      continue;
    }
    if (curl.responseCode() != 200) {
      std::cerr << "Error fetching '" << url 
                << "', response code " << curl.responseCode()
                << ", " << HTTP::statusMsg(curl.responseCode())
                << std::endl;
      continue;
    }

    if (!saveFilename.empty()) {
      std::ofstream os(saveFilename);
      if (!os) {
         std::cerr << "Error opening '" << saveFilename << "', " << strerror(errno) << std::endl;
         return 1;
      }
      os << curl.str();
    }

    if (parser == "USGS0") {
      ParserUSGS0 a(url, curl.str(), qVerbose);
    } else if (parser == "USGS1") {
      ParserUSGS1 a(url, curl.str(), qVerbose);
    } else if (parser == "NOAA0") {
      ParserNOAA0 a(url, curl.str(), qVerbose);
    } else {
      std::cerr << "Unrecongized parser '" << parser << "'" << std::endl;
      continue;
    }
  }

  return 0;
}
