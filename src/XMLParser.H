#ifndef INC_XMLParser_H_
#define INC_XMLParser_H_

#include "rapidxml.hpp"
#include <string>
#include <vector>
#include <iosfwd>

class XMLParser {
private:
  std::vector<char> mContent; // Copy of input string
  rapidxml::xml_document<> mDoc;

public:
  explicit XMLParser(const std::string& str);

  class Attr {
  private:
    rapidxml::xml_attribute<> *mAttr;
  public:
    explicit Attr(rapidxml::xml_attribute<> *ptr = 0) : mAttr(ptr) {} 

    operator bool () const {return mAttr;}

    const char *name() const {return mAttr ? mAttr->name() : "";}
    const char *value() const {return mAttr ? mAttr->value() : "";}
 
    Attr next(const char *name=0) const {
      return mAttr ? Attr(mAttr->next_attribute(name)) : Attr();
    }

    Attr next(const std::string& name) const {return next(name.c_str());}

    Attr& operator ++ () {
      mAttr = mAttr ? mAttr->next_attribute() : mAttr; 
      return *this;
    }

    Attr& operator -- () {
      mAttr = mAttr ? mAttr->previous_attribute() : mAttr; 
      return *this;
    }

    Attr operator ++ (int) {Attr a(mAttr); ++(*this); return a;}
    Attr operator -- (int) {Attr a(mAttr); --(*this); return a;}
  }; // Attr

  class Node {
  private:
    rapidxml::xml_node<> *mNode;
  public:
    explicit Node(rapidxml::xml_node<> *ptr = 0) : mNode(ptr) {}

    operator bool () const {return mNode;}

    const char *name() const {return mNode ? mNode->name() : "";}
    const char *value() const {return mNode ? mNode->value() : "";}

    enum Type {
      DOCUMENT = rapidxml::node_document,
      NODE = rapidxml::node_element,
      DATA = rapidxml::node_data,
      CDATA = rapidxml::node_cdata,
      COMMENT = rapidxml::node_comment,
      DECLARATION = rapidxml::node_declaration,
      DOCTYPE = rapidxml::node_doctype,
      PI = rapidxml::node_pi,
      EMPTY
    };

    Type type() const {return mNode ? (Type) mNode->type() : EMPTY;}

    Node& operator ++ () {mNode = mNode ? mNode->next_sibling() : mNode; return *this;}
    Node& operator -- () {mNode = mNode ? mNode->previous_sibling() : mNode; return *this;}

    Node operator ++ (int) {Node n(mNode); ++(*this); return n;}
    Node operator -- (int) {Node n(mNode); --(*this); return n;}


    Node next(const char *name=0) const {return mNode ? Node(mNode->next_sibling(name)) : Node();}
    Node next(const std::string& name) const {return next(name.c_str());}

    Node firstChild(const char *name=0) const {
      return mNode ? Node(mNode->first_node(name)) : Node();
    }
    Node firstChild(const std::string& name) const {return firstChild(name.c_str());}

    Attr firstAttr(const char *name=0) const {
      return mNode ? Attr(mNode->first_attribute(name)) : Attr();
    }
    Attr firstAttr(const std::string& name) const {return firstAttr(name.c_str());}

    Node parent() const {return mNode ? Node(mNode->parent()) : Node();}

    Node findNode(const char *name) const;
    std::string findValue(const char *name) const;
    std::string findAttr(const char *name) const;

    void traverse(std::ostream& os, const int level=0) const;
  }; // Node
     
    
  Node first(const char *name=0) const {return Node(mDoc.first_node(name));}
  Node first(const std::string& name) const {return first(name.c_str());}
}; // XMLParser

#endif // INC_XMLParser_H_
