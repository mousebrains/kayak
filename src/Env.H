#ifndef INC_Env_H_
#define INC_Env_H_

#include <string>

namespace Env  {
  std::string get(const std::string &name);
  std::string get(const char *name);
}

#endif // INC_Env_H_
