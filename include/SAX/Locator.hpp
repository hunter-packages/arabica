#ifndef ARABICA_LOCATOR_H
#define ARABICA_LOCATOR_H

// Locator.h

#include <string>

#include <SAX/ArabicaConfig.hpp>
#include <Arabica/StringAdaptor.hpp>

namespace Arabica
{
namespace SAX 
{

/**
 * Interface for associating a SAX event with a document location.
 *
 * <p>If a SAX parser provides location information to the SAX
 * application, it does so by implementing this interface and then
 * passing an instance to the application using the content
 * handler's {@link ContentHandler#setDocumentLocator
 * setDocumentLocator} method.  The application can use the
 * object to obtain the location of any other content handler event
 * in the XML source document.</p>
 *
 * <p>Note that the results returned by the object will be valid only
 * during the scope of each content handler method: the application
 * will receive unpredictable results if it attempts to use the
 * locator at any other time.</p>
 *
 * <p>SAX parsers are not required to supply a locator, but they are
 * very strongly encouraged to do so.  If the parser supplies a
 * locator, it must do so before reporting any other document events.
 * If no locator has been set by the time the application receives
 * the {@link ContentHandler#startDocument startDocument}
 * event, the application should assume that a locator is not 
 * available.</p>
 *
 * @since SAX 1.0
 * @author Jez Higgins, 
 *         <a href="mailto:jez@jezuk.co.uk">jez@jezuk.co.uk</a>
 * @version 2.0
 * @see ContentHandler#setDocumentLocator 
 */
template<class string_type, class string_adaptor = Arabica::default_string_adaptor<string_type> >
class Locator
{
public:
  typedef string_type stringT;

  virtual ~Locator() { }

  /**
   * Return the public identifier for the current document event.
   *
   * <p>The return value is the public identifier of the document
   * entity or of the external parsed entity in which the markup
   * triggering the event appears.</p>
   *
   * @return A string containing the public identifier, or
   *         an empty string if none is available.
   * @see #getSystemId
   */
  virtual stringT getPublicId() const = 0;
  /**
   * Return the system identifier for the current document event.
   *
   * <p>The return value is the system identifier of the document
   * entity or of the external parsed entity in which the markup
   * triggering the event appears.</p>
   *
   * <p>If the system identifier is a URL, the parser must resolve it
   * fully before passing it to the application.</p>
   *
   * @return A string containing the public identifier, or
   *         an empty string if none is available.
   * @see #getPublicId
   */
  virtual stringT getSystemId() const = 0;
  /**
   * Return the line number where the current document event ends.
   *
   * <p><strong>Warning:</strong> The return value from the method
   * is intended only as an approximation for the sake of error
   * reporting; it is not intended to provide sufficient information
   * to edit the character content of the original XML document.</p>
   *
   * <p>The return value is an approximation of the line number
   * in the document entity or external parsed entity where the
   * markup triggering the event appears.</p>
   *
   * <p>If possible, the SAX driver should provide the line position 
   * of the first character after the text associated with the document 
   * event.  The first line in the document is line 1.</p>
   *
   * @return The line number, or -1 if none is available.
   * @see #getColumnNumber
   */
  virtual size_t getLineNumber() const = 0;
  /**
   * Return the column number where the current document event ends.
   *
   * <p><strong>Warning:</strong> The return value from the method
   * is intended only as an approximation for the sake of error
   * reporting; it is not intended to provide sufficient information
   * to edit the character content of the original XML document.</p>
   *
   * <p>The return value is an approximation of the column number
   * in the document entity or external parsed entity where the
   * markup triggering the event appears.</p>
   *
   * <p>If possible, the SAX driver should provide the line position 
   * of the first character after the text associated with the document 
   * event.</p>
   *
   * <p>If possible, the SAX driver should provide the line position 
   * of the first character after the text associated with the document 
   * event.  The first column in each line is column 1.</p>
   *
   * @return The column number, or -1 if none is available.
   * @see #getLineNumber
   */
  virtual size_t getColumnNumber() const = 0;
}; // class Locator

} // namespace SAX
} // namespace Arabica

#endif
// end of file
