#ifndef INC_XMLParser_H_
#define INC_XMLParser_H_

#include "rapidxml.hpp"
#include <string>
#include <vector>

class XMLParser {
private:
  std::vector<char> mContent; // Copy of input string
  rapidxml::xml_document<> mDoc;

public:
  XMLParser(const std::string& str);

  class Attr {
  private:
    rapidxml::xml_attribute<> *mAttr;
  public:
    explicit Attr(rapidxml::xml_attribute<> *ptr = 0) : mAttr(ptr) {} 

    operator bool () const {return mAttr;}

    const char *name() const {return mAttr->name();}
    const char *value() const {return mAttr->value();}

    void next(const char *name=0) {mAttr = mAttr->next_attribute(name);}
    void next(const std::string& name) {return next(name.c_str());}
  }; // Attr

  class Node {
  private:
    rapidxml::xml_node<> *mNode;
  public:
    explicit Node(rapidxml::xml_node<> *ptr = 0) : mNode(ptr) {}

    operator bool () const {return mNode;}

    const char *name() const {return mNode->name();}
    const char *value() const {return mNode->value();}

    enum Type {
      NODE = rapidxml::node_element
    };

    Type type() const {return (Type) mNode->type();}

    void next(const char *name=0) {mNode = mNode->next_sibling(name);}
    void next(const std::string& name) {return next(name.c_str());}

    Node firstChild(const char *name=0) const {return Node(mNode->first_node(name));}
    Node firstChild(const std::string& name) const {return firstChild(name.c_str());}

    Attr firstAttr(const char *name=0) const { return Attr(mNode->first_attribute(name));}
    Attr firstAttr(const std::string& name) const {return firstAttr(name.c_str());}
  }; // Node
     
    
  Node first(const char *name=0) const {return Node(mDoc.first_node(name));}
  Node first(const std::string& name) const {return Node(mDoc.first_node(name.c_str()));}
}; // XMLParser

#endif // INC_XMLParser_H_
