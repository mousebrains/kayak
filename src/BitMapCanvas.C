#include "BitMapCanvas.H"
#include "Paths.H"
#include "Convert.H"
#include "MyException.H"
#include <sstream>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace {
  void ftErrChk(FT_Error err, const std::string& msg) {
    if (!err) return;
    std::ostringstream oss;
    oss << "Error: " << msg << ", " << err;
    throw MyException(oss.str());
    }
} // anonymous

BitMapCanvas::BitMapCanvas(size_t width,
                           size_t height)
  : Canvas(width, height)
  , mCanvas(height, tRow(width, color()))
{
}

size_t
BitMapCanvas::color(const RGB& rgb)
{
  tColors::const_iterator it(mColors.find(rgb));

  return it == mColors.end() ?
         mColors.insert(std::make_pair(rgb, mColors.size())).first->second :
         it->second;
}

void
BitMapCanvas::lineTo(int x,
                     int y)
{
  const int dx(x < mxPos ? (mxPos - x) : (x - mxPos));
  const int dy(y < myPos ? (myPos - y) : (y - myPos));

  if (!dx && !dy) { // nothing to do
    return;
  }

  const int colorIndex(color());

  if (dx > dy) { // step in x
    const double slope((double) (y - myPos) / (double) (x - mxPos));
    const int delta(x < mxPos ? 1 : -1);
    for (int i(x); i != mxPos; i += delta) {
      insert(i, (int) (slope * (i - x) + y), colorIndex);
    }
  } else { // step in y
    const double slope((double) (x - mxPos) / (double) (y - myPos));
    const int delta(y < myPos ? 1 : -1);
    for (int i(y); i != myPos; i += delta) {
      insert((int) (slope * (i - y) + x), i, colorIndex);
    }
  }

  mxPos = x;
  myPos = y;
}

void
BitMapCanvas::insert(int x,
                     int y,
                     int colorIndex)
{
  Transform::tPoint p(mTransform.transformPoint(x, y));
  x = p.first;
  y = p.second;
  if ((x >= 0) && (x < (int) mWidth) && (y >= 0) && (y < (int) mHeight)) {
    mCanvas[y][x] = colorIndex;
  }
}

BitMapCanvas::MeasuredText
BitMapCanvas::fillText(const std::string& str,
                       int x,
                       int y,
                       const bool qFill)
{
  static FT_Library library;
  static bool qInitialize(true);
  if (qInitialize) {
    qInitialize = false;
    ftErrChk(FT_Init_FreeType(&library), "Initializing freetype library");
  }

  // Find the face if it already exists, if not, then build it and save it
  FT_Face face(0);
  typedef std::map<Font, FT_Face> tFaces;
  static tFaces faces;
  tFaces::const_iterator it(faces.find(mFont));
  if (it == faces.end()) { // Does not exist, so build it
    const std::string font(Paths::font());
    const size_t xDPI(96), yDPI(xDPI); // web default 96 dpi
    const size_t xSize(mFont.size() * 64), ySize(xSize); // in 26.6 units i.e. *65
    ftErrChk(FT_New_Face(library, font.c_str(), 0, &face), "New font face, " + font);
    ftErrChk(FT_Set_Char_Size(face, xSize, ySize, xDPI, yDPI), "Set font size, " + font); 
    faces.insert(std::make_pair(mFont, face));
  } else {
    face = it->second;
  }

  const bool qKerning(FT_HAS_KERNING(face));

  MeasuredText info;
  FT_UInt prevGlyphIndex(0);

  for (std::string::size_type i(0), e(str.size()); i < e; ++i) {
    const FT_UInt glyphIndex(FT_Get_Char_Index(face, str[i]));
    if (!glyphIndex) {
      throw MyException("Error: Getting glyphIndex for '" + str + "', index " + Convert::toStr(i));
    }
    ftErrChk(FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT), "Loading glyph");
    FT_GlyphSlot slot(face->glyph);

    if (qKerning && prevGlyphIndex) {
      FT_Vector delta;
      FT_Get_Kerning(face, prevGlyphIndex, glyphIndex, FT_KERNING_DEFAULT, &delta);
      info.width += delta.x >> 6;
      info.height += delta.y >> 6;
    }

    info.width  += slot->advance.x >> 6;
    info.height += slot->advance.y >> 6; // Should be zero

    prevGlyphIndex = glyphIndex;
  }

  if (info.height == 0) {
    info.height = mFont.size();
  }

  if (!qFill) {
    return info;
  }

  if (mTextAlign == "middle" || mTextAlign == "center") {
    x -= info.width / 2;
  } else if (mTextAlign == "right") {
    x -= info.width;
  }

  if (mTextBaseline == "center") {
    y += info.height / 2;
  } else if (mTextBaseline == "top") {
    y += info.height;
  }

  { // Draw text
    int colorIndex(color());
    FT_UInt prevGlyphIndex(0);
    for (std::string::size_type i(0), e(str.size()); i < e; ++i) {
      const FT_UInt glyphIndex(FT_Get_Char_Index(face, str[i]));
      if (!glyphIndex) {
        throw MyException("Error: Getting glyphIndex for '" + str + "'");
      }
      ftErrChk(FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT), "Loading glyph");
      FT_GlyphSlot slot(face->glyph);
      ftErrChk(FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL), "Rendering glyph");
      if (qKerning && prevGlyphIndex) {
        FT_Vector delta;
        FT_Get_Kerning(face, prevGlyphIndex, glyphIndex, FT_KERNING_DEFAULT, &delta);
        x += delta.x >> 6;
        y += delta.y >> 6;
      }
      prevGlyphIndex = glyphIndex;
      const FT_Bitmap& bitmap(slot->bitmap);
      unsigned char *ptr(bitmap.buffer);
      for (int r(0), re(bitmap.rows); r < re; ++r) {
        for (int c(0), ce(bitmap.width); c < ce; ++c) {
          if (!ptr[c]) continue; // Color is zero, so do nothing
          const int dx(c + slot->bitmap_left - 1);
          const int dy(r - slot->bitmap_top + 1);
          const int ci(ptr[c] == 255 ? colorIndex : color(mFillStyle.scale((double) ptr[c] / 255)));
          insert(x + dx, y + dy, ci); 
        } // for c 
        ptr += bitmap.width;
      } // for r
      x += slot->advance.x >> 6;
      y += slot->advance.y >> 6;
    } // for i 
  }
  return info; 
}
