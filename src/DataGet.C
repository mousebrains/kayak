#include "DataGet.H"
#include "Master.H"
#include "Gauges.H"
#include "Data.H"
#include "Convert.H"
#include "CGI.H"
#include "JSON.H"
#include <sstream>
#include <cmath>
#include <algorithm>

namespace {
  const CGI cgi;
} // anonymous

DataGet::DataGet(const bool qOne)
  : mHash(cgi.get("h", std::string()))
  , mKey(Master::deHash(mHash))
  , mGaugeKey(0)
  , mLow(0)
  , mHigh(0)
  , mLowOptimal(0)
  , mHighOptimal(0)
  , mLatitude(1e9)
  , mLongitude(1e9)
{
  if (mHash.empty()) { // No hash
    mMsg = "Hash not set";
    return;
  }

  const std::string type(cgi.get("t", "flow"));
  const time_t now(time(0));
  const time_t t1(cgi.isSet("edate") ? Convert::toTime(cgi.get("edate")) : now);
  const time_t t0(cgi.isSet("sdate") ? Convert::toTime(cgi.get("sdate")) : t1 - 14 * 86400);

  mType = Data::decodeType(type);

  if (mType == Data::LASTTYPE) {
    mMsg = "Invalid type '" + type + "'";
    return;
  }

  fetch(t0 < t1 ? t0 : t1, t0 < t1 ? t1 : t0, qOne);
}

void
DataGet::fetch(time_t stime,
               time_t etime,
               bool qOne)
{
  Master::PlotInfo pi(Master().plotInfo(mKey, mType));

  if (pi.gaugeKey == 0) {
    mMsg = "No gauge found for key " + Convert::toStr(mKey);
    return;
  }

  mGaugeKey = pi.gaugeKey;

  Data data;
  mSourceKeys = data.source().gaugeKey2Keys(pi.gaugeKey);

  if (mSourceKeys.empty()) {
    mMsg = "No data sources found for key " + Convert::toStr(mKey) 
         + " which is gauge key " + Convert::toStr(pi.gaugeKey);
    return;
  }

  mTypes = data.types(mSourceKeys); // Which data types are known for this gauge key

  if (mTypes.empty()) { // No data for this gauge
    mMsg = "No data for " + pi.name;
    return;
  }

  if (std::find(mTypes.begin(), mTypes.end(), mType) == mTypes.end()) { 
    mMsg = "Data type, " + Convert::toStr(mType) + ", is not known for gauge " + pi.name;
    return;
  }

  { // round stime back to midnight
    struct tm *ptr(localtime(&stime));
    ptr->tm_hour = 0;
    ptr->tm_min = 0;
    ptr->tm_sec = 0;
    stime = mktime(ptr);
  }
  { // round etime forward to midnight
    struct tm *ptr(localtime(&etime));
    ptr->tm_hour = 23;
    ptr->tm_min = 59;
    ptr->tm_sec = 59;
    etime = mktime(ptr);
  }

  if (qOne) {
    mObs = data.observations(mSourceKeys, mType, stime, etime);

    if (mObs.empty()) {
      std::ostringstream oss;
      oss << "No data sources found for key " << mKey << " which is gauge key " << pi.gaugeKey
          << " of type " << mType;
      mMsg = oss.str();
      return;
    }
  } else { // !qOne
    mAnObs = data.observations(mSourceKeys, stime, etime);

    if (mAnObs.empty()) {
      std::ostringstream oss;
      oss << "No data sources found for key " << mKey << " which is gauge key " << pi.gaugeKey
          << " of any type";
      mMsg = oss.str();
      return;
    }
  } // if qOne

  if (pi.name.empty() || pi.location.empty() ||
      (fabs(pi.latitude) > 90) || (fabs(pi.longitude) > 180)) {
    Gauges gauges;
    const Gauges::PlotInfo gi(gauges.plotInfo(pi.gaugeKey));
    if (pi.name.empty()) {
      pi.name = gi.name;
    }
    if (pi.location.empty()) {
      pi.location = gi.location;
    }
    if ((fabs(pi.latitude) > 90) || (fabs(pi.longitude) > 180)) {
      pi.latitude = gi.latitude;
      pi.longitude = gi.longitude;
    }
  }

  mTitle = pi.name + (pi.location.empty() ? std::string() : (" " + pi.location));
  mYLabel = mkLabel(mType);
  mUnits = mkUnits(mType);
  mLow = pi.low;
  mHigh = pi.high;
  mLowOptimal = pi.lowOptimal;
  mHighOptimal = pi.highOptimal;
  mLatitude = pi.latitude;
  mLongitude = pi.longitude;
}

