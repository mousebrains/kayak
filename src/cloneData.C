// From the existing mySQL Master table and get all the gauges in db_name and merged_dbs
// From the existing mySQL data tables get all the gauges stored there
// Take the interesection of the two sets
// Ignore any data tables without any data
// Ignore any data tables without recent data
// Save only what remains into SQLite's data and gauges tables
// 
#include "MySQL.H"
#include <iostream>

int
main (int argc,
    char **argv)
{
  int maxCount(argc > 1 ? atoi(argv[1]) : 1000000000);
  MySQL dsql("levels_data");
}
