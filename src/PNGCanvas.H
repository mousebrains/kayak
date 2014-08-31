#ifndef INC_PNGCanvas_H_
#define INC_PNGCanvas_H_

#include "BitMapCanvas.H"
#include <vector>
#include <map>
#include <string>

class PNGCanvas : public BitMapCanvas {
public:
  PNGCanvas(size_t width, size_t height);

  std::string toStr();
};

#endif // INC_PNGCanvas_H_