std::string
DataGet::mkLabel(const Data::Type type) const
{
  switch (type) {
    case Data::INFLOW:
    case Data::FLOW: return "Flow";
    case Data::GAUGE: return "Gauge";
    case Data::TEMPERATURE: return "Temp";
    case Data::LASTTYPE: break;
  }
  return "???";
}

std::string
DataGet::mkUnits(const Data::Type type) const
{
  switch (type) {
    case Data::INFLOW:
    case Data::FLOW: return "CFS";
    case Data::GAUGE: return "Feet";
    case Data::TEMPERATURE: return "F";
    case Data::LASTTYPE: break;
  }
  return "???";
}

std::string
DataGet::json() const
{
  std::ostringstream oss;
  json(oss);
  return oss.str();
}

std::ostream& 
DataGet::json(std::ostream& os) const
{
  return mAnObs.empty() ? jsonObs(os) : jsonAnObs(os);
}

std::ostream&
DataGet::jsonObs(std::ostream& os) const
{
  if (!mMsg.empty()) {
    os << "{error:'" << errmsg() << "'}";
    return os;
  }

  os << "{title:'" << title()
     << "',tlabel:'Date" 
     << "',ylabel:'" << ylabel()
     << "',yunits:'" << units()
     << "',type:'" << mType
     << "',hash:'" << mHash << "'";

  if (!isnan(low())) {
    os << ",low:" << low();
  }
  if (!isnan(high())) {
    os << ",high:" << high();
  }

  if (!isnan(lowOptimal())) {
    os << ",lowOptimal:" << lowOptimal();
  }
  if (!isnan(highOptimal())) {
    os << ",highOptimal:" << highOptimal();
  }

  { // Spit out t
    JSON::Array<time_t> dd("t");
    for (const_iterator it(begin()), et(end()); it != et; ++it) {
      dd(it->first);
    }
    os << "," << dd;
  }

  { // Spit out y
    JSON::Array<double> dd("y");
    for (const_iterator it(begin()), et(end()); it != et; ++it) {
      dd(roundY(it->second));
    }
    os << "," << dd;
  }
  os << "}";
  return os;
} // jsonObs

