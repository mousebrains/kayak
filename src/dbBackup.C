#include "Paths.H"
#include "Convert.H"
#include "File.H"
#include <sqlite3.h>
#include <iostream>
#include <getopt.h>
#include <dirent.h>
#include <cerrno>
#include <set>

namespace {
  void errchk(int rc, sqlite3 *db, const std::string& msg) {
    switch (rc) {
      case SQLITE_DONE:
      case SQLITE_OK: return;
      default:
        std::cerr << msg << ", " << sqlite3_errmsg(db) << std::endl; 
        exit(1);
    }
  }
} // anonymous

int
main (int argc,
      char **argv)
{
  const char *options("p:?");

  int nBackups(0);

  for (int c; (c = getopt(argc, argv, options)) != EOF;) {
    switch (c) {
      case 'p': nBackups = atoi(optarg); break;
      default: std::cerr << "Unrecognized options(" << ((char) c) << ")" << std::endl;
      case '?':
        std::cerr << "Usage: " << argv[0] << " -{" << options << "}\n"
                  << "-p # Number of backups to retain\n"
                  << "-?   Display this message" << std::endl;
        exit (1);
    }
  }

  const std::string src(Paths::dbname());
  const std::string tgt(src + Convert::toStr(time(0), ".%Y%m%d.%H%M%S"));
  sqlite3 *srcDB, *tgtDB;

  errchk(sqlite3_open(src.c_str(), &srcDB), srcDB, "Opening '" + src + "'");
  errchk(sqlite3_open(tgt.c_str(), &tgtDB), tgtDB, "Opening '" + tgt + "'");

  sqlite3_backup *back(sqlite3_backup_init(tgtDB, "main", srcDB, "main"));
  if (!back) errchk(SQLITE_ERROR, tgtDB, "Backup init '" + src + "' -> '" + tgt + "'");

  int rc;
  for (rc = sqlite3_backup_step(back, -1); 
       (rc == SQLITE_OK) || (rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED);
       rc = sqlite3_backup_step(back, -1)) {
    sqlite3_sleep(250); // Wait 250ms before trying again
  }
 
  if (rc != SQLITE_DONE) {
    errchk(rc, tgtDB, "Backup step '" + src + "' -> '" + tgt + "'");
  }

  errchk(sqlite3_backup_finish(back), tgtDB, "Backup finish '" + src + "' -> '" + tgt + "'");

  errchk(sqlite3_close(srcDB), srcDB, "Closing '" + src + "'");
  errchk(sqlite3_close(tgtDB), tgtDB, "Closing '" + tgt + "'");

  if (nBackups > 0) { // check that we don't have too many backup copies
    const std::string dirname(File::dirname(src));
    DIR *ptr(opendir(dirname.c_str()));
    if (!ptr) {
      std::cerr << "Error opening '" << dirname << "', " << strerror(errno) << std::endl;
      exit(1);
    }

    typedef std::set<std::string> tFiles;
    tFiles files;
    const std::string prefix(File::tail(src) + ".");

    for (struct dirent *dp; (dp = readdir(ptr));) {
      const std::string fn(dp->d_name);
      if (fn.find(prefix) == 0) files.insert(fn);
    }
    closedir(ptr);

    if (files.size() > nBackups) {
      int n(files.size() - nBackups);
      for (tFiles::const_iterator it(files.begin()), et(files.end()); 
           n && (it != et); ++it, --n) {
        const std::string fn(dirname + *it);
        if (unlink(fn.c_str())) {
          std::cerr << "Error deleting '" << fn << "', " << strerror(errno) << std::endl;
        }
      }
    }
  }

  return 0;
}
