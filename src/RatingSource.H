#ifndef INC_RatingSource_H_
#define INC_RatingSource_H_

#include "CommonSource.H"

class RatingSource : public CommonSource {
public:
  RatingSource() : CommonSource("rating") {}
};

#endif // INC_RatingSource_H_
