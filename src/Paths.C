#include "Paths.H"

namespace {
  std::string userRoot() {return "/home/tpw";}
  std::string root() {return userRoot() + "/kayaking2";}
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
