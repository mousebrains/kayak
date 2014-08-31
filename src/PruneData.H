#ifndef INC_PruneData_H_
#define INC_PruneData_H_

#include <ctime>
#include <string>
#include <map>

class PruneData {
public:
  typedef std::multimap<time_t, size_t> tMap;
private:
  typedef std::map<time_t, int> tTimes;

  tTimes mTimes;

  void initialize();
  time_t initializeHours(time_t t, const int nWeeks, const int dHours);
public:
  PruneData();

    // Set rowid's, second field, to zero if they should be kept
    // and return number of entries to drop
  tMap::size_type operator () (tMap& a); 
};

#endif // INC_PruneData_H_
