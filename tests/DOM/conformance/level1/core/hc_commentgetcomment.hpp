#ifndef test_hc_commentgetcomment
#define test_hc_commentgetcomment


/*
This C++ source file was generated for Arabica
from the source document contained in the W3C
DOM Conformance Test Suite.
The source document contained the following notice:


Copyright (c) 2001-2004 World Wide Web Consortium,
(Massachusetts Institute of Technology, Institut National de
Recherche en Informatique et en Automatique, Keio University). All
Rights Reserved. This program is distributed under the W3C's Software
Intellectual Property License. This program is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.
See W3C License http://www.w3.org/Consortium/Legal/ for more details.

*/
/**
 *     A comment is all the characters between the starting
 *   '<!--' and ending '-->' 
 *   Retrieve the nodes of the DOM document.  Search for a 
 *   comment node and the content is its value.
* @author Curt Arnold
* @see <a href="http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-1334481328">http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-1334481328</a>
* @see <a href="http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-F68D095">http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-F68D095</a>
* @see <a href="http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-F68D080">http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-F68D080</a>
* @see <a href="http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-111237558">http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-111237558</a>
* @see <a href="http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-111237558">http://www.w3.org/TR/1998/REC-DOM-Level-1-19981001/level-one-core#ID-111237558</a>
* @see <a href="http://www.w3.org/Bugs/Public/show_bug.cgi?id=509">http://www.w3.org/Bugs/Public/show_bug.cgi?id=509</a>
*/

template<class string_type, class string_adaptor>
class hc_commentgetcomment : public DOMTestCase<string_type, string_adaptor> 
{
    typedef DOMTestCase<string_type, string_adaptor> baseT;
   public: 
     hc_commentgetcomment(std::string name) : baseT(name)
     {

       //   check if loaded documents are supported for content type
       const std::string contentType = baseT::getContentType();
    baseT::preload(contentType, "hc_staff", false);
    }

  typedef typename Arabica::DOM::DOMImplementation<string_type, string_adaptor>  DOMImplementation;
  typedef typename Arabica::DOM::Document<string_type, string_adaptor> Document; 
  typedef typename Arabica::DOM::DocumentType<string_type, string_adaptor> DocumentType;
  typedef typename Arabica::DOM::DocumentFragment<string_type, string_adaptor> DocumentFragment; 
  typedef typename Arabica::DOM::Node<string_type, string_adaptor> Node;
  typedef typename Arabica::DOM::Element<string_type, string_adaptor> Element;
  typedef typename Arabica::DOM::Attr<string_type, string_adaptor> Attr;
  typedef typename Arabica::DOM::NodeList<string_type, string_adaptor> NodeList;
  typedef typename Arabica::DOM::NamedNodeMap<string_type, string_adaptor> NamedNodeMap;
  typedef typename Arabica::DOM::Entity<string_type, string_adaptor> Entity;
  typedef typename Arabica::DOM::EntityReference<string_type, string_adaptor> EntityReference;
  typedef typename Arabica::DOM::CharacterData<string_type, string_adaptor> CharacterData;
  typedef typename Arabica::DOM::CDATASection<string_type, string_adaptor> CDATASection;
  typedef typename Arabica::DOM::Text<string_type, string_adaptor> Text;
  typedef typename Arabica::DOM::Comment<string_type, string_adaptor> Comment;
  typedef typename Arabica::DOM::ProcessingInstruction<string_type, string_adaptor> ProcessingInstruction;
  typedef typename Arabica::DOM::Notation<string_type, string_adaptor> Notation;

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
      NodeList elementList;
      Node child;
      String childName;
      String childValue;
      int commentCount = 0;
      int childType;
      NamedNodeMap attributes;
      doc = (Document) baseT::load("hc_staff", false);
      elementList = doc.getChildNodes();
      for (unsigned int indexN65630 = 0; indexN65630 != elementList.getLength(); indexN65630++) {
          child = (Node) elementList.item(indexN65630);
    childType = (int) child.getNodeType();
      
      if (baseT::equals(8, childType)) {
          childName = child.getNodeName();
      baseT::assertEquals("#comment", childName, __LINE__, __FILE__);
  childValue = child.getNodeValue();
      baseT::assertEquals(" This is comment number 1.", childValue, __LINE__, __FILE__);
  attributes = child.getAttributes();
      baseT::assertNull(attributes, __LINE__, __FILE__);
      commentCount += 1;
      }
      }
      assertTrue((commentCount < 2));
      
   }
  
   /*
    *  Gets URI that identifies the test.
    */
   std::string getTargetURI() const
   {
      return "http://www.w3.org/2001/DOM-Test-Suite/level1/core/hc_commentgetcomment";
   }
};

#endif
