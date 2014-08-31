#include "MakePlot.H"
#include "Axis.H"
#include "Canvas.H"
#include "DataGet.H"
#include "Convert.H"
#include <cmath>
#include <cstdio>

namespace {
  void rotateText(Canvas& canvas, const std::string& str, const int x, const int y) {
    canvas.save();
    canvas.setTransform(0,-1,1,0,x,y);
    canvas.fillText(str, 0, 0);
    canvas.restore();
  }
} // anonymous

bool
MakePlot(Canvas& canvas,
         const DataGet& obs)
{
  const time_t now(time(0));

  if (obs.empty()) return false;

  time_t tMin(obs.begin()->first);
  time_t tMax(obs.rbegin()->first);
  double min(1e300), max(-min);

  for (DataGet::const_iterator it(obs.begin()), et(obs.end()); it != et; ++it) {
    min = min <= it->second ? min : it->second;
    max = max >= it->second ? max : it->second;
  }

  if (tMin == tMax) {
    tMin -= 1800; 
    tMax += 1800; 
  }

  if (min == max) {
    min -= 0.5;
    max += 0.5;
  }


  const size_t nWidth(canvas.width());
  const size_t nHeight(canvas.height());
  const size_t nMin(nWidth < nHeight ? nWidth : nHeight);
  const size_t nLeft(ceil(nMin * 0.1));
  const size_t nRight(nWidth - nLeft);
  const size_t nTop(ceil(nHeight * 0.075));
  const size_t nBot(nHeight - nLeft); // Same as left
  const size_t titleFontSize(ceil(nTop * 0.65));
  const size_t labelFontSize(ceil(nLeft * 0.40));
  const std::string fontFamily("Vera");

  const double rho(1); // 1 label per inch
  const double ppi(96); // 96 pixels per inch
  tAxis taxis(canvas, tMin, tMax, rho, ppi, labelFontSize, nLeft, nRight, nWidth, 0);
  const bool qAdjustYBot(!taxis.labelPos().empty() &&
                         ((taxis.labelPos()[0] - taxis.width()[0] / 2) < (int) nLeft));
  yAxis yaxis(canvas, min, max, rho, ppi, taxis.fontSize(), nBot, nTop, 
              qAdjustYBot ? nBot : nHeight, 0);

  canvas.fillStyle(Canvas::RGB(0,0,0));
  canvas.textAlign("middle");
  canvas.textBaseline("center");
  canvas.font(titleFontSize, fontFamily);
  canvas.fillText(obs.title(), (nLeft + nRight) / 2, nTop/2);

  canvas.textBaseline("bottom");
  canvas.font(labelFontSize, fontFamily);
  canvas.fillText("Generated on " + Convert::toStr(now, "%d-%b-%Y %H:%M:%S"), 
                  (nLeft + nRight)/2, nHeight - canvas.fontSize()/3);

  canvas.textAlign("middle");
  canvas.textBaseline("top");
  rotateText(canvas, obs.ylabel() + " (" + obs.units() + ")", 0, (nTop+nBot)/2);

  canvas.font(yaxis.fontSize(), fontFamily);
  for (size_t i(0), ie(taxis.labels().size()); i < ie; ++i) { // Draw t labels
    canvas.fillText(taxis.labels()[i], taxis.labelPos()[i], nBot+2);
  }

  canvas.textBaseline("bottom");
  for (size_t i(0), ie(yaxis.labels().size()); i < ie; ++i) { // Draw y labels
    rotateText(canvas, yaxis.labels()[i], nLeft-2, yaxis.labelPos()[i]);
  }

  canvas.fillStyle(Canvas::RGB(0xc0,0xc0,0xc0));

  for (size_t i(0), ie(taxis.pos().size()); i < ie; ++i) { // Draw t ticks
    canvas.moveTo(taxis.pos()[i], nBot);  
    canvas.lineTo(taxis.pos()[i], nTop);  
  }

  for (size_t i(0), ie(yaxis.pos().size()); i < ie; ++i) { // Draw y ticks
    canvas.moveTo(nLeft, yaxis.pos()[i]);  
    canvas.lineTo(nRight, yaxis.pos()[i]);  
  }

  canvas.fillStyle(Canvas::RGB(0,0,0));
  canvas.strokeRect(nLeft, nTop, nRight - nLeft, nBot - nTop);

  const double tLHS(taxis.lhs());
  const double yLHS(yaxis.lhs());
  const double dx(taxis.rhs() - tLHS);
  const double dy(yaxis.rhs() - yLHS);
  const double dWidth(nRight - nLeft); // Width in pixels of plotting area
  const double dHeight(nBot - nTop); // Height in pixels of plotting area
  bool qFirst(true);
  canvas.fillStyle(Canvas::RGB(0,0,255));
  for (DataGet::const_iterator it(obs.begin()), et(obs.end()); it != et; ++it) {
    const int ix(nLeft + dWidth * (it->first - tLHS) / dx);
    const int iy(nBot - dHeight * (it->second - yLHS) / dy);
    if (qFirst) {
      canvas.moveTo(ix, iy);
      qFirst = false;
    } else {
      canvas.lineTo(ix, iy);
    }
  }

  if (obs.low() != 0) { // low level given, so draw a horizontal line corresponding to it
    const int iy(round(nBot - dHeight * (obs.low() - yLHS) / dy));
    canvas.fillStyle(Canvas::RGB(218,165,32)); // Goldenrod
    canvas.moveTo(nLeft, iy);
    canvas.lineTo(nRight, iy);
  }

  if (obs.high() != 0) { // high level given, so draw a horizontal line corresponding to it
    const int iy(round(nBot - dHeight * (obs.low() - yLHS) / dy));
    canvas.fillStyle(Canvas::RGB(255,0,0)); // red
    canvas.moveTo(nLeft, iy);
    canvas.lineTo(nRight, iy);
  }

  return true;
}
