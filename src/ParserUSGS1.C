// Parse a USGS XML site information file 

#include "ParserUSGS1.H"
#include "XMLParser.H"
#include "Gauges.H"
#include "GaugeTranslate.H"
#include "StateCounty.H"
#include "Convert.H"
#include <iostream>
#include <sstream>

ParserUSGS1::ParserUSGS1(const std::string& url,
                         const std::string& str,
                         const bool qVerbose)
{
  const XMLParser xml(str);
  for (XMLParser::Node root(xml.first()); root; ++root) { // usgs_nwis level
    for (XMLParser::Node site(root.firstChild()); site; ++site) { // site 
      collection(site);
    }
  }
}

void
ParserUSGS1::collection(const XMLParser::Node& site)
{
  static const std::set<std::string> toIgnore = {
      "agency_cd", 
      "alt_datum_cd", 
      "land_net_ds", 
      "site_tp_cd", 
      "coord_acy_cd", 
      "dec_lat_long_datum_cd",
      "district_cd",
      "alt_acy_va",
      "basin_cd",
      "topo_cd",
      "construction_dt",
      "inventory_dt",
      "contrib_drain_area_va",
      };

  Gauges::Info info;
  std::string state, county, description;

  for (XMLParser::Node node(site.firstChild()); node; ++node) { // Walk through site information
    const std::string name(node.name());
    if (toIgnore.find(name) != toIgnore.end()) continue;
    const std::string val(node.value());
    if (name == "site_no") { 
      info.name = val;
      info.idUSGS = val;
    } else if (name == "station_nm") { description = val;
    } else if (name == "dec_lat_va") { info.latitude = Convert::strTo<double>(val);
    } else if (name == "dec_long_va") { info.longitude = Convert::strTo<double>(val);
    } else if (name == "state_cd") { state = val;
    } else if (name == "county_cd") { county = val;
    } else if (name == "alt_va") { info.elevation = Convert::strTo<double>(val);
    } else if (name == "huc_cd") { info.idUnit = val;
    } else if (name == "drain_area_va") { info.drainageArea = Convert::strTo<double>(val);
    } else {
       std::cerr << "Unregonized USGS site field(" << name << ") value(" << val << ")" << std::endl;
    }
  }

  info.gaugeKey = mGauges.name2key(info.name, false);
  if (info.gaugeKey <= 0) { // Look up by idUSGS
    info.gaugeKey = mGauges.idUSGS2gaugeKey(info.name);
  }

  if (info.gaugeKey > 0) { // Found a gauge to put information into
    if (!description.empty()) { // Translate string
      GaugeTranslate translate;
      const GaugeTranslate::Info a(translate(description));
      info.description = a.description;
      info.location = a.location;
    }

    if (!state.empty()) {
      const int skey(Convert::strTo<int>(state));
      StateCounty stateCounty;
      info.state = stateCounty.state(skey);
      if (!county.empty()) {
        const int ckey(Convert::strTo<int>(county));
        info.county = stateCounty.county(skey, ckey);
      }
    }
    mGauges.putInfo(info);
  }
}
