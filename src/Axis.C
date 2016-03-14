#include "Axis.H"
#include "Canvas.H"
#include <cmath>
#include <cstdio>
#include <cstdlib>

class Seen {
private:
  struct MyPair {
    double first, second;
    MyPair(const double f, const double s) : first(f), second(s) {}
    bool operator < (const MyPair& rhs) const {
      return (first < rhs.first) || ((first == rhs.first) && (second < rhs.second));
    }
  };
  typedef std::set<MyPair> tSeen;
  tSeen mqSeen;
public:
  bool operator() (const double x, const double y) {
    MyPair a(x, y);
    if (mqSeen.find(a) != mqSeen.end())
      return true;
    mqSeen.insert(a);
    return false;
  }
}; // Seen

Axis::Axis(Canvas& canvas,
           const double lhs,
           const double rhs,
           const double rho,
           const double ppi,
           const int fontSize,
           const int lhsAxis,
           const int rhsAxis,
           const int lhsFrame,
           const int rhsFrame)
  : mCanvas(canvas)
  , mLHS(lhs)
  , mRHS(rhs)
  , mRange(std::abs(rhs - lhs))
  , mRho(rho / ppi)  // Convert from ticks/inch to ticks/pixel
  , mFontSize(fontSize)
  , mFontSizeMin(5 < fontSize ? 5 : fontSize)
  , mLHSAxis(lhsAxis)
  , mRHSAxis(rhsAxis)
  , mWidthAxis(mRHSAxis - mLHSAxis)
  , mLHSFrame(lhsFrame)
  , mRHSFrame(rhsFrame)
  , mqLabels(true)
  , mBest(lhs, rhs, fontSize)
{
  mQ = {1, 5, 2, 2.5, 4, 3};
  mFormats = {0,1,2};
}

yAxis::yAxis(Canvas& canvas,
             const double lhs,
             const double rhs,
             const double rho,
             const double ppi,
             const int fontSize,
             const int lhsAxis,
             const int rhsAxis,
             const int lhsFrame,
             const int rhsFrame)
  : Axis(canvas, lhs, rhs, rho, ppi, fontSize, 
         lhsAxis, rhsAxis, lhsFrame, rhsFrame)
{
  mkTicks(false);
}

