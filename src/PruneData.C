#include "PruneData.H"
#include <iostream>

PruneData::PruneData()
{
}

PruneData::tMap::size_type
PruneData::operator () (tMap& a)
{
  if (a.empty()) { // Nothing to check
    return 0; // Nothing to keep
  }

  if (mTimes.empty()) {
    initialize();
  }

  tMap::iterator jit(a.begin());
  tMap::iterator jet(a.end());

  tMap::size_type n(0);

  for (tTimes::const_iterator it(mTimes.begin()), et(mTimes.end()); it != et; ++it) {
    const time_t t(it->first);
    const int dtMax(it->second);
    tMap::iterator jt(a.lower_bound(t)); // time in a >= t
    if (jt == jet) { // no time greater, so check previous time
      tMap::reverse_iterator kt(a.rbegin());
      const int dt(t - kt->first);
      if (dt <= dtMax) {
        n += (kt->second != 0);
        kt->second = 0;
      }
    } else if (jt == jit) { // At beginning
      const int dt(jt->first - t);
      if (dt <= dtMax) {
        n += (jt->second != 0);
        jt->second = 0;
      }
    } else { // not at beginning or past end
      tMap::iterator kt(jt--); // previous entry
      const int dtLHS(t - jt->first);
      const int dtRHS(kt->first - t);
      if (dtLHS < dtRHS) {
        if (dtLHS <= dtMax) {
          n += (jt->second != 0);
          jt->second = 0;
        }
      } else {
        if (dtRHS <= dtMax) {
          n += (kt->second != 0);
          kt->second = 0;
        }
      }
    } 
  }

  return a.size() - n; // # to prune
}

void
PruneData::initialize()
{
  time_t t(time(0)); // now
  struct tm *ptr(localtime(&t));

  ptr->tm_sec = 0; // floor seconds to minute
  ptr->tm_min = (ptr->tm_min / 15) * 15; // floor to 15 minutes
  t = mktime(ptr); // back to seconds
  t += 15 * 60; // 15 minutes into the future

  mTimes.clear(); // Clear all existing target times

  for (int i(0), e(4 * 24 * 2 * 7); i < e; ++i) { // Every 15 minutes back 2 weeks
    mTimes.insert(std::make_pair(t, 450));
    t -= 15 * 60;
  }

  t = initializeHours(t + 86400, 4, 1); // every hours for 4 weeks
  t = initializeHours(t, 8, 2); // every 2 hours for 8 weeks
  t = initializeHours(t, 52, 6); // every 6 hours for 26 weeks
  t = initializeHours(t, 52, 12); // every 12 hours for 52 weeks
  t = initializeHours(t, 52, 24); // every 24 hours for 52 weeks
}

time_t 
PruneData::initializeHours(time_t t, 
                           const int nWeeks,
                           const int dHours)
{
  const int dt((dHours * 3600) / 2);

  for (int i(0), e(nWeeks * 7); i < e; ++i) { // walk back nWeeks by days
    struct tm *ptr(localtime(&t));
    ptr->tm_sec = 0;
    ptr->tm_min = 0;
    ptr -> tm_hour = 0;
    t = mktime(ptr);
    for (int j(0); j < 24; j += dHours) {
      mTimes.insert(std::make_pair(t + j * 3600, dt));
    }
    t -= 86400;
  }
  return t;
}
