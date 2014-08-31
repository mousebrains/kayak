#include "Rating.H"
#include "Gauges.H"
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace {
  class MyFields : public DataFields {
  public:
    MyFields() : DataFields("rating") {}
    const char *flow() const {return "flow";}
    const char *gauge() const {return "gauge";}
  } fields;
}

Rating::Rating()
  : CommonData(fields)
{
}

Rating::~Rating()
{
}
