#include "Master.H"
#include "Convert.H"
#include <iostream>
#include <getopt.h>

int
main (int argc,
      char **argv)
{
  const char *options("k");
  bool qKey2Hash(true);

  for (int c; (c = getopt(argc, argv, options)) != EOF;) {
    qKey2Hash = !qKey2Hash;
  }
 
  for (int i(optind); i < argc; ++i) {
    if (qKey2Hash) {
      const size_t val(atoi(argv[i]));
      const std::string str(Master::mkHash(val));
      std::cout << "argv[" << i << "]=" << val << " hash '" 
                << str
                << "' dehash "
                << Master::deHash(str)
                << std::endl;
    } else { // Hash 2 key
      const size_t key(Master::deHash(argv[i]));
      const std::string hash(Master::mkHash(key));
      std::cout << "argv[" << i << "]='" << argv[i] << "' key " << key << " hash(" << hash << ")" << std::endl;
    }
  }

  return 0;
}
