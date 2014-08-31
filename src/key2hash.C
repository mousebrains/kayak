#include "Master.H"
#include "Convert.H"
#include <iostream>

int
main (int argc,
      char **argv)
{
  for (int i(1); i < argc; ++i) {
    const size_t val(atoi(argv[i]));
    const std::string str(Master::mkHash(val));
    
    std::cout << "arg[" << i << "]=" << val << " hash '" 
              << str
              << "' dehash "
              << Master::deHash(str)
              << std::endl;
  }

  return 0;
}
