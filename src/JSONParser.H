#ifndef INC_JSONParser_H_
#define INC_JSONParser_H_

#include "rapidxml.hpp"
#include <string>
#include <vector>

class JSONParser {
private:
  std::vector<char> mContent; // Copy of input string
  rapidxml::xml_document<> mDoc;

public:
  JSONParser(const std::string& str);

  class Attr {
  private:
    rapidxml::xml_attribute<> *mAttr;
  public:
    explicit Attr(rapidxml::xml_attribute<> *ptr = 0) : mAttr(ptr) {} 

    operator bool () const {return mAttr;}

    const char *name() const {return mAttr->name();}
    const char *value() const {return mAttr->value();}

    bool next() {mAttr = mAttr->next_attribute(); return mAttr;}
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

    bool next() {mNode = mNode->next_sibling(); return mNode;}

    Node firstChild() const {return Node(mNode->first_node());}

    Attr firstAttr() const {return Attr(mNode->first_attribute());}
  }; // Node
     
    
  Node first() const {return Node(mDoc.first_node());}
}; // JSONParser

#endif // INC_JSONParser_H_
