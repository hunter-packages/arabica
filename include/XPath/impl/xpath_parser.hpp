#ifndef ARABICA_XPATHIC_XPATH_PARSER_HPP
#define ARABICA_XPATHIC_XPATH_PARSER_HPP

#include <boost/spirit/core.hpp>
#include <boost/spirit/tree/ast.hpp>
#include <string>
#include <stdexcept>
#include <map>
#include "xpath_object.hpp"
#include "xpath_expression.hpp"
#include "xpath_ast.hpp"
#include "xpath_grammar.hpp"
#include "xpath_namespace_context.hpp"
#include "xpath_function_resolver.hpp"
#include "xpath_variable_resolver.hpp"
#include "xpath_resolver_holder.hpp"

namespace Arabica
{
namespace XPath
{

class SyntaxException : public std::runtime_error
{
public:
  SyntaxException(const std::string& thing) : std::runtime_error("Bad XPath: " + thing) { }
}; // class SyntaxException

class RuntimeException : public std::runtime_error
{
public:
  RuntimeException(const std::string& thing) : std::runtime_error("Cannot evaluate XPath: " + thing) { }
}; // class RuntimeException

class UnsupportedException : public std::runtime_error
{
public:
  UnsupportedException(const std::string& thing) : std::runtime_error("Sorry, haven't implemented '" + thing + "' yet") { }
}; // class UnsupportedException

namespace impl 
{
  template<class string_type, class string_adaptor> class CompilationContext;

  template<class string_type, class string_adaptor> class StepExpression;

  template<class string_type, class string_adaptor>
  class StepList : public std::deque<impl::StepExpression<string_type, string_adaptor>*> { };
} // namespace impl

template<class string_type, class string_adaptor = Arabica::default_string_adaptor<string_type> >
class XPath 
{
public:
  XPath()
  {
    resetNamespaceContext();
    resetVariableResolver();
    resetFunctionResolver();
  } // XPath

  ~XPath()
  {
  } // ~XPath

  XPathExpressionPtr<string_type, string_adaptor> compile(const string_type& xpath) const
  {
    return do_compile(xpath, &XPath::parse_xpath, expression_factory());
  } // compile

  XPathExpressionPtr<string_type, string_adaptor> compile_expr(const string_type& xpath) const
  {
    return do_compile(xpath, &XPath::parse_xpath_expr, expression_factory());
  } // compile_expr

  XPathExpressionPtr<string_type, string_adaptor> compile_match(const string_type& xpath) const
  {
    return do_compile(xpath, &XPath::parse_xpath_match, match_factory());
  } // compile_match

  XPathExpressionPtr<string_type, string_adaptor> compile_attribute_value_template(const string_type& xpath) const
  {
    return do_compile(xpath, &XPath::parse_xpath_attribute_value_template, attribute_value_factory());
  } // compile_attribute_value_template

  XPathValuePtr<string_type> evaluate(const string_type& xpath, const DOM::Node<string_type>& context) const
  {
    ExecutionContext<string_type, string_adaptor> executionContext;
    executionContext.setVariableResolver(getVariableResolver());
    return compile(xpath)->evaluate(context, executionContext);
  } // evaluate

  XPathValuePtr<string_type> evaluate_expr(const string_type& xpath, const DOM::Node<string_type>& context) const
  {
    ExecutionContext<string_type, string_adaptor> executionContext;
    executionContext.setVariableResolver(getVariableResolver());
    return compile_expr(xpath)->evaluate(context, executionContext);
  } // evaluate_expr

  void setNamespaceContext(const NamespaceContext<string_type, string_adaptor>& namespaceContext) { namespaceContext_.set(namespaceContext); }
  void setNamespaceContext(NamespaceContextPtr<string_type, string_adaptor> namespaceContext) { namespaceContext_.set(namespaceContext); }
  const NamespaceContext<string_type, string_adaptor>& getNamespaceContext() const { return namespaceContext_.get(); }
  void resetNamespaceContext() { namespaceContext_.set(NamespaceContextPtr<string_type, string_adaptor>(new NullNamespaceContext<string_type, string_adaptor>())); }

  void setVariableResolver(const VariableResolver<string_type, string_adaptor>& variableResolver) { variableResolver_.set(variableResolver); }
  void setVariableResolver(VariableResolverPtr<string_type, string_adaptor> variableResolver) { variableResolver_.set(variableResolver); }
  const VariableResolver<string_type, string_adaptor>& getVariableResolver() const { return variableResolver_.get(); }
  void resetVariableResolver() { variableResolver_.set(VariableResolverPtr<string_type, string_adaptor>(new NullVariableResolver<string_type, string_adaptor>())); }

