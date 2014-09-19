#include "Paths.H"

#ifndef PATHS_USER_ROOT
#define PATHS_USER_ROOT "/home/tpw"
#endif // PATHS_USER_ROOT

#ifndef PATHS_FILES_ROOT
#define PATHS_FILES_ROOT PATHS_USER_ROOT "/kayaking2"
#endif // PATHS_FILES_ROOT

#ifndef PATHS_WEB_BASE
#define PATHS_WEB_BASE "/kayaking2"
#endif // PATHS_WEB_ROOT

#ifndef PATHS_FONTFACE
#define PATHS_FONTFACE "/robot/Roboto-Thin.ttf"
#endif // PATHS_WEB_ROOT


namespace {
  std::string filesRoot() {return std::string(PATHS_FILES_ROOT);}
  std::string files() {return filesRoot() + "/files";}
  std::string templates() {return filesRoot() + "/templates";}

  std::string mainPageName() {return "/index.html";}
} // Anonymous

namespace Paths {
  std::string dbname() {return files() + "/a.db";}
  std::string mainPageHead() {return templates() + mainPageName() + ".head";}
  std::string mainPageTail() {return templates() + mainPageName() + ".tail";}

  std::string font() {return filesRoot() + "/fonts/truetype" + PATHS_FONTFACE;}

  std::string webBase () {return std::string(PATHS_WEB_BASE);}
} // Paths
