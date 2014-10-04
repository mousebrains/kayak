#include "PruneData.H"
#include "Data.H"
#include <iostream>

int
main (int argc,
      char **argv)
{
  PruneData pd;

  Data data;
  Types::Keys keys(data.allKeys());
  Types::Keys k;
 
  for (Types::Keys::const_iterator it(keys.begin()), et(keys.end()); it != et; ++it) {
    for (int i(Data::FLOW); i < Data::LASTTYPE; ++i) { // Walk through all data types
      Data::tTimeKeys a(data.timeKeys(*it, (Data::Type) i));
      Data::tTimeKeys::size_type n(pd(a));
      if (n) { // Something to prune
        size_t nDropped(0);
        for (Data::tTimeKeys::const_iterator jt(a.begin()), jet(a.end()); jt != jet; ++jt) {
          if (jt->second) {
            k.insert(jt->second);
            ++nDropped;
          }
        }
        std::cout << "Dropping " << nDropped << " rows of " << a.size() << " from " 
                  << data.source().key2name(*it) << " for type " << i << std::endl;
      } 
    }
    if (!data.nRows(*it)) {
	    std::cout << data.source().key2name(*it) << " is empty" << std::endl; 
    }
  }

  if (!k.empty()) {
    std::cout << "Dropped " << k.size() << " rows" << std::endl;
    data.dropRows(k);
    data.checkPoint();
  }
}