  void setFunctionResolver(const FunctionResolver<string_type, string_adaptor>& functionResolver) { functionResolver_.set(functionResolver); }
  void setFunctionResolver(FunctionResolverPtr<string_type, string_adaptor> functionResolver) { functionResolver_.set(functionResolver); }
  const FunctionResolver<string_type, string_adaptor>& getFunctionResolver() const { return functionResolver_.get(); }
  void resetFunctionResolver() { functionResolver_.set(FunctionResolverPtr<string_type, string_adaptor>(new NullFunctionResolver<string_type, string_adaptor>())); }

private:
  typedef XPathExpression<string_type, string_adaptor>* (*compileFn)(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  typedef typename impl::types<string_adaptor>::tree_info_t(XPath::*parserFn)(const string_type& str) const;
  typedef XPathExpression<string_type, string_adaptor>* (*compilerFn)(typename impl::types<string_adaptor>::node_iter_t const& i, 
                                                                      impl::CompilationContext<string_type, string_adaptor>& context);

  XPathExpressionPtr<string_type, string_adaptor> do_compile(const string_type& xpath, 
                                                             parserFn parser,
                                                             const std::map<int, compileFn>& factory) const
  {
    typename impl::types<string_adaptor>::tree_info_t ast;
    try {
      ast = (this->*parser)(xpath);
      if(!ast.full)
        throw SyntaxException(string_adaptor::asStdString(xpath));

      impl::CompilationContext<string_type, string_adaptor> context(*this, getNamespaceContext(), getFunctionResolver());
      return XPathExpressionPtr<string_type, string_adaptor>(compile_with_factory(ast.trees.begin(), context, factory));
    } // try
    catch(const std::exception&) 
    {
      throw;
    } // catch
    catch(...) 
    {
      throw SyntaxException(string_adaptor().asStdString(xpath));
    } // catch
  } // do_compile

  typename impl::types<string_adaptor>::tree_info_t parse_xpath(const string_type& str) const
  {
    typename impl::types<string_adaptor>::str_iter_t first = string_adaptor::begin(str), last = string_adaptor::end(str);
    return ast_parse(first, last, xpathg_);
  } // parse_xpath

  typename impl::types<string_adaptor>::tree_info_t parse_xpath_expr(const string_type& str) const
  {
    typename impl::types<string_adaptor>::str_iter_t first = string_adaptor::begin(str), last = string_adaptor::end(str);
    return ast_parse(first, last, xpathge_);
  } // parse_xpath_expr

  typename impl::types<string_adaptor>::tree_info_t parse_xpath_match(const string_type& str) const
  {
    typename impl::types<string_adaptor>::str_iter_t first = string_adaptor::begin(str), last = string_adaptor::end(str);
    return ast_parse(first, last, xpathgm_);
  } // parse_xpath_match

  typename impl::types<string_adaptor>::tree_info_t parse_xpath_attribute_value_template(const string_type& str) const
  {
    typename impl::types<string_adaptor>::str_iter_t first = string_adaptor::begin(str), last = string_adaptor::end(str);
    return ast_parse(first, last, xpathavt_);
  } // parse_xpath_attribute_value_template

  impl::xpath_grammar xpathg_;
  impl::xpath_grammar_expr xpathge_;
  impl::xpath_grammar_match xpathgm_;
  impl::xpath_grammar_attribute_value xpathavt_;

  impl::ResolverHolder<const NamespaceContext<string_type, string_adaptor> > namespaceContext_;
  impl::ResolverHolder<const VariableResolver<string_type, string_adaptor> > variableResolver_;
  impl::ResolverHolder<const FunctionResolver<string_type, string_adaptor> > functionResolver_;

  /////////////////////////////////////////////////////////////////////////////////
public:
  static XPathExpression<string_type, string_adaptor>* compile_expression(typename impl::types<string_adaptor>::node_iter_t const& i, 
                                                          impl::CompilationContext<string_type, string_adaptor>& context)
  {
    return compile_with_factory(i, context, XPath::expression_factory());
  } // compile_expression

  static XPathExpression<string_type, string_adaptor>* compile_match(typename impl::types<string_adaptor>::node_iter_t const& i, 
                                                          impl::CompilationContext<string_type, string_adaptor>& context)
  {
    return compile_with_factory(i, context, XPath::match_factory());
  } // compile_match

  static XPathExpression<string_type, string_adaptor>* compile_attribute_value(typename impl::types<string_adaptor>::node_iter_t const& i, 
                                                          impl::CompilationContext<string_type, string_adaptor>& context)
  {
    return compile_with_factory(i, context, XPath::attribute_value_factory());
  } // compile_attribute_value

  static XPathExpression<string_type, string_adaptor>* compile_with_factory(typename impl::types<string_adaptor>::node_iter_t const& i, 
                                                                            impl::CompilationContext<string_type, string_adaptor>& context,
                                                                            const std::map<int, compileFn>& factory)
  {
    long id = impl::getNodeId<string_adaptor>(i);
    typename std::map<int, compileFn>::const_iterator f = factory.find(id);
    if(f == factory.end())
    {
      //XPath::dump(i, 0);
      throw UnsupportedException(string_adaptor().asStdString(XPath::names()[id]));
    }
  
    try {
      return (f->second)(i, context);
    }
    catch(...)
    {
      //XPath::dump(i, 0);
      throw;
    }
  } // compile_with_factory

private:
  static XPathExpression<string_type, string_adaptor>* createAbsoluteLocationPath(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createRelativeLocationPath(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createSingleStepRelativeLocationPath(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createExpression(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createFunction(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createBinaryExpression(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createLiteral(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createNumber(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createVariable(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createSingleStepAbsoluteLocationPath(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createUnaryExpression(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createUnaryNegativeExpr(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);

  static impl::StepList<string_type, string_adaptor> createStepList(typename impl::types<string_adaptor>::node_iter_t const& from, typename impl::types<string_adaptor>::node_iter_t const& to, impl::CompilationContext<string_type, string_adaptor>& context);

  static XPathExpression<string_type, string_adaptor>* createDocMatch(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createSingleMatchStep(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createRelativePathPattern(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createAlternatePattern(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);

  static impl::StepList<string_type, string_adaptor> createPatternList(typename impl::types<string_adaptor>::node_iter_t const& from, typename impl::types<string_adaptor>::node_iter_t const& to, impl::CompilationContext<string_type, string_adaptor>& context);

  static XPathExpression<string_type, string_adaptor>* createAttributeValue(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createEmbeddedExpr(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createDoubleLeftCurly(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);
  static XPathExpression<string_type, string_adaptor>* createDoubleRightCurly(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context);

  static std::map<int, compileFn>& expression_factory()
  {
    static std::map<int, compileFn> f = init_createFunctions();
    return f;
  } // expression_factory

  static std::map<int, compileFn>& match_factory()
  {
    static std::map<int, compileFn> f = init_matchCreateFunctions();
    return f;
  } // match_factory

  static std::map<int, compileFn>& attribute_value_factory()
  {
    static std::map<int, compileFn> f = init_attributeValueCreateFunctions();
    return f;
  } // attribute_value_factory

  static std::map<int, string_type>& names()
  {
    static std::map<int, string_type> n = init_debugNames();
    return n;
  } // names

  static const std::map<int, compileFn> init_createFunctions()
  {
    std::map<int, compileFn> factory;

    factory[impl::AbsoluteLocationPath_id] = createAbsoluteLocationPath;
    factory[impl::RelativeLocationPath_id] = createRelativeLocationPath;
    factory[impl::AbbreviatedAbsoluteLocationPath_id] = createAbsoluteLocationPath;
    factory[impl::Step_id] = createRelativeLocationPath;
    factory[impl::PathExpr_id] = createRelativeLocationPath;
    factory[impl::FilterExpr_id] = createRelativeLocationPath;

    factory[impl::PrimaryExpr_id] = createExpression;

    factory[impl::FunctionCall_id] = createFunction;

    factory[impl::AdditiveExpr_id] = createBinaryExpression;
    factory[impl::MultiplicativeExpr_id] = createBinaryExpression;
    factory[impl::EqualityExpr_id] = createBinaryExpression;
    factory[impl::RelationalExpr_id] = createBinaryExpression;
    factory[impl::OrExpr_id] = createBinaryExpression;
    factory[impl::AndExpr_id] = createBinaryExpression;
    factory[impl::UnionExpr_id] = createBinaryExpression;

    factory[impl::Literal_id] = createLiteral;
    factory[impl::Number_id] = createNumber;
    factory[impl::Digits_id] = createNumber;

    factory[impl::VariableReference_id] = createVariable;

    factory[impl::NodeTest_id] = createSingleStepRelativeLocationPath;
    factory[impl::QName_id] = createSingleStepRelativeLocationPath;
    factory[impl::NCName_id] = createSingleStepRelativeLocationPath;
    factory[impl::AnyName_id] = createSingleStepRelativeLocationPath;
    factory[impl::Text_id] = createSingleStepRelativeLocationPath;
    factory[impl::Comment_id] = createSingleStepRelativeLocationPath;
    factory[impl::ProcessingInstruction_id] = createSingleStepRelativeLocationPath;
    factory[impl::Node_id] = createSingleStepRelativeLocationPath;
    factory[impl::Slash_id] = createSingleStepAbsoluteLocationPath;

    factory[impl::SelfSelect_id] = createSingleStepRelativeLocationPath;
    factory[impl::ParentSelect_id] = createSingleStepRelativeLocationPath;

    factory[impl::UnaryExpr_id] = createUnaryExpression;
    factory[impl::UnaryMinusOperator_id] = createUnaryNegativeExpr;

    return factory;
  } // init_createFunctions

  static const std::map<int, compileFn> init_matchCreateFunctions()
  {
    std::map<int, compileFn> factory;

    factory[impl::Slash_id] = createDocMatch;
    factory[impl::NCName_id] = createSingleMatchStep;
    factory[impl::AnyName_id] = createSingleMatchStep;
    factory[impl::Node_id] = createSingleMatchStep;
    factory[impl::Text_id] = createSingleMatchStep;
    factory[impl::Comment_id] = createSingleMatchStep;
    factory[impl::ProcessingInstruction_id] = createSingleMatchStep;
    factory[impl::StepPattern_id] = createRelativePathPattern;
    factory[impl::LocationPathPattern_id] = createRelativePathPattern;
    factory[impl::RelativePathPattern_id] = createRelativePathPattern;
    factory[impl::Pattern_id] = createAlternatePattern;

    return factory;
  } // init_matchCreateFunctions

  static const std::map<int, compileFn> init_attributeValueCreateFunctions()
  {
    std::map<int, compileFn> factory;

    factory[impl::AttributeValueTemplate_id] = createAttributeValue;
    factory[impl::DoubleLeftCurly_id] = createDoubleLeftCurly;
    factory[impl::DoubleRightCurly_id] = createDoubleRightCurly;
    factory[impl::EmbeddedExpr_id] = createEmbeddedExpr;
    factory[impl::AttrLiteral_id] = createLiteral;

    return factory;
  } // init_attributeValueCreateFunctions
    
  static const std::map<int, string_type> init_debugNames()
  {
    std::map<int, string_type> names;
    typedef string_adaptor SA;

    names[impl::LocationPath_id] = SA::construct_from_utf8("LocationPath");
    names[impl::AbsoluteLocationPath_id] = SA::construct_from_utf8("AbsoluteLocationPath");
    names[impl::RelativeLocationPath_id] = SA::construct_from_utf8("RelativeLocationPath");
    names[impl::Step_id] = SA::construct_from_utf8("Step");
    names[impl::AxisSpecifier_id] = SA::construct_from_utf8("AxisSpecifier");
    names[impl::NodeTest_id] = SA::construct_from_utf8("NodeTest");
    names[impl::Predicate_id] = SA::construct_from_utf8("Predicate");
    names[impl::PredicateExpr_id] = SA::construct_from_utf8("PredicateExpr");
    names[impl::AbbreviatedAbsoluteLocationPath_id] = SA::construct_from_utf8("AbbreviatedAbsoluteLocationPath");
    names[impl::AbbreviatedStep_id] = SA::construct_from_utf8("AbbreviatedStep");
    names[impl::AbbreviatedAxisSpecifier_id] = SA::construct_from_utf8("AbbreviatedAxisSpecifier");
    names[impl::Expr_id] = SA::construct_from_utf8("Expr");
    names[impl::PrimaryExpr_id] = SA::construct_from_utf8("PrimaryExpr");
    names[impl::FunctionCall_id] = SA::construct_from_utf8("FunctionCall");
    names[impl::Argument_id] = SA::construct_from_utf8("Argument");
    names[impl::UnionExpr_id] = SA::construct_from_utf8("UnionExpr");
    names[impl::PathExpr_id] = SA::construct_from_utf8("PathExpr");
    names[impl::FilterExpr_id] = SA::construct_from_utf8("FilterExpr");
    names[impl::OrExpr_id] = SA::construct_from_utf8("OrExpr");
    names[impl::AndExpr_id] = SA::construct_from_utf8("AndExpr");
    names[impl::EqualityExpr_id] = SA::construct_from_utf8("EqualityExpr");
    names[impl::RelationalExpr_id] = SA::construct_from_utf8("RelationalExpr");
    names[impl::AdditiveExpr_id] = SA::construct_from_utf8("AdditiveExpr");
    names[impl::MultiplicativeExpr_id] = SA::construct_from_utf8("MultiplicativeExpr");
    names[impl::UnaryExpr_id] = SA::construct_from_utf8("UnaryExpr");
    names[impl::Literal_id] = SA::construct_from_utf8("Literal");
    names[impl::Number_id] = SA::construct_from_utf8("Number");
    names[impl::Digits_id] = SA::construct_from_utf8("Digits");
    names[impl::MultiplyOperator_id] = SA::construct_from_utf8("MultiplyOperator");
    names[impl::FunctionName_id] = SA::construct_from_utf8("FunctionName");
    names[impl::VariableReference_id] = SA::construct_from_utf8("VariableReference");
    names[impl::NameTest_id] = SA::construct_from_utf8("NameTest");
    names[impl::S_id] = SA::construct_from_utf8("S");
    names[impl::NodeType_id] = SA::construct_from_utf8("NodeType");
    names[impl::AxisName_id] = SA::construct_from_utf8("AxisName");

    names[impl::QName_id] = SA::construct_from_utf8("QName");
    names[impl::Prefix_id] = SA::construct_from_utf8("Prefix");
    names[impl::LocalPart_id] = SA::construct_from_utf8("LocalPart");
    names[impl::NCName_id] = SA::construct_from_utf8("NCName");
    names[impl::NCNameChar_id] = SA::construct_from_utf8("NCNameChar");

    names[impl::Slash_id] = SA::construct_from_utf8("/");
    names[impl::SlashSlash_id] = SA::construct_from_utf8("//");

    names[impl::AncestorOrSelf_id] = SA::construct_from_utf8("ancestor-or-self::");
    names[impl::Ancestor_id] = SA::construct_from_utf8("ancestor::");
    names[impl::Attribute_id] = SA::construct_from_utf8("attribute::");
    names[impl::Child_id] = SA::construct_from_utf8("child::");
    names[impl::DescendantOrSelf_id] = SA::construct_from_utf8("descendant-or-self::");
    names[impl::Descendant_id] = SA::construct_from_utf8("descendant::");
    names[impl::FollowingSibling_id] = SA::construct_from_utf8("following-sibling::");
    names[impl::Following_id] = SA::construct_from_utf8("following::");
    names[impl::Namespace_id] = SA::construct_from_utf8("namespace::");
    names[impl::Parent_id] = SA::construct_from_utf8("parent::");
    names[impl::PrecedingSibling_id] = SA::construct_from_utf8("preceding-sibling::");
    names[impl::Preceding_id] = SA::construct_from_utf8("preceding::");
    names[impl::Self_id] = SA::construct_from_utf8("self::");

    names[impl::Comment_id] = SA::construct_from_utf8("comment()");
    names[impl::Text_id] = SA::construct_from_utf8("text()");
    names[impl::ProcessingInstruction_id] = SA::construct_from_utf8("processing-instruction()");
    names[impl::Node_id] = SA::construct_from_utf8("node()");
    names[impl::AnyName_id] = SA::construct_from_utf8("AnyName");

    names[impl::SelfSelect_id] = SA::construct_from_utf8("SelfSelect");
    names[impl::ParentSelect_id] = SA::construct_from_utf8("ParentSelect");

    names[impl::LeftSquare_id] = SA::construct_from_utf8("[");
    names[impl::RightSquare_id] = SA::construct_from_utf8("]");

    names[impl::LeftBracket_id] = SA::construct_from_utf8("(");
    names[impl::RightBracket_id] = SA::construct_from_utf8(")");

    names[impl::PlusOperator_id] = SA::construct_from_utf8("+");
    names[impl::MinusOperator_id] = SA::construct_from_utf8("-");
    names[impl::ModOperator_id] = SA::construct_from_utf8("mod");
    names[impl::DivOperator_id] = SA::construct_from_utf8("div");
    names[impl::EqualsOperator_id] = SA::construct_from_utf8("=");
    names[impl::NotEqualsOperator_id] = SA::construct_from_utf8("!=");
    names[impl::LessThanOperator_id] = SA::construct_from_utf8("<");
    names[impl::LessThanEqualsOperator_id] = SA::construct_from_utf8("<=");
    names[impl::GreaterThanOperator_id] = SA::construct_from_utf8(">");
    names[impl::GreaterThanEqualsOperator_id] = SA::construct_from_utf8(">=");

    names[impl::OrOperator_id] = SA::construct_from_utf8("or");
    names[impl::AndOperator_id] = SA::construct_from_utf8("and");
    names[impl::UnionOperator_id] = SA::construct_from_utf8("union");
    names[impl::UnaryMinusOperator_id] = SA::construct_from_utf8("minus");

    names[impl::Pattern_id] = SA::construct_from_utf8("Pattern");
    names[impl::LocationPathPattern_id] = SA::construct_from_utf8("LocationPathPattern");
    names[impl::IdKeyPattern_id] = SA::construct_from_utf8("IdKeyPattern");
    names[impl::RelativePathPattern_id] = SA::construct_from_utf8("RelativePathPattern");
    names[impl::StepPattern_id] = SA::construct_from_utf8("StepPattern");
    names[impl::ChildOrAttributeAxisSpecifier_id] = SA::construct_from_utf8("ChildOrAttributeAxisSpecifier");

    names[impl::AttributeValueTemplate_id] = SA::construct_from_utf8("AttributeValueTemplate");
    names[impl::DoubleLeftCurly_id] = SA::construct_from_utf8("{{");
    names[impl::DoubleRightCurly_id] = SA::construct_from_utf8("}}");
    names[impl::LeftCurly_id] = SA::construct_from_utf8("{");
    names[impl::RightCurly_id] = SA::construct_from_utf8("}");
    names[impl::EmbeddedExpr_id] = SA::construct_from_utf8("EmbeddedExpr");
    names[impl::AttrLiteral_id] = SA::construct_from_utf8("AttrLiteral");

    return names;
  } // init_debugNames
/*  
  static void dump(typename impl::types<string_adaptor>::node_iter_t const& i, int depth)
  {
    long id = static_cast<long>(i->value.id().to_long());

    for(int d = 0; d < depth; ++d)
      std::cerr << ' ';
    std::cerr << names()[id] << " - " << std::string(i->value.begin(), i->value.end()) << std::endl;

    for(typename impl::types<string_adaptor>::node_iter_t c = i->children.begin(); c != i->children.end(); ++c)
      dump(c, depth+2);
  } // dump
*/
  XPath(const XPath&);
  XPath& operator=(const XPath&);
  bool operator==(const XPath&) const;
}; // class XPath

} // namespace XPath

} // namespace Arabica

#include <boost/lexical_cast.hpp>
#include "xpath_value.hpp"
#include "xpath_arithmetic.hpp"
#include "xpath_relational.hpp"
#include "xpath_logical.hpp"
#include "xpath_step.hpp"
#include "xpath_compile_context.hpp"
#include "xpath_variable.hpp"
#include "xpath_function_holder.hpp"
#include "xpath_union.hpp"

namespace Arabica
{

namespace XPath
{

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createAbsoluteLocationPath(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  return new impl::AbsoluteLocationPath<string_type, string_adaptor>(createStepList(i->children.begin(), i->children.end(), context));
} // createAbsoluteLocationPath

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createRelativeLocationPath(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  return new impl::RelativeLocationPath<string_type, string_adaptor>(createStepList(i->children.begin(), i->children.end(), context));
} // createRelativeLocationPath

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createSingleStepRelativeLocationPath(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  typename impl::types<string_adaptor>::node_iter_t n = i;
  return new impl::RelativeLocationPath<string_type, string_adaptor>(impl::StepFactory<string_type, string_adaptor>::createStep(n, context));
} // createSingleStepRelativeLocationPath

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createExpression(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  typename impl::types<string_adaptor>::node_iter_t c = i->children.begin();
  impl::skipWhitespace<string_adaptor>(c);
  return XPath<string_type, string_adaptor>::compile_expression(c, context);
} // createExpression

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createFunction(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  typename impl::types<string_adaptor>::node_iter_t c = i->children.begin();
  string_type name = string_adaptor::construct(c->value.begin(), c->value.end());
  ++c;
  impl::skipWhitespace<string_adaptor>(c);
  assert(impl::getNodeId<string_adaptor>(c) == impl::LeftBracket_id);
  ++c;
  impl::skipWhitespace<string_adaptor>(c);

  std::vector<XPathExpressionPtr<string_type, string_adaptor> > args;
  while(impl::getNodeId<string_adaptor>(c) != impl::RightBracket_id)
  {
    XPathExpressionPtr<string_type, string_adaptor> arg(XPath<string_type, string_adaptor>::compile_expression(c++, context));
    args.push_back(arg);

    impl::skipWhitespace<string_adaptor>(c);
  } // while ...
  // maybe trailing whitespace ...

  return impl::FunctionHolder<string_type, string_adaptor>::createFunction(name, args, context);
} // createFunction

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createBinaryExpression(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  typename impl::types<string_adaptor>::node_iter_t c = i->children.begin();
  XPathExpression<string_type, string_adaptor>* p1 = XPath<string_type, string_adaptor>::compile_expression(c, context);
  ++c;

  do
  {
    long op = impl::getNodeId<string_adaptor>(c);
    ++c;
    XPathExpression<string_type, string_adaptor>* p2 = XPath<string_type, string_adaptor>::compile_expression(c, context);

    switch(op)
    {
      case impl::PlusOperator_id:
        p1 = new impl::PlusOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::MinusOperator_id:
        p1 = new impl::MinusOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::MultiplyOperator_id:
        p1 = new impl::MultiplyOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::DivOperator_id:
        p1 = new impl::DivideOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::ModOperator_id:
        p1 = new impl::ModOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::EqualsOperator_id:
        p1 = new impl::EqualsOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::NotEqualsOperator_id:
        p1 = new impl::NotEqualsOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::LessThanOperator_id:
        p1 = new impl::LessThanOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::LessThanEqualsOperator_id:
        p1 = new impl::LessThanEqualsOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::GreaterThanOperator_id:
        p1 = new impl::GreaterThanOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::GreaterThanEqualsOperator_id:
        p1 = new impl::GreaterThanEqualsOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::OrOperator_id:
        p1 = new impl::OrOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::AndOperator_id:
        p1 = new impl::AndOperator<string_type, string_adaptor>(p1, p2);
        break;
      case impl::UnionOperator_id:
        p1 = new impl::UnionExpression<string_type, string_adaptor>(p1, p2);
        break;
      default:
        throw UnsupportedException(boost::lexical_cast<std::string>(op));
    } // switch
  } 
  while(++c != i->children.end());

  return p1;
} // createBinaryExpression

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createLiteral(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  string_type str = string_adaptor::construct(i->value.begin(), i->value.end());
  return new StringValue<string_type, string_adaptor>(str);
} // createLiteral

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createNumber(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  string_type str = string_adaptor::construct(i->value.begin(), i->value.end());
  return new NumericValue<string_type, string_adaptor>(boost::lexical_cast<double>(str));
} // createNumber

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createVariable(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  string_type str = string_adaptor::construct(i->value.begin()+1, i->value.end());
  return new Variable<string_type, string_adaptor>(str); 
} // createVariable

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createSingleStepAbsoluteLocationPath(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  typename impl::types<string_adaptor>::node_iter_t n = i;
  return new impl::AbsoluteLocationPath<string_type, string_adaptor>(impl::StepFactory<string_type, string_adaptor>::createStep(n, context));
} // createSingleStepAbsoluteLocationPath

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createUnaryExpression(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  return XPath<string_type, string_adaptor>::compile_expression(i->children.begin(), context);
} // createUnaryExpression

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createUnaryNegativeExpr(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  return new impl::UnaryNegative<string_type, string_adaptor>(XPath<string_type, string_adaptor>::compile_expression(i+1, context));
} // createUnaryNegativeExpr

template<class string_type, class string_adaptor>
impl::StepList<string_type, string_adaptor> XPath<string_type, string_adaptor>::createStepList(typename impl::types<string_adaptor>::node_iter_t const& from, 
                                                            typename impl::types<string_adaptor>::node_iter_t const& to, 
                                                            impl::CompilationContext<string_type, string_adaptor>& context)
{
  impl::StepList<string_type, string_adaptor> steps;

  typename impl::types<string_adaptor>::node_iter_t c = from;
  typename impl::types<string_adaptor>::node_iter_t end = to;

  while(c != end)
    switch(impl::getNodeId<string_adaptor>(c))
    {
      case impl::S_id:
      case impl::Slash_id:
        ++c; // just drop it
        break;
      case impl::RelativeLocationPath_id:
        // might get here when handling an absolute path
        end = c->children.end();
        c = c->children.begin();
        break;
      case impl::Step_id:
        {
          typename impl::types<string_adaptor>::node_iter_t step = c->children.begin();
          steps.push_back(impl::StepFactory<string_type, string_adaptor>::createStep(step, c->children.end(), context));
          ++c;
        }
        break;
      default:
        steps.push_back(impl::StepFactory<string_type, string_adaptor>::createStep(c, end, context));
    } // switch(impl::getNodeId(c))

  return steps;
} // createStepList

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createDocMatch(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  typename impl::types<string_adaptor>::node_iter_t n = i;
  return new impl::RelativeLocationPath<string_type, string_adaptor>(impl::StepFactory<string_type, string_adaptor>::createStep(n, context));
} // createDocMatch

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createSingleMatchStep(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  typename impl::types<string_adaptor>::node_iter_t n = i;

  impl::StepList<string_type, string_adaptor> steps;
  steps.push_back(new impl::TestStepExpression<string_type, string_adaptor>(SELF, new impl::NotAttributeNodeTest<string_type>()));
  steps.push_back(impl::StepFactory<string_type, string_adaptor>::createStep(n, context, SELF));
  return new impl::RelativeLocationPath<string_type, string_adaptor>(steps);

} // createSingleMatchStep

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createRelativePathPattern(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  return new impl::RelativeLocationPath<string_type, string_adaptor>(createPatternList(i->children.begin(), i->children.end(), context));
} // createRelativePathPattern

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createAlternatePattern(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  // child sequence is test union_op test ...
  typename impl::types<string_adaptor>::node_iter_t n = i->children.begin(), e = i->children.end();

  XPathExpression<string_type, string_adaptor>* p1 = compile_match(n++, context);
  while(n != e)
  {
    ++n; // skip |
    p1 = new impl::OrOperator<string_type, string_adaptor>(p1, compile_match(n++, context));
  } // while 

  return p1;
} // createAlternatePattern

template<class string_adaptor>
Axis getPatternAxis(typename impl::types<string_adaptor>::node_iter_t const& from, 
                    typename impl::types<string_adaptor>::node_iter_t const& to)
{
  typename impl::types<string_adaptor>::node_iter_t next = from + 1;
  while((next != to) && (impl::getNodeId<string_adaptor>(next) == impl::Predicate_id))
    ++next;

  if(next == to)
    return SELF;

  int id = impl::getNodeId<string_adaptor>(next);
  
  if(id == impl::SlashSlash_id)
    return ANCESTOR_OR_SELF;
  if((id == impl::AbbreviatedAxisSpecifier_id) || (id == impl::Attribute_id))
    return static_cast<Axis>(-1);

  return PARENT;
} // getPatternAxis

template<class string_type, class string_adaptor>
void createStepsFromPattern(impl::StepList<string_type, string_adaptor>& steps,
                            typename impl::types<string_adaptor>::node_iter_t const& from, 
                            typename impl::types<string_adaptor>::node_iter_t const& to, 
                            impl::CompilationContext<string_type, string_adaptor>& context,
                            Axis override = static_cast<Axis>(-1))
{
  typename impl::types<string_adaptor>::node_iter_t c = from;
  typename impl::types<string_adaptor>::node_iter_t end = to;

  // start
  switch(impl::getNodeId<string_adaptor>(c))
  {
  case impl::Slash_id:
    steps.push_front(impl::StepFactory<string_type, string_adaptor>::createStep(c, context, PARENT));
  case impl::SlashSlash_id:
    ++c;
    break;
  } // switch

  bool is_attr = false;

  while(c != end)
  {
    switch(impl::getNodeId<string_adaptor>(c))
    {
    case impl::StepPattern_id:
      createStepsFromPattern(steps, c->children.begin(), c->children.end(), context, getPatternAxis<string_adaptor>(c, end));
      ++c;
      break;
    case impl::RelativePathPattern_id:
      createStepsFromPattern(steps, c->children.begin(), c->children.end(), context);
      ++c;
      break;
    case impl::Child_id:
    case impl::Slash_id:
    case impl::SlashSlash_id:
      ++c;
      break;
    case impl::AbbreviatedAxisSpecifier_id:
    case impl::Attribute_id:
      is_attr = true;
      ++c;
      break;
    default:
      {
        if(is_attr) 
          steps.push_front(new impl::TestStepExpression<string_type, string_adaptor>(SELF, new impl::AttributeNodeTest<string_type>()));
        else
          steps.push_front(new impl::TestStepExpression<string_type, string_adaptor>(SELF, new impl::NotAttributeNodeTest<string_type>()));

        Axis axis = getPatternAxis<string_adaptor>(c, end);
        if(override != static_cast<Axis>(-1))
          axis = override;
        steps.push_front(impl::StepFactory<string_type, string_adaptor>::createStep(c, end, context, axis));
      }
    } // switch ...
  } // while(c != end)
} // createStepsFromPattern

template<class string_type, class string_adaptor>
impl::StepList<string_type, string_adaptor> XPath<string_type, string_adaptor>::createPatternList(typename impl::types<string_adaptor>::node_iter_t const& from, 
                                                            typename impl::types<string_adaptor>::node_iter_t const& to, 
                                                            impl::CompilationContext<string_type, string_adaptor>& context)
{
  impl::StepList<string_type, string_adaptor> steps;

  typename impl::types<string_adaptor>::node_iter_t c = from;

  // start
  switch(impl::getNodeId<string_adaptor>(c))
  {
  case impl::Slash_id:
    steps.push_front(impl::StepFactory<string_type, string_adaptor>::createStep(c, context, PARENT));
  case impl::SlashSlash_id:
    ++c;
    break;
  } // switch

  createStepsFromPattern(steps, c, to, context);

  return steps;
} // createPatternList


template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createAttributeValue(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  std::vector<XPathExpressionPtr<string_type, string_adaptor> > args;
  for(typename impl::types<string_adaptor>::node_iter_t a = i->children.begin(), e = i->children.end(); a != e; ++a)
  {
    XPathExpressionPtr<string_type, string_adaptor> arg(XPath<string_type, string_adaptor>::compile_attribute_value(a, context));
    args.push_back(arg);
  } // while ...
  // maybe trailing whitespace ...

  return impl::FunctionHolder<string_type, string_adaptor>::createFunction(string_adaptor::construct_from_utf8("concat"), args, context);
} // createAttributeValue

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createEmbeddedExpr(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  return compile_expression(i->children.begin() + 1, context);
} // createEmbeddedExpr

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createDoubleLeftCurly(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  return new StringValue<string_type, string_adaptor>("{");
} // createDoubleLeftCurly

template<class string_type, class string_adaptor>
XPathExpression<string_type, string_adaptor>* XPath<string_type, string_adaptor>::createDoubleRightCurly(typename impl::types<string_adaptor>::node_iter_t const& i, impl::CompilationContext<string_type, string_adaptor>& context)
{
  return new StringValue<string_type, string_adaptor>("}");
} // createDoubleRightCurly


} // namespace XPath
} // namespace Arabica

#endif