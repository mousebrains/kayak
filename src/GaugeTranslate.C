#include "GaugeTranslate.H"
#include "Tokens.H"
#include "Convert.H"
#include <iostream>

namespace {
  class MyFields {
  public:
    const char *table() {return "gaugeTranslate";}
    const char *str() {return "str";}
    const char *description() {return "description";}
    const char *location() {return "location";}
  };

  MyFields fields;

  MyDB::Stmt::tStrings translate(const std::string& str) {
    typedef std::set<std::string> tState;
    typedef std::map<std::string, std::string> tLoc;
    static const tState toState = {
        "oreg.",
        "or",
        "wa"
      };
    static const tLoc toLoc = {
        {"nr", "nr"},
        {"near", "nr"},
        {"at", "at"},
        {"-", ""}
      };
    static const tLoc xlat = {
        {"mf", "MF"},
        {"nf", "NF"},
        {"sf", "SF"},
        {"cr", "Creek"},
        {"o'conner", "O'Conner"}
      };
   
    MyDB::Stmt::tStrings a;
    Tokens toks(Convert::tolower(str), ", \t\n\r");
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
        tok[0] = (tok[0] >= 'a' && tok[0] <= 'z') ? (tok[0] - 'a' + 'A') : tok[0]; // Cap 1st char
      }
      if (state == 0) {
        name += (name.empty() ? "" : " ") + tok;
        continue;
      }
      location += (location.empty() ? "" : " ") + tok;
    }

    a.push_back(name);
    a.push_back(location);
 
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
