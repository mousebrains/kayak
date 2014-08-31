#include <sqlite3.h>
#include <iostream>

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
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " sourceDatabase destDatabase" << std::endl;
    return 1;
  }

  const std::string src(argv[1]);
  const std::string tgt(argv[2]);
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
}
