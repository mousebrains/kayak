#include "Calc.H"
#include <iostream>

namespace {
  void doCalc(MyDB& db, const std::string& field, const Data::Type type, const int nHoursBack) {
    MyDB::Stmt s(db);
    s << "SELECT gaugeKey," << field 
      << " FROM gauges WHERE "
      << field << " IS NOT NULL;";

    int rc;
    Data data;
    const time_t t0((time(0) / 3600) * 3600);
    const time_t dt(4 * 86400); // 7200 in production

    while ((rc = s.step()) == SQLITE_ROW) {
      const int gkey(s.getInt());
      const std::string expr(s.getString());
      Calc calc(gkey, expr, type);
      for (int i(0); i <= nHoursBack; ++i) {
        calc.update(data, t0 - i * 3600, dt); // Update data table
      }
    }

    if (rc != SQLITE_DONE) {
      s.errorCheck(rc, "doCalc");
    }
  }
} // anonymous

int
main (int argc,
      char **argv)
{
  const int nHoursBack(argc > 1 ? atoi(argv[1]) : 0);

  MyDB db;
  doCalc(db, "calcFlow", Data::FLOW, nHoursBack);
  doCalc(db, "calcGauge", Data::GAUGE, nHoursBack);
  return 0;
}
