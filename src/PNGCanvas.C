#include "PNGCanvas.H"
#include "Paths.H"
#include <sstream>
#include <exception>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H
// #include <iostream>

namespace {
  struct MyException : public std::exception {
    std::string mReason;
    MyException(const std::string& str) : mReason(str) {}
    ~MyException() throw () {}
    const char *what() const throw() {return mReason.c_str();}
  };

  std::string pngErrMsg("Setting jump"); // used when setjmp is active
  const size_t pngCompressionLevel(7); // compression level for png

  void pngUserWriteData(png_structp png_ptr, png_bytep data, png_size_t length) {
    std::ostream *os((std::ostream *) png_get_io_ptr(png_ptr));
    os->write((const char *) data, (size_t) length);
  }

  void pngUserFlushData(png_structp png_ptr) {
  }
} // anonymous

PNGCanvas::PNGCanvas(size_t width,
                     size_t height)
  : BitMapCanvas(width, height)
{
}

std::string
PNGCanvas::toStr()
{
  png_struct *png_ptr(png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0));

  if (!png_ptr) {
    throw MyException("Error creating png write struct");
  }

  pngErrMsg = "Setting jump";

  if (setjmp(png_jmpbuf(png_ptr))) {
    throw MyException(pngErrMsg);
  }

  pngErrMsg = "Creating info structure";
  png_info *info_ptr(png_create_info_struct(png_ptr));
  if (!info_ptr) {
    throw MyException(pngErrMsg);
  }

  { // Setup the palette
    png_color *rgb(new png_color[mColors.size()]);
    memset(rgb, 0, sizeof(png_color) * mColors.size());
    for (tColors::const_iterator it(mColors.begin()), et(mColors.end()); it != et; ++it) {
      const size_t i(it->second);
      const RGB& a(it->first);
      rgb[i].red = a.red(); // Invert
      rgb[i].green = a.green(); // Invert
      rgb[i].blue = a.blue(); // Invert
    }
    pngErrMsg = "While setting palette";
    png_set_PLTE(png_ptr, info_ptr, rgb, mColors.size());
    delete rgb;
  }

  std::stringstream oss;

  pngErrMsg = "set write fn";
  png_set_write_fn(png_ptr, &oss, pngUserWriteData, pngUserFlushData);

  pngErrMsg = "While setting header";
  png_set_IHDR(png_ptr, info_ptr, mWidth, mHeight, RGB::bitDepth(),
               PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
               PNG_FILTER_TYPE_BASE);

  pngErrMsg = "While setting compression level";
  png_set_compression_level(png_ptr, pngCompressionLevel);

  pngErrMsg = "While writing header";
  png_write_info(png_ptr, info_ptr);

  { // write out pixels
    pngErrMsg = "While writing image";
    unsigned char *ptr(new unsigned char[mWidth]);
    for (tCanvas::size_type i(0), e(mCanvas.size()); i < e; ++i) {
      const tRow& row(mCanvas[i]);
      memset(ptr, 0, sizeof(unsigned char) * mWidth);
      for (tRow::size_type j(0), je(row.size()); j < je; ++j) {
        ptr[j] = row[j];
      }
      png_write_row(png_ptr, ptr);
    }
    delete ptr;
  }

  pngErrMsg = "While writing end";
  png_write_end(png_ptr, 0);
  
  return oss.str();
}
