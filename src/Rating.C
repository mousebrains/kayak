#include "Rating.H"
#include "Gauges.H"
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace {
  class MyFields : public CommonData::DataFields {
  public:
    MyFields() : DataFields("rating") {}
    const char *flow() const {return "flow";}
    const char *gauge() const {return "gauge";}
  } fields;
}

Rating::Rating()
  : CommonData(fields.table())
{
}

Rating::~Rating()
{
}
