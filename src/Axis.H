#ifndef INC_Axis_H_
#define INC_Axis_H_

#include <set>
#include <vector>
#include <string>

class Canvas;

class Axis {
protected:
  Canvas& mCanvas;
  double mLHS;
  double mRHS;
  double mRange;
  const double mRho;
  const int mFontSize;
  const int mFontSizeMin;
  const int mLHSAxis;
  const int mRHSAxis;
  const int mWidthAxis;
  const int mLHSFrame;
  const int mRHSFrame;
  const bool mqLabels;

  typedef std::vector<double> tQ;
  tQ mQ;

  typedef std::vector<size_t> tFormats;
  tFormats mFormats;

  double myRound(const double x) const;
  double mkScore(const double simp, const double cov, const double dens, const double leg) const;
  double simplicity(const int i, const int j, const bool qZero) const;
  double density(const double lStep) const;
  double coverage(const double lLHS, const double lRHS) const;

  double firstTick(const double lStep, const bool qTight) const;
  double lastTick(const double lStep, const bool qTight) const;

  int transform(const double x, const double lhs, const double rhs) const;

  double overlapScore(const int distance, const int fs) const;

  struct Label {
    std::string label;
    double score;
    bool qSkip;
    Label(const std::string& str, const double sc, const bool q) : label(str), score(sc), qSkip(q){}
  };

  virtual Label mkLabel(const double x, const int iCase) const;

  virtual bool qZero(const double q0, const double lStep) const;

  void mkTicks(const bool qTight);

  struct LegInfo {
    double score;
    double lhs;
    double rhs;
    int fontSize;
    std::vector<double> ticks;
    std::vector<int> pos;
    std::vector<std::string> label;
    std::vector<int> labelPos;
    std::vector<int> width;
    LegInfo(const double lLHS, const double lRHS, const int fs);
  } mBest; // LegInfo

  virtual double stride(const double x, const double lStep) const;

  LegInfo legibility(const double lhs, const double rhs,
                     const double lLHS, const double lRHS, const double lStep) const;

  Axis(Canvas& canvas,
       const double lhs, const double rhs, const double rho,
       const double ppi, const int fontSize, 
       const int lhsAxis, const int rhsAxis,
       const int lhsFrame, const int rhsFrame);
public:
  virtual ~Axis() {}

  double score() const {return mBest.score;}
  std::vector<double> ticks() const {return mBest.ticks;}
  std::vector<int> pos() const {return mBest.pos;}
  std::vector<std::string> labels() const {return mBest.label;}
  std::vector<int> labelPos() const {return mBest.labelPos;}
  std::vector<int> width() const {return mBest.width;}
  int fontSize() const {return mBest.fontSize;}

  double lhs() const {return mBest.lhs;}
  double rhs() const {return mBest.rhs;}
}; // Axis

class yAxis : public Axis {
public:
  yAxis(Canvas& canvas,
        const double lhs, const double rhs, const double rho,
        const double ppi, const int fontSize,
        const int lhsAxis, const int rhsAxis,
        const int lhsFrame, const int rhsFrame);
}; // yAxis

class tAxis : public Axis {
private:
  time_t mT0;
  double mTimeNorm;
  struct tm mStartTime;

  std::vector<std::string> mTimeFormats;

  struct tm mkTime(const double x) const;

  Label mkLabel(const double x, const int iCase) const;
  bool qZero(const double, const double) const;
public:
  tAxis(Canvas& canvas,
        const double lhs, const double rhs, const double rho,
        const double ppi, const int fontSize,
        const int lhsAxis, const int rhsAxis,
        const int lhsFrame, const int rhsFrame);
}; // tAxis

#endif // INC_Axis_H_
