#ifndef INC_Canvas_H_
#define INC_Canvas_H_

// Abstract class modeled on the HTML5 Canvas interface

#include <string>
#include <vector>

class Canvas {
public:
  class RGB {
  private:
    int mColor;
  public:
    RGB();
    RGB(const int red, const int green, const int blue);
    int red() const;
    int green() const;
    int blue() const;
    RGB scale(const double a);
    static size_t bitDepth() {return 8;} // 8 bit colors
    bool operator < (const RGB& rhs) const {return mColor < rhs.mColor;}
  };
protected:
  class Font {
  private:
    size_t mSize;
    std::string mFace;
  public:
    Font(const size_t sz, const std::string& face) : mSize(sz), mFace(face) {}
    size_t size() const {return mSize;}
    const std::string& face() const {return mFace;}

    bool operator < (const Font& rhs) const {
      return (mSize < rhs.mSize) || ((mSize == rhs.mSize) && (mFace < rhs.mFace));
    }
  };

  class Transform {
  private:
    std::vector<double> m;
  public:
    Transform();
    void rotate(const double theta);
    void scale(const double scaleX, const double scaleY);
    void translate(const double dx, const double dy);
    void transform(const double m11, const double m12, 
                   const double m21, const double m22,
                   const double dx, const double dy);
    void setTransform(const double m11, const double m12, 
                      const double m21, const double m22,
                      const double dx, const double dy);

    typedef std::pair<double, double> tPoint;
    tPoint transformPoint(const double x, const double y);
  };

  Transform mTransform;
  typedef std::vector<Transform> tTransforms;
  tTransforms mTransforms;

  size_t mWidth;
  size_t mHeight;
  int mxPos, myPos;
  
  RGB mFillStyle;
  Font mFont;
  std::string mTextAlign;
  std::string mTextBaseline;

  Canvas(size_t width, size_t height);
public:
  virtual ~Canvas() {}

  size_t width() const {return mWidth;}
  size_t height() const {return mHeight;}

  size_t fontSize() const {return mFont.size();}
  const std::string& fontFace() const {return mFont.face();}

  virtual void fillStyle(RGB style) {mFillStyle = style;}
  virtual RGB fillStyle() const {return mFillStyle;}

  // virtual void fillRect(int x, int y, int width, int height) = 0;
  // virtual void clrRect(int x, int y, int width, int height) = 0;
  virtual void strokeRect(int x, int y, int width, int height);

  // virtual arc(int x, int y, int r, double thetaStart, double thetaStop) = 0;

  virtual void moveTo(int x, int y) {mxPos = x; myPos = y;}
  virtual void lineTo(int x, int y) = 0;

  virtual void beginPath() {} 
  virtual void stroke() {}

  virtual void font(const size_t sz, const std::string& face) {mFont = Font(sz, face);}
  virtual void textAlign(const std::string& str) {mTextAlign = str;}
  virtual void textBaseline(const std::string& str) {mTextBaseline = str;}

  struct MeasuredText {
    size_t width;
    size_t height;
    MeasuredText() : width(0), height(0) {}
  };
  
  virtual MeasuredText fillText(const std::string& str, int x, int y) = 0;
  // virtual MeasuredText strokeText(const std::string& str, int x, int y) = 0;

  virtual MeasuredText measureText(const std::string& str) const = 0;

  virtual void rotate(const double theta) {mTransform.rotate(theta);}
  virtual void scale(const double scaleX, const double scaleY) {mTransform.scale(scaleX, scaleY);}
  virtual void translate(const double dx, const double dy) {mTransform.translate(dx, dy);}
  virtual void transform(const double scaleX, const double skewX, 
                         const double skewY, const double scaleY,
                         const double dx, const double dy) {
    mTransform.transform(scaleX, skewX, skewY, scaleY, dx, dy);
  }

  virtual void setTransform(const double scaleX, const double skewX, 
                            const double skewY, const double scaleY,
                            const double dx, const double dy) {
    mTransform.setTransform(scaleX, skewX, skewY, scaleY, dx, dy);
  }

  virtual void save() {mTransforms.push_back(mTransform);} // save current context
  virtual void restore(); // restore context from previous save
}; // Canvas

#endif // INC_Canvas_H_
