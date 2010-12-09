
/*
This c++ source file was generated by for Arabica
and is a derived work from the source document.
The source document contained the following notice:


Copyright (c) 2001 World Wide Web Consortium,
(Massachusetts Institute of Technology, Institut National de
Recherche en Informatique et en Automatique, Keio University). All
Rights Reserved. This program is distributed under the W3C's Software
Intellectual Property License. This program is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.
See W3C License http://www.w3.org/Consortium/Legal/ for more details.

*/

#ifndef test_noderemovechild
#define test_noderemovechild

#include "dom_conf_test.hpp"

/**
 *     The "removeChild(oldChild)" method removes the child node
 *     indicated by "oldChild" from the list of children and
 *     returns it. 
 *     
 *     Remove the first employee by invoking the
 *     "removeChild(oldChild)" method an checking the
 *     node returned by the "getParentNode()" method.   It
 *     should be set to null.
* @author NIST
* @author Mary Brady
* @see <a href="http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-1734834066">http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-1734834066</a>
*/

template<class string_type, class string_adaptor>
class noderemovechild : public DOMTestCase<string_type, string_adaptor> 
{
   public: 
     noderemovechild(std::string name) : DOMTestCase(name)
     {

       //   check if loaded documents are supported for content type
       const std::string contentType = getContentType();
    preload(contentType, "staff", true);
    }

  typedef typename DOMTestCase<string_type, string_adaptor> baseT;
  using baseT::DOMImplementation;
  using baseT::Document; 
  using baseT::DocumentType;
  using baseT::DocumentFragment; 
  using baseT::Node;
  using baseT::Element;
  using baseT::Attr;
  using baseT::NodeList;
  using baseT::NamedNodeMap;
  using baseT::Entity;
  using baseT::EntityReference;
  using baseT::CharacterData;
  using baseT::CDATASection;
  using baseT::Text;
  using baseT::Comment;
  using baseT::ProcessingInstruction;
  using baseT::Notation;

  typedef typename Arabica::DOM::DOMException DOMException;
  typedef string_type String;
  typedef string_adaptor SA;
  typedef bool boolean;

   /*
    * Runs the test case.
    */
   void runTest()
   {
      Document doc;
      Element rootNode;
      NodeList childList;
      Node childToRemove;
      Node removedChild;
      Node parentNode;
      doc = (Document) load("staff", true);
      rootNode = doc.getDocumentElement();
      childList = rootNode.getChildNodes();
      childToRemove = childList.item(1);
      removedChild = rootNode.removeChild(childToRemove);
      parentNode = removedChild.getParentNode();
      assertNull(parentNode);
      
   }
  
   /*
    *  Gets URI that identifies the test.
    */
   std::string getTargetURI() const
   {
      return "http://www.w3.org/2001/DOM-Test-Suite/level1/core/noderemovechild";
   }
};

#endif