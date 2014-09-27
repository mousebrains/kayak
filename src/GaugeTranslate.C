#include "GaugeTranslate.H"
#include "Tokens.H"
#include "String.H"
// #include <iostream>
#include <cstring>

namespace {
  class MyFields {
  public:
    const char *table() {return "gaugeTranslate";}
    const char *str() {return "str";}
    const char *description() {return "description";}
    const char *location() {return "location";}
  };

  MyFields fields;

  void capitalize(std::string& str) {
    if (!str.empty()) {
      str[0] = std::toupper(str[0]);
      for (std::string::size_type i(0); (i = str.find('.', i)) != str.npos;) {
        if (++i < str.size()) {
          str[i] = std::toupper(str[i]);
        }
      }
    }
  }
 
  MyDB::Stmt::tStrings translate(std::string str) {
    typedef std::set<std::string> tState;
    typedef std::map<std::string, std::string> tLoc;
    static const tState toState = {
        "id",
        "oreg.",
        "or",
        "wa"
      };
    static const tLoc toLoc = {
        {"nr", "nr"},
        {"near", "nr"},
        {"at", "at"},
        {"above", "abv"},
        {"ab", "abv"},
        {"abv", "abv"},
        {"below", "blw"},
        {"blw", "blw"},
        {"bl", "blw"},
        {"-", ""}
      };
    static const tLoc xlat = {
        {"mf", "MF"},
        {"nf", "NF"},
        {"sf", "SF"},
        {"r", "River"},
        {"ck", "Creek"},
        {"cr", "Creek"},
        {"tkte", "Toketee"},
        {"o'conner", "O'Conner"},
        {"n.umpqua", "N Umpqua"}
      };

    static const tLoc phrases = {
      {"middle fork", "MF"}
      };
   
    MyDB::Stmt::tStrings a;
    str = String::tolower(str);

    for (tLoc::const_iterator it(phrases.begin()), et(phrases.end()); it != et; ++it) { 
      std::string::size_type i(str.find(it->first));
      if (i == str.npos) continue;
      str.replace(i, it->first.size(), it->second);
    }

    Tokens toks(str, ", \t\n\r()");
    std::string name;
    std::string location;
    int state(0);
    for (Tokens::size_type i(0), e(toks.size()); i < e; ++i) {
      std::string tok(toks[i]);
      if ((i == (e-1)) && (toState.find(tok) != toState.end())) break;
      tLoc::const_iterator it(toLoc.find(tok));
      if (it != toLoc.end()) { // Add to name
        location += (location.empty() ? "" : " ") + it->second;
        state = 1;
        continue;
      }
      it = xlat.find(tok);
      if (it != xlat.end()) {
        tok = it->second;
      } else {
        capitalize(tok);
      }
      if (state == 0) {
        name += (name.empty() ? "" : " ") + tok;
        continue;
      }
      location += (location.empty() ? "" : " ") + tok;
    }

    a.push_back(name);
    a.push_back(location);
 
// std::cout << "'" << str << "' -> '" << name << "' '" << location << "'" << std::endl;
    return a; 
  }
} // anonymous

GaugeTranslate::Info
GaugeTranslate::operator () (const std::string& str)
{
  tInfo::const_iterator it(mInfo.find(str));
  if (it == mInfo.end()) {
    MyDB::Stmt s(mDB);
    s << "SELECT "
      << fields.description()
      << "," << fields.location()
      << " FROM " << fields.table() << " WHERE " << fields.str() << "=";
    s.quoteString(str);
    s << ";"; 

    MyDB::Stmt::tStrings a(s.queryStrings());

    Info info;
    if (a.empty()) { // Insert into translation table with a best guess translation
      a = translate(str);
      // MyDB::Stmt s1(mDB);
      // s1 << "INSERT INTO " << fields.table() << " VALUES("
         // << s1.quotedString(str)
         // << ","
         // << s1.quotedString(a[0])
         // << ","
         // << s1.quotedString(a[1])
         // << ");";
      // s1.query();
    }
    info.description = a[0];
    info.location = a[1];

    it = mInfo.insert(std::make_pair(str, info)).first;
  }

  return it->second;
}
