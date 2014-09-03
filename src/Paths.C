#include "Paths.H"

#ifndef PATHS_USER_ROOT
#define PATHS_USER_ROOT "/home/tpw"
#endif // PATHS_USER_ROOT

#ifndef PATHS_PREFIX
#define PATHS_PREFIX PATHS_USER_ROOT "/kayaking2"
#endif // PATHS_PREFIX

namespace {
  std::string userRoot() {return std::string(PATHS_USER_ROOT);}
  std::string root() {return std::string(PATHS_PREFIX);}
  std::string files() {return root() + "/files";}

  std::string templates() {return root() + "/templates";}
  std::string pages() {return userRoot() + "/public_html/kayaking2";}

  std::string mainPageName() {return "/index.html";}
} // Anonymous

namespace Paths {
  std::string dbname() {return files() + "/a.db";}
  std::string mainPage() {return pages() + mainPageName();}
  std::string mainPageHead() {return templates() + mainPageName() + ".head";}
  std::string mainPageTail() {return templates() + mainPageName() + ".tail";}

  std::string fontPath() {return userRoot() + "/local/share/fonts/truetype/ttf-bitstream-vera";}
} // Paths