std::ostream&
DataGet::jsonAnObs(std::ostream& os) const
{
  std::vector<double> yMin(3, INFINITY);
  std::vector<double> yMax(3, -INFINITY);
  bool qName(false);
  bool qURL(false);


  for (tAnObs::const_iterator it(mAnObs.begin()), et(mAnObs.end()); it != et; ++it) {
    qName |= !it->name.empty();
    qURL |= !it->url.empty();

    yMin[0] = yMin[0] < it->flow        ? yMin[0] : it->flow;
    yMin[1] = yMin[1] < it->gauge       ? yMin[1] : it->gauge;
    yMin[2] = yMin[2] < it->temperature ? yMin[2] : it->temperature;
    yMax[0] = yMax[0] > it->flow        ? yMax[0] : it->flow;
    yMax[1] = yMax[1] > it->gauge       ? yMax[1] : it->gauge;
    yMax[2] = yMax[2] > it->temperature ? yMax[2] : it->temperature;
  }

  yMin[0] = roundY(yMin[0], Data::FLOW);
  yMax[0] = roundY(yMax[0], Data::FLOW);
  yMin[1] = roundY(yMin[1], Data::GAUGE);
  yMax[1] = roundY(yMax[1], Data::GAUGE);
  yMin[2] = roundY(yMin[2], Data::TEMPERATURE);
  yMax[2] = roundY(yMax[2], Data::TEMPERATURE);

  os << "{title:'" << title()
     << "',hash:'" << mHash << "'";

  { // spit out t
    JSON::Array<time_t> dd("t");
    os << ",tlabel:'Date',";
    for (tAnObs::const_iterator it(mAnObs.begin()), et(mAnObs.end()); it != et; ++it) {
      dd(it->time);
    }
    os << dd;
  }

  size_t cnt(0);

  if (!isnan(yMin[0])) { // We have flow information
    const Data::Type type(Data::FLOW);
    std::string prefix("y");
    ++cnt;
    os << "," 
       << prefix << "label:'" << mkLabel(type) << "',"
       << prefix << "units:'" << mkUnits(type) << "',";

    JSON::Array<double> dd(prefix);
    for (tAnObs::const_iterator it(mAnObs.begin()), et(mAnObs.end()); it != et; ++it) {
      dd(roundY(it->flow, type));
    }
    os << dd;
  }

  if (!isnan(yMin[1])) { // We have gauge information
    const Data::Type type(Data::GAUGE);
    std::string prefix("y");
    if (cnt != 0) {
      prefix += Convert::toStr(cnt);
    }
    ++cnt;
    os << "," 
       << prefix << "label:'" << mkLabel(type) << "',"
       << prefix << "units:'" << mkUnits(type) << "',";

    JSON::Array<double> dd(prefix);
    for (tAnObs::const_iterator it(mAnObs.begin()), et(mAnObs.end()); it != et; ++it) {
      dd(roundY(it->gauge, type));
    }
    os << dd;
  }

  if (!isnan(yMin[2])) { // We have temperature information
    const Data::Type type(Data::TEMPERATURE);
    std::string prefix("y");
    if (cnt != 0) {
      prefix += Convert::toStr(cnt);
    }
    ++cnt;

    os << "," 
       << prefix << "label:'" << mkLabel(type) << "',"
       << prefix << "units:'" << mkUnits(type) << "',";

    JSON::Array<double> dd(prefix);
    for (tAnObs::const_iterator it(mAnObs.begin()), et(mAnObs.end()); it != et; ++it) {
      dd(roundY(it->temperature, type));
    }
    os << dd;
  }
 
  if (qName) {
    std::string prefix("y");
    if (cnt != 0) {
      prefix += Convert::toStr(cnt);
    }
    ++cnt;
    os << "," << prefix << "label:'Name',";
   
    JSON::Array<std::string> ds(prefix); 
    for (tAnObs::const_iterator it(mAnObs.begin()), et(mAnObs.end()); it != et; ++it) {
      ds(it->name);
    }
    os << ds;
  } // qName

  if (qURL) {
    std::string prefix("y");
    if (cnt != 0) {
      prefix += Convert::toStr(cnt);
    }
    ++cnt;
    os << "," << prefix << "label:'URL',";
    JSON::Array<std::string> ds(prefix);
    for (tAnObs::const_iterator it(mAnObs.begin()), et(mAnObs.end()); it != et; ++it) {
      ds(it->url);
    }
    os << ds;
  } // qURL

  os << "}";
  return os;
}

double
DataGet::roundY(const double y) const
{
  return roundY(y, mType);
}

double
DataGet::roundY(const double y, const Data::Type type)
{
  switch (type) {
    case Data::FLOW:
    case Data::INFLOW:
    case Data::TEMPERATURE:
    case Data::LASTTYPE:
      return round(y);
    case Data::GAUGE:
      return round(y*10)/10;
  }
  return y;
}