tAxis::tAxis(Canvas& canvas,
             const double lhs,
             const double rhs,
             const double rho,
             const double ppi,
             const int fontSize,
             const int lhsAxis,
             const int rhsAxis,
             const int lhsFrame,
             const int rhsFrame)
  : Axis(canvas, lhs, rhs, rho, ppi, fontSize, 
         lhsAxis, rhsAxis, lhsFrame, rhsFrame)
  , mT0(0)
  , mTimeNorm(1)
  , mStartTime(mkTime(lhs))
{
  if (mRange > (4 * 31 * 86400)) { // > 4 months
      struct tm eTime(mkTime(rhs));
      mTimeNorm = -1; // 1 month steps
      mTimeFormats = {"%b-%Y", "%b-%y", "%m/%Y", "%m/%y"};
      mQ = {1, 2, 3, 4, 6};
      mLHS = 0;
      mRHS = (eTime.tm_year - mStartTime.tm_year) * 12 
           + (eTime.tm_mon - mStartTime.tm_mon) 
           + ((mStartTime.tm_mday != 1) || (mStartTime.tm_hour != 1) || 
              (mStartTime.tm_min != 1) || (mStartTime.tm_sec != 1));
      mStartTime.tm_mday = 1;
      mStartTime.tm_hour = 0;
      mStartTime.tm_min = 0;
      mStartTime.tm_sec = 0;
      mT0 = mktime(&mStartTime);
  } else { // <= 4 months so use time scaling
    if (mRange > (4 * 86400)) { // (4 days, 4 months]
      mTimeNorm = 86400; // 1 day steps
      mTimeFormats = {"%b-%d", "%m/%d"};
      mQ = {1, 7, 2, 3, 4};
      mStartTime.tm_hour = 0;
      mStartTime.tm_min = 0;
      mStartTime.tm_sec = 0;
    } else if (mRange > (4 * 3600)) { // (4 hours, 4 days]
      mTimeNorm = 3600; // 1 hour steps
      mTimeFormats = {"%b-%d %H:%M", "%m/%d %H:%M", "%d %H:%M", "%d %H"};
      mQ = {1, 6, 2, 4, 3};
      mStartTime.tm_min = 0;
      mStartTime.tm_sec = 0;
    } else if (mRange > (40 * 60)) { // (40 minutes, 4 hours]
      mTimeNorm = 600; // 10 minute steps
      mTimeFormats = {"%H:%M"};
      mQ = {1, 6, 2, 4, 3};
      mStartTime.tm_min -= mStartTime.tm_min % 10;
      mStartTime.tm_sec = 0;
    } else if (mRange > (4 * 60)) { // (4 minutes, 40 minutes]
      mTimeNorm = 60; // 1 minutes steps
      mTimeFormats = {"%H:%M:%S", "%M:%S"};
      mQ = {1, 5, 2, 4, 3};
      mStartTime.tm_sec = 0;
    } else { // < 4 minutes
      mTimeNorm = 1; // 1 second steps
      mTimeFormats = {"%H:%M:%S", "%M:%S"};
      mQ = {1,6,2,5,4,3};
    }
    mT0 = mktime(&mStartTime);
    mLHS = 0;
    mRHS = ceil((rhs - mT0) / mTimeNorm);
  }

  mRange = mRHS - mLHS;

  mFormats.clear();
  for (size_t i(0), e(mTimeFormats.size()); i < e; ++i) {
    mFormats.push_back(i);
  }

  mkTicks(true);

  // Adjust best lhs/rhs so caller will know what it is in time_t units
  mBest.lhs = mT0;
  struct tm etime(mkTime(mBest.rhs));
  mBest.rhs = mktime(&etime);
}

int
Axis::transform(const double x,
                const double lhs,
                const double rhs) const
{
  return round((x-lhs)/(rhs-lhs) * mWidthAxis) + mLHSAxis;
}

double
Axis::myRound(const double x) const
{
  return round(x * 10000) / 10000;
}

double
Axis::mkScore(const double simp,
              const double cov,
              const double dens,
              const double leg) const
{
  return 0.2 * simp + 0.25 * cov + 0.5 * dens + 0.05 * leg;
}

double
Axis::simplicity(const int i, 
                 const int j, 
                 const bool qZero) const
{
  return 1 - (double) i / (mQ.size() - 1) - (j + 1) + qZero;
}

double
Axis::density(const double lStep) const
{
  const double ratio(mRange / lStep / mWidthAxis / mRho);
  const double recip(1/ratio);

  return 2 - (recip > ratio ? recip : ratio);
}

double
Axis::coverage(const double lLHS,
               const double lRHS) const
{
  const double d0(mRHS - lRHS);
  const double d1(mLHS - lLHS);
  const double d2(0.1 * mRange);

  return 1 - (d0 * d0 + d1 * d1) / (d2 * d2) / 2;
}

double
Axis::overlapScore(const int distance,
                   const int fs) const
{
  const double threshold(round(0.5 * fs));
  return (distance >= threshold) ? 1 : (distance > 0) ? (2 - threshold / distance) : -INFINITY;
}

double
Axis::firstTick(const double lStep,
                const bool qTight) const
{
  const double a(qTight ? ceil(mLHS / lStep) : floor(mLHS / lStep));
  return myRound(a * lStep);
}

double
Axis::lastTick(const double lStep,
               const bool qTight) const
{
  const double a(qTight ? floor(mRHS / lStep) : ceil(mRHS / lStep));
  return myRound(a * lStep);
}

