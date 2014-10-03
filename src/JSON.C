#include "JSON.H"
#include <iostream>
#include <set>
#include <map>
#include <cmath>

namespace {
  const char* comma(",");
  const char* empty("");

  typedef std::set<time_t> tDelta;

  time_t gcd(time_t a, time_t b) { // Find the greatest common divisor of two time diffs
    while (b != 0) { // Euclidean algorithm
      const time_t t(b);
      b = a % b;
      a = t;
    }
    return a;
  }

  time_t gcd(const tDelta& delta) { // Find greatest common divisor of all the deltas
    if (delta.empty()) return 1;
    if (delta.size() == 1) return *(delta.begin());

    tDelta::const_iterator it(delta.begin());
    const tDelta::const_iterator et(delta.end());

    time_t a(gcd(*(it++), *(it++))); // gcd of first pair of time diffs

    for (; it != et; ++it) { // Now work on 3rd, ... time diff against all previous ones
      a = gcd(a, *it);
    }
    return a;
  }

}

namespace JSON {
template<typename T>
std::ostream&
Array<T>::dumpSmall(std::ostream& os) const
{
  os << mName << ":[";

  if (mValues.empty()) {
    T min(mValues[0]);
    T max(mValues[0]);
    for (size_t i(0), e(mValues.size()); i < e; ++i) {
      const T& val(mValues[i]);
      os << ((i > 0) ? comma : empty) << mValues[i];
      min = min > val ? val : min;
      max = max < val ? val : max;
    }
    os << "],"
       << mName << "Min:" << min << ","
       << mName << "Max:" << max;
  } else {
    os << "]";
  }
  return os;
}

template<typename T>
std::ostream&
Array<T>::dump(std::ostream& os) const
{
  return dumpSmall(os);
}


template<> // string specialization
std::ostream&
Array<std::string>::dumpSmall(std::ostream& os) const
{
  os << mName << ":[";

  for (size_t i(0), e(mValues.size()); i < e; ++i) {
    const char quote(mValues[i].find("'") == std::string::npos ? '\'' : '"');
    os << ((i > 0) ? comma : empty) 
       << quote
       << mValues[i]
       << quote;
  }

  os << "]";
  return os;
}

template<> // string specialization
std::ostream&
Array<std::string>::dump(std::ostream& os) const
{
  if (mValues.size() < 2) {
    return dumpSmall(os);
  }

  typedef std::map<std::string, size_t> tMap;
  tMap str2index;
  std::vector<size_t> indices;
  std::vector<std::string> values;

  for (size_t i(0), e(mValues.size()); i < e; ++i) {
    const std::string& str(mValues[i]);
    tMap::const_iterator it(str2index.find(str));
    if (it == str2index.end()) {
      it = str2index.insert(std::make_pair(str, str2index.size())).first;
      values.push_back(str);
    }
    indices.push_back(it->second);
  }

  os << mName << "Map" << ":[";
  for (size_t i(0), e(values.size()); i < e; ++i) {
    const char quote(values[i].find("'") == std::string::npos ? '\'' : '"');
    os << ((i > 0) ? comma : empty) 
       << quote
       << values[i]
       << quote;
  }
  os << "]," << mName << ":[";
  for (size_t i(0), e(indices.size()); i < e; ++i) {
    os << (i > 0 ? comma : empty) << indices[i];
  }
  os << "]," << mName << "Enc:'Map'";
  
  return os;
}

template <> // time_t specialization
std::ostream&
Array<time_t>::dump(std::ostream& os) const
{
  if (mValues.size() < 2) {
    return dumpSmall(os);
  }
    
  tDelta deltas;
  time_t min(mValues[0]);
  time_t max(mValues[0]);
  for (size_t i(1), e(mValues.size()); i < e; ++i) {
    const time_t t(mValues[i]);
    const time_t dt(t - mValues[i-1]);
    min = min > t ? t : min;
    max = max < t ? t : max;
    if (dt) {
      deltas.insert(dt);
    }
  }

  const time_t dt(gcd(deltas));

  os << mName << "Min:" << min << "," 
     << mName << "Max:" << max << "," 
     << mName << "Step:" << dt << "," 
     << mName << "Enc:'Step'" << ","
     << mName << ":[";

  for (size_t i(0), e(mValues.size()); i < e; ++i) {
    os << (i > 0 ? comma : empty) << ((mValues[i] - min) / dt);
  }

  os << "]";
  return os;
}

template <> // double specialization
std::ostream&
Array<double>::dump(std::ostream& os) const
{
  double min(mValues[0]);
  double max(mValues[0]);
  for (size_t i(1), e(mValues.size()); i < e; ++i) {
    const double val(mValues[i]);
    min = isnan(min) || (min > val) ? val : min;
    max = isnan(max) || (max < val) ? val : max;
  }

  os << mName << "Min:" << (isnan(min) ? 0 : min) << "," 
     << mName << "Max:" << (isnan(max) ? 0 : max) << "," 
     << mName << "Enc:'Min'" << ","
     << mName << ":[";

  for (size_t i(0), e(mValues.size()); i < e; ++i) {
    os << (i > 0 ? comma : empty);
    if (!isnan(mValues[i])) {
      os << (mValues[i] - min);
    }
  }

  os << "]";
  return os;
}

template class Array<time_t>;
template class Array<double>; 
template class Array<std::string>; 
} // JSON
