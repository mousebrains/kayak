#include "GuideBook.H"
#include "Convert.H"
#include <sstream>

namespace {
  class GuideFields {
  public:
    const char *table() const {return "Guides";}
    const char *key() const {return "key";}
    const char *bookKey() const {return "bookKey";}
  } guides; // Guides fields
 
  class BookFields {
  public:
    const char *table() const {return "GuideBooks";}
    const char *key() const {return "bookKey";}
  } books; // GuideBooks fields 

  struct Book {
    int bookKey; // Key into GuideBooks table
    std::string name; // Guide book name
    std::string edition; // Guide book edition
    std::string url; // Guide book url
    Book(const int key) : bookKey(key) {};
    bool operator < (const Book& rhs) const {return bookKey < rhs.bookKey;}
  };

  typedef std::set<Book> tBooks;
  tBooks mBooks;

  const Book& loadBook(MyDB& db, const int bookKey) {
    tBooks::const_iterator it(mBooks.find(bookKey));

    if (it == mBooks.end()) { // Not loaded, so go do it
      MyDB::Stmt s(db);
      s << "SELECT * FROM " << books.table() << " WHERE " << books.key() << "=" << bookKey << ";";
      int rc(s.step());
      Book book(bookKey);
      if (rc == SQLITE_ROW) { // Found an entry
        // book.bookKey = s.getInt(0); // Already have
        book.name = s.getString(1);
        book.edition = s.getString(2);
        book.url = s.getString(3);
        it = mBooks.insert(book).first;
        rc = s.step();
      } else if (rc == SQLITE_DONE) { // Did not find an entry 
        it = mBooks.insert(book).first;
      }
      if (rc != SQLITE_DONE) {
        s.errorCheck(rc, std::string(__FILE__) + " line " + Convert::toStr(__LINE__));
      }
    }
    return *it;
  } // loadBook

  std::string maybe(const std::string& url, const std::string body) {
    if (url.empty()) return body;
    return "<a href='" + url + "'>" + body + "</a>";
  } 
} // anonymous

GuideBook::GuideBook(const int key)
{
  loadInfo(key);
}

GuideBook::GuideBook(const tKeys& keys)
{
  for (tKeys::const_iterator it(keys.begin()), et(keys.end()); it != et; ++it) {
    loadInfo(*it);
  }
}

void
GuideBook::loadInfo(const int key)
{
  MyDB::Stmt s(mDB);
  s << "SELECT * FROM " << guides.table() << " WHERE " << guides.key() << "=" << key << ";";

  int rc;

  while ((rc = s.step()) == SQLITE_ROW) { // Loop over what we found 
    Info info(key);
    // info.key = s.getInt(0); // Already have, so skip
    info.bookKey = s.getInt(1);
    info.page = s.getString(2);
    info.run = s.getString(3);
    info.url = s.getString(4);

    const Book& book(loadBook(mDB, info.bookKey));
    info.name = book.name;
    info.edition = book.edition;
    info.urlGB = book.url;
    mInfo.insert(info);
  } // while

  if (rc != SQLITE_DONE) {
    s.errorCheck(rc, std::string(__FILE__) + " line " + Convert::toStr(__LINE__));
  }
}

std::string
GuideBook::Info::mkHTML() const
{
  std::string str(maybe(urlGB, name + (edition.empty() ? "" : (" " + edition))));

  if (!page.empty())
    str += " Page: " + maybe(url, page);

  if (!run.empty())
    str += " Run: " + maybe(url, run);

  return str;
}
