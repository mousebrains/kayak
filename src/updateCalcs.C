#include "Calc.H"
#include <iostream>

namespace {
  void doCalc(MyDB& db, const std::string& field, const Data::Type type) {
    MyDB::Stmt s(db);
    s << "SELECT gaugeKey," << field 
      << " FROM gauges WHERE "
      << field << " IS NOT NULL;";

    int rc;
    while ((rc = s.step()) == SQLITE_ROW) {
      const int gkey(s.getInt());
      const std::string expr(s.getString());
      Calc calc(gkey, expr, type);
      calc.update(); // Update data table
    }

    if (rc != SQLITE_DONE) {
      s.errorCheck(rc, "doCalc");
    }
  }
} // anonymous

int
main (int,
      char **)
{
  MyDB db;
  doCalc(db, "calcFlow", Data::FLOW);
  doCalc(db, "calcGauge", Data::GAUGE);
  return 0;
}
