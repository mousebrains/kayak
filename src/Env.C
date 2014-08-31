#include "Env.H"
#include <cstdlib>

std::string
Env::get(const std::string& name)
{
  return get(name.c_str());
}

std::string
Env::get(const char *name)
{
  const char *ptr(getenv(name));
  return ptr ? std::string(ptr) : std::string();
}
