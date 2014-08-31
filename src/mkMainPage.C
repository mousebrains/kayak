#include "Master.H"
#include "Paths.H"
#include "File.H"
#include "ReadFile.H"
#include "afstream.H"
#include <iostream>
#include <cstdio>

namespace {
  std::string genRow(const std::string& state, const std::string& label) {
    return "<tr><th><a href=\"tbl.php?P=" + state + "\">" + label +
            "</a>&nbsp;</th><td><a href=\"txt.php?P=" + state + "\">Text version</a></td></tr>\n";
  }
} // anonymous

int
main (int argc,
      char **argv)
{
  bool qForce(false);
  const char *options("fh");
  for (int c; (c = getopt(argc, argv, options)) != EOF;) {
    switch (c) {
      case 'f': qForce = true; break;
      default: std::cerr << "Unrecognized option '" << ((char) c) << "'" << std::endl;
      case 'h':
        std::cerr << "Usage: " << argv[0] << "-{" << options << "}" << std::endl
                  << std::endl
                  << "-f force page to be generated" << std::endl
                  << "-h display this message" << std::endl;
        return 1;
    }
  }

  Master m;
  const std::string& filename(Paths::mainPage());
  const std::string& headname(Paths::mainPageHead());
  const std::string& tailname(Paths::mainPageTail());
  const time_t tRef(File::mtime(filename));

  qForce |= m.qModifiedSince(tRef)
         || tRef <= File::mtime(headname)
         || tRef <= File::mtime(tailname)
         || tRef <= File::mtime(argv[0]);

  if (!qForce) {
    return 0;
  }

  const Master::tSet states(m.allStates());
  std::string contents(ReadFile(headname));
  contents += genRow("All", "All states");

  for (Master::tSet::const_iterator it(states.begin()), et(states.end()); it != et; ++it) {
    contents += genRow(*it, *it);
  }

  contents += ReadFile(tailname);

  oafstream os(filename, 0644);

  os << contents;

  return 0;
}
