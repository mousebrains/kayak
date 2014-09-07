#include "ReadFile.H"
#include <fstream>
#include <iostream>
#include <cerrno>
#include <cstring>

std::string
ReadFile(const std::string& filename,
         const bool complain)
{
  std::string str;
  std::ifstream is(filename.c_str());

  if (!is) {
    if (complain)
      std::cerr << "Error opening '" << filename << "', " << strerror(errno) << std::endl;
    return str;
  }

  char buffer[1024 * 1024];
  while (is.read(buffer, sizeof(buffer))) {
    str.append(buffer, is.gcount()); // Append to the string
  }
  str.append(buffer, is.gcount()); // Append to the string what was left in the buffer

  return str;
}
