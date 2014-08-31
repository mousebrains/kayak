#include "Canvas.H"
#include <cmath>

Canvas::Canvas(size_t width, size_t height)
  : mWidth(width)
  , mHeight(height)
  , mFont(16, "Vera")
  , mTextAlign("left")
  , mTextBaseline("bottom")
{
}

void
Canvas::strokeRect(int x, 
                   int y, 
                   int width, 
                   int height)
{
  moveTo(x,y);
  lineTo(x + width, y);
  lineTo(x + width, y + height);
  lineTo(x, y + height);
  lineTo(x,y);
}

void
Canvas::restore() 
{
  if (!mTransforms.empty()) {
    mTransform = mTransforms.back();
    mTransforms.pop_back();
  }
}

Canvas::RGB::RGB()
  : mColor(0xffffff)
{
}

Canvas::RGB::RGB(const int red,
                 const int grn,
                 const int blu)
  : mColor(((red << 16) & 0xff0000) +
           ((grn <<  8) & 0x00ff00) +
           ( blu        & 0x0000ff))
{
}

int Canvas::RGB::red()   const { return (mColor >> 16) & 0xff; }
int Canvas::RGB::green() const { return (mColor >>  8) & 0xff; }
int Canvas::RGB::blue()  const { return  mColor        & 0xff; }

Canvas::RGB
Canvas::RGB::scale(const double a)
{
  return RGB(a * red(), a * green(), a * blue());
}

Canvas::Transform::Transform() 
  : m(6,0)
{
  m[0] = 1;
  m[3] = 1;
}

void
Canvas::Transform::rotate(const double theta)
{
  const double s(sin(theta));
  const double c(cos(theta));
  const double m11 =  c * m[0] + s * m[2];
  const double m12 =  c * m[1] + s * m[3];
  const double m21 = -s * m[0] + c * m[2];
  const double m22 = -s * m[1] + c * m[3];
  m[0] = m11;
  m[1] = m12;
  m[2] = m21;
  m[3] = m22;

}

void 
Canvas::Transform::scale(const double scaleX, 
                         const double scaleY) 
{
  m[0] *= scaleX;
  m[1] *= scaleX;
  m[2] *= scaleY;
  m[3] *= scaleY;
}

void
Canvas::Transform::translate(const double dx, 
                             const double dy) 
{
  m[4] += dx * m[0] + dy * m[2];
  m[5] += dx * m[1] + dy * m[3];
}

void Canvas::Transform::transform(const double m11, const double m12,
                                  const double m21, const double m22,
                                  const double dx, const double dy) 
{
  m[0] *= m11;
  m[1] *= m12;
  m[2] *= m21;
  m[3] *= m22;
  m[4] *= dx;
  m[5] *= dy;
}

void 
Canvas::Transform::setTransform(const double m11, const double m12,
                                const double m21, const double m22,
                                const double dx, const double dy) 
{
  m[0] = m11;
  m[1] = m12;
  m[2] = m21;
  m[3] = m22;
  m[4] = dx;
  m[5] = dy;
}

Canvas::Transform::tPoint
Canvas::Transform::transformPoint(const double x, 
                                  const double y) 
{
  const double xp(x * m[0] + y * m[2] + m[4]);
  const double yp(x * m[1] + y * m[3] + m[5]);

  return std::make_pair(xp, yp);
} 
