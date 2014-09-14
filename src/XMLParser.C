#include "XMLParser.H"
#include <iostream>
#include <cstring>

XMLParser::XMLParser(const std::string& str)
  : mContent(str.begin(), str.end())
{
  mDoc.parse<0>(&mContent[0]); 
}

XMLParser::Node
XMLParser::Node::findNode(const char *name) const
{
  for (Node node(*this); node; ++node) { // traverse until we find name
    if (node.type() == NODE) {
      if (!strcmp(node.name(), name)) return node; // Found it
      Node n(node.firstChild().findNode(name)); // Recursively descend the tree
      if (n) return n;
    }
  }
  return Node(); // Didn't find it
}

std::string
XMLParser::Node::findValue(const char *name) const
{
  Node node(findNode(name));
  return node ? node.value() : "";
}

std::string
XMLParser::Node::findAttr(const char *name) const
{
  Attr attr(firstAttr(name));
  return attr ? attr.value() : "";
}

void
XMLParser::Node::traverse(std::ostream& os,
                          const int level) const
{
  const std::string prefix(level, ' ' );
  std::cout << level << prefix << "node '" << name() << "'='" << value() << "'" << std::endl;
  for (Attr attr(firstAttr()); attr; ++attr) {
    os << level << prefix << "attr '" << attr.name() << "'='" << attr.value() << "'" << std::endl;
  }
  for (Node a(firstChild()); a; ++a) {
    if (a.type() == NODE) {
      a.traverse(os, level+1);
    }
  }
}