void
Axis::mkTicks(const bool qTight)
{
  const size_t nQ(mQ.size());
  const double stepNorm(pow(10, round(log10(mRange)) - 1));
  std::vector<int> offsetIndices;
  Seen qSeen;
  mBest = LegInfo(mLHS, mRHS, mFontSize);

  for (size_t j(0); j < 10; ++j) {
    offsetIndices.push_back(j);
    for (size_t i(0); i < nQ; ++i) {
      const double q(mQ[i]);
      const double denom(pow(10, floor(log10((j+1) * q)))); 
      const double lStep((j+1) * q / denom * stepNorm);
      for (size_t k(0); k <= j; ++k) {
        const double q0(q * q * offsetIndices[k] / denom);
        if (qSeen(lStep, q0)) { continue; }
        const double simp(simplicity(i, j, qZero(q0, lStep)));
        if (mkScore(simp, 1, 1, 1) <= mBest.score) { break; }
        const double dens(density(lStep));
        if (mkScore(simp, 1, dens, 1) <= mBest.score) { break; }
        const double lLHS(firstTick(lStep, qTight));
        const double lRHS(lastTick(lStep, qTight));
        const double cov(coverage(lLHS, lRHS));
        if (mkScore(simp, cov, dens, 1) <= mBest.score) { break; }
        const LegInfo leg(legibility((qTight ? mLHS : lLHS), (qTight ? mRHS : lRHS),
                                     lLHS, lRHS, lStep));
        const double score(mkScore(simp, cov, dens, leg.score));
        if (score <= mBest.score) { break; }
        mBest = leg;
        mBest.score = score;
      } // for k
    } // for i
  } // for j
} // mkTicks

Axis::LegInfo::LegInfo(const double lLHS,
                       const double lRHS,
                       const int fs)
  : score(-INFINITY)
  , lhs(lLHS)
  , rhs(lRHS)
  , fontSize(fs)
{
}

Axis::LegInfo
Axis::legibility(const double lhs,
                 const double rhs,
                 const double lLHS,
                 const double lRHS,
                 const double lStep) const
{
  LegInfo info(lhs, rhs, mFontSize);
  const double lMin(lLHS < lRHS ? lLHS : lRHS);
  const double lMax(lLHS > lRHS ? lLHS : lRHS);
  bool qZeroExtended(false);

  for (double tick(lMin); tick <= lMax; tick = stride(tick, lStep)) {
    info.ticks.push_back(tick);
    info.pos.push_back(transform(tick, lhs, rhs));
    qZeroExtended |= tick == 0;
  }

  if (!mqLabels) {
    info.score = 1;
    return info;
  }

  const size_t nTicks(info.ticks.size());

  for (tFormats::size_type i(0), e(mFormats.size()); i < e; ++i) {
    const size_t iFormat(mFormats[i]);
    double netScore(0);
    std::vector<std::string> labels;
    std::set<std::string> seen;
    bool qDuplicate(false);
    bool qSkip(false);
    for (size_t j(0); j < nTicks; ++j) {
      const Label label(mkLabel(info.ticks[j], iFormat));
      if (seen.find(label.label) != seen.end()) { // A duplicate label, so stop
        qDuplicate = true;
        break;
      }
      seen.insert(label.label);
      labels.push_back(label.label);
      netScore += label.score;
      qSkip |= label.qSkip;
    } // for j
    if (qSkip && qDuplicate) { // rest of formats will cause duplicates
      break;
    }

    netScore *= 0.9 / nTicks;
    netScore += 0.1 * qZeroExtended;
    if (((netScore + 3) / 4) <= info.score) { continue; } 
   
    for (int fs(mFontSize); fs >= mFontSizeMin; --fs) {
      const double fsScore(fs >= mFontSize ? 
                           1 : 0.8 * (fs - mFontSizeMin) / (mFontSize - mFontSizeMin)); 
      if (((netScore + fsScore + 2) / 4) <= info.score) { break; }
      std::vector<int> width;
      std::vector<int> labelPos;
      double oScore(INFINITY);
      mCanvas.font(fs, mCanvas.fontFace());
      for (size_t j(0); j < nTicks; ++j) {
        width.push_back(mCanvas.measureText(labels[j]).width);
        labelPos.push_back(info.pos[j]);
        if (j == 0) { // first tick, adjust to be inside the plot box
          const int distance((mRHSFrame > mLHSFrame ? 1 : -1) *
                             round(std::abs(labelPos[j] - mLHSFrame) - width[j]/2));
          labelPos[j] -= distance < 0 ? 0 : distance;
        } // first tick
        if (j == (nTicks - 1)) { // Last tick, pull label inside box
          const int distance((mRHSFrame > mLHSFrame ? 1 : -1) *
                             round(std::abs(labelPos[j] - mRHSFrame) - width[j]/2));
          labelPos[j] += distance < 0 ? 0 : distance;
        } // last tick
        if (j > 0) { // check overlap between labels
          const int distance(round(std::abs(labelPos[j-1] - labelPos[j]) -
                                   (width[j-1] + width[j]) / 2));
          const double sc(overlapScore(distance, fs));
          oScore =  oScore < sc ? oScore : sc;
        } // not first tick
      } // for j
      const double score((netScore + fsScore + oScore + 1) / 4);
      if (score <= info.score) { continue; }
      info.score = score;
      info.fontSize = fs;
      info.label = labels;
      info.labelPos = labelPos;
      info.width = width;
    } // for fs
  } // for i
 
  return info;
}

