#ifndef INC_TimeIt_H_
#define INC_TimeIt_H_

#include <iosfwd>
#include <sys/time.h>

class TimeIt {
private:
  struct timeval mStart;
public:
  TimeIt();
  TimeIt& reset();
  double dt() const; 
}; // TimeIt

std::ostream& operator << (std::ostream& os, const TimeIt& t);

#endif // INC_TimeIt_H_
