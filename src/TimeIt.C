#include "TimeIt.H"
#include <iostream>

TimeIt::TimeIt()
{
  gettimeofday(&mStart, 0);
}

TimeIt&
TimeIt::reset()
{
  gettimeofday(&mStart, 0);
  return *this;
}

double
TimeIt::dt() const
{
  struct timeval t;
  gettimeofday(&t, 0);
  return (double) (t.tv_sec - mStart.tv_sec) + (double) (t.tv_usec - mStart.tv_usec) / 1e6;
}

std::ostream&
operator << (std::ostream& os,
             const TimeIt& t)
{
  os << t.dt();
  return os;
}