double
Axis::stride(const double x,
             const double lStep) const
{
  return myRound(x + lStep);
}

Axis::Label
Axis::mkLabel(const double x,
              const int iCase) const
{
  if (x == 0) {
    return Label("0", 1, false);
  }
  const double ax(std::abs(x));
  const char *format("%g");
  double score(ax >= 1e-4 && ax < 1e6);

  switch (iCase) {
    case 0: break; // The defaults
    case 1: format = "%e"; score = 0.25; break;
    case 2:
      format = (ax >= 1e3) && (ax < 1e6) ? "%.0fK" :
               (ax >= 1e6) && (ax < 1e9) ? "%.0fM" :
               (ax >= 1e9) && (ax < 1e12) ? "%.0fG" :
               0;
      score = 0.75;
      break;
  }

  if (!format) {
    return Label("", -INFINITY, false);
  }

  char buffer[1024];
  snprintf(buffer, sizeof(buffer), format, x);
  return Label(buffer, score, false);
}

bool
Axis::qZero(const double q0, 
            const double lStep) const
{ // 0 = q0 + n lStep, true if n is an integer else false
  return myRound(round(-q0 / lStep) * lStep + q0) == 0;
}

tAxis::Label
tAxis::mkLabel(const double x,
               const int iCase) const
{
  const struct tm t(mkTime(x));
  char buffer[1024];
  strftime(buffer, sizeof(buffer), mTimeFormats[iCase].c_str(), &t);

  return Label(buffer, 1. - (double) iCase / mTimeFormats.size(), false);
}

bool
tAxis::qZero(const double , 
             const double ) const
{
  return false;
}

struct tm
tAxis::mkTime(const double x) const
{
  if (mTimeNorm > 0) { // simple normalization
    const time_t t(x * mTimeNorm + mT0);
    struct tm a;
    localtime_r(&t, &a);
    return a;
  }

  // Step by months 
  struct tm a(mStartTime);
  const int wholeMonths(floor(std::abs(x))); // Whole months to step
  int weeks(round((std::abs(x) - wholeMonths) * 4)); // weeks to step

  if (x < 0) { // Step backwards in months
    a.tm_mon -= wholeMonths; // backwards appropriate number of months;
    a.tm_year -= (int) (floor((double) std::abs(a.tm_mon) / 12)); // backup up number of years
  } else { // Step forwards in months
    a.tm_mon += wholeMonths; // Advance appropriate number of months
    a.tm_year += (int) floor((double) a.tm_mon / 12); // Advance appropriate number of years
  }
  a.tm_mon = (a.tm_mon < 0 ? -a.tm_mon : a.tm_mon) % 12; // Take into account year movement
  a.tm_mday = 1 + weeks * 7; // Move to week number within month, 1st, 8th, 15th, 22nd.
  a.tm_hour = 0;
  a.tm_min = 0;
  a.tm_sec = 0;

  return a;
}
