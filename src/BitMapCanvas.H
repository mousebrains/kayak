#ifndef INC_BitMapCanvas_H_
#define INC_BitMapCanvas_H_

#include "Canvas.H"
#include <vector>
#include <map>
#include <string>

class BitMapCanvas : public Canvas {
private:
  MeasuredText fillText(const std::string& str, int x, int y, const bool qFill);
protected:
  typedef std::map<RGB, size_t> tColors;
  tColors mColors;

  typedef std::vector<int> tRow;
  typedef std::vector<tRow> tCanvas;
  tCanvas mCanvas;

  size_t color(const RGB& rgb);
  size_t color() {return color(mFillStyle);}

  void insert(int x, int y, int colorIndex);
public:
  BitMapCanvas(size_t width, size_t height);
  virtual ~BitMapCanvas() {}

  void lineTo(int x, int y);

  MeasuredText fillText(const std::string& str, int x, int y) {return fillText(str, x, y, true);}
  MeasuredText measureText(const std::string& str) const {
    return (const_cast<BitMapCanvas *>(this))->fillText(str, 0, 0, false);}
};

#endif // INC_BitMapCanvas_H_
