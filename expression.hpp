/*! \file expression.hpp
Defines the Expression type and assiciated functions.
 */
#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>
#include <vector>
#include <list>
#include <map>

#include "token.hpp"
#include "atom.hpp"

// forward declare Environment
class Environment;

/*! \class Expression
\brief An expression is a tree of Atoms.

An expression is an atom called the head followed by a (possibly empty) 
list of expressions called the tail.
 */
class Expression {
public:

  typedef std::vector<Expression>::const_iterator ConstIteratorType;

  /// Default construct and Expression, whose type in NoneType
  Expression();

  /*! Construct an Expression with given Atom as head an empty tail
    \param atom the atom to make the head
  */
  Expression(const Atom & a);

  /// deep-copy construct an expression (recursive)
  Expression(const Expression & a);

  /// construct a list of expressions
  Expression(const std::list<Expression> & a);

  /// construct a vector of expressions
  Expression(const std::vector<Expression> & a);
  
  /// deep-copy assign an expression  (recursive)
  Expression & operator=(const Expression & a);

  /// return a reference to the head Atom
  Atom & head();

  /// return a const-reference to the head Atom
  const Atom & head() const;

  /// append Atom to tail of the expression
  void append(const Atom & a);

  /// return a pointer to the last expression in the tail, or nullptr
  Expression * tail();
  
  /// return "object-name" property
  std::string objName();
  
  /// return "size" property for point
  double pointSize();
    
  /// return "thickness" property for line
  double lineThick();
  
  /// return "position" property for text
  Expression textPos();
  
  /// return "scale" property for text
  double textScale();
  
  ///return "rotation" property for text
  double textRotation();

  /// return a const-iterator to the beginning of tail
  ConstIteratorType tailConstBegin() const noexcept;

  /// return a const-iterator to the tail end
  ConstIteratorType tailConstEnd() const noexcept;

  /// convienience member to determine if head atom is a number
  bool isHeadNumber() const noexcept;

  /// convienience member to determine if head atom is a symbol
  bool isHeadSymbol() const noexcept;
  
  /// convienience member to determine if head atom is a complex
  bool isHeadComplex() const noexcept;

  /// Evaluate expression using a post-order traversal (recursive)
  Expression eval(Environment & env);

  /// Evalutate expression from lambda
  Expression eval_lambda(const std::vector<Expression> & args, Environment & env);

  /// steps used in map and apply
  std::vector<Expression> eval_app_map(Environment & env, Expression arguments);
  
  /// the property list
  std::map<std::string, Expression> m_prop;

  /// equality comparison for two expressions (recursive)
  bool operator==(const Expression & exp) const noexcept;
  
private:

  // the head of the expression
  Atom m_head;

  // the tail list is expressed as a vector for access efficiency
  // and cache coherence, at the cost of wasted memory.
  std::vector<Expression> m_tail;

  // convenience typedef
  typedef std::vector<Expression>::iterator IteratorType;
  
  // internal helper methods
  Expression handle_lookup(const Atom & head, const Environment & env);
  Expression handle_define(Environment & env);
  Expression handle_begin(Environment & env);
  Expression handle_lambda(Environment & env);
  Expression handle_setprop(const std::vector<Expression> & args);
  Expression handle_getprop(const std::vector<Expression> & args);
  Expression handle_discrete(const std::vector<Expression> & args);
  Expression handle_continuous(const std::vector<Expression> & args, Environment & env);

  Expression make_point(double x, double y, double size);
  Expression make_line(double x1, double y1, double x2, double y2, double thickness);
  Expression make_text(double x, double y, std::string text, double scale, double rotation);
};

/// Render expression to output stream
std::ostream & operator<<(std::ostream & out, const Expression & exp);

/// inequality comparison for two expressions (recursive)
bool operator!=(const Expression & left, const Expression & right) noexcept;
  
#endif
