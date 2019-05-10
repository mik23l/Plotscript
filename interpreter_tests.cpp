#include "catch.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <complex>
#include <list>
#include <thread>

#include "semantic_error.hpp"
#include "interpreter.hpp"
#include "expression.hpp"
#include "message_queue.hpp"
#include "consumer.hpp"

Expression run(const std::string & program){
  
  std::istringstream iss(program);
    
  Interpreter interp;
    
  bool ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  Expression result;
  REQUIRE_NOTHROW(result = interp.evaluate());

  return result;
}

TEST_CASE( "Test Interpreter parser with expected input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r)))";

  std::istringstream iss(program);
 
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == true);
}

TEST_CASE( "Test Interpreter parser with numerical literals", "[interpreter]" ) {

  std::vector<std::string> programs = {"(1)", "(+1)", "(+1e+0)", "(1e-0)"};
  
  for(auto program : programs){
    std::istringstream iss(program);
 
    Interpreter interp;

    bool ok = interp.parseStream(iss);

    REQUIRE(ok == true);
  }

  {
    std::istringstream iss("(define x 1abc)");
    
    Interpreter interp;

    bool ok = interp.parseStream(iss);

    REQUIRE(ok == false);
  }
}

TEST_CASE( "Test Interpreter parser with truncated input", "[interpreter]" ) {

  {
    std::string program = "(f";
    std::istringstream iss(program);
  
    Interpreter interp;
    bool ok = interp.parseStream(iss);
    REQUIRE(ok == false);
  }
  
  {
    std::string program = "(begin (define r 10) (* pi (* r r";
    std::istringstream iss(program);

    Interpreter interp;
    bool ok = interp.parseStream(iss);
    REQUIRE(ok == false);
  }
}

TEST_CASE( "Test Interpreter parser with extra input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r))) )";
  std::istringstream iss(program);

  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with single non-keyword", "[interpreter]" ) {

  std::string program = "hello";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with empty input", "[interpreter]" ) {

  std::string program;
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with empty expression", "[interpreter]" ) {

  std::string program = "( )";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with bad number string", "[interpreter]" ) {

  std::string program = "(1abc)";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with incorrect input. Regression Test", "[interpreter]" ) {

  std::string program = "(+ 1 2) (+ 3 4)";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter result with literal expressions", "[interpreter]" ) {
  
  { // Number
    std::string program = "(4)";
    Expression result = run(program);
    REQUIRE(result == Expression(4.));
  }

  { // Symbol
    std::string program = "(pi)";
    Expression result = run(program);
    REQUIRE(result == Expression(atan2(0, -1)));
  }
  
  { // Symbol
    std::string program = "(e)";
    Expression result = run(program);
    REQUIRE(result == Expression(exp(1)));
  }

}

TEST_CASE( "Test Interpreter result with simple procedures (add)", "[interpreter]" ) {

  { // add, binary case
    std::string program = "(+ 1 2)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(3.));
  }
  
  { // add, 3-ary case
    std::string program = "(+ 1 2 3)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(6.));
  }

  { // add, 6-ary case
    std::string program = "(+ 1 2 3 4 5 6)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(21.));
  }
}
  
TEST_CASE( "Test Interpreter special forms: begin and define", "[interpreter]" ) {

  {
    std::string program = "(define answer 42)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(42.));
  }

  {
    std::string program = "(begin (define answer 42)\n(answer))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(42.));
  }
  
  {
    std::string program = "(begin (define answer (+ 9 11)) (answer))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(20.));
  }

  {
    std::string program = "(begin (define a 1) (define b 1) (+ a b))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(2.));
  }
  
  { //define error
	std::string input = "(define 4 s)";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //define error
	std::string input = "(define ^ 3)";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //procedure error
	std::string input = "(s 2)";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "Test a medium-sized expression", "[interpreter]" ) {

  {
    std::string program = "(+ (+ 10 1) (+ 30 (+ 1 1)))";
    Expression result = run(program);
    REQUIRE(result == Expression(43.));
  }
}

TEST_CASE( "Test arithmetic procedures", "[interpreter]" ) {

  {
    std::vector<std::string> programs = {"(+ 1 -2)",
					 "(+ -3 1 1)",
					 "(- 1)",
					 "(- 1 2)",
					 "(* 1 -1)",
					 "(* 1 1 -1)",
					 "(/ -1 1)",
					 "(/ -1)",
					 "(/ 1 -1)"};

    for(auto s : programs){
      Expression result = run(s);
      REQUIRE(result == Expression(-1.));
    }
  }
  
  { //sqrt
    std::string program = "(sqrt 4)";
    Expression result = run(program);
    REQUIRE(result == Expression(2.));
  }
  
  { //exponent
    std::string program = "(^ 4 2)";
    Expression result = run(program);
    REQUIRE(result == Expression(16.));
  }
  
  { //ln
    std::string program = "(ln e)";
    Expression result = run(program);
    REQUIRE(result == Expression(1.));
  }
  
  { //sin, cos, tan
    std::string program1 = "(sin 0)";
    Expression result1 = run(program1);
	
	std::string program2 = "(cos 0)";
    Expression result2 = run(program2);
	
	std::string program3 = "(tan 0)";
    Expression result3 = run(program3);
	
	
    REQUIRE(result1 == Expression(0.));
	REQUIRE(result2 == Expression(1.));
	REQUIRE(result3 == Expression(0.));
  }
}

TEST_CASE( "Test arithmetic procedures with complex", "[interpreter]" ) {
	
	{ //add
    std::string program = "(+ I 7 I)";
    Expression result = run(program);
	std::complex<double> testresult(7,2);
	REQUIRE(result == Expression(testresult));
	}
	
	{ //sub
    std::string program1 = "(- I)";
    Expression result1 = run(program1);
	std::complex<double> testresult1(-0,-1);
	
	std::string program2 = "(- I 9)";
    Expression result2 = run(program2);
	std::complex<double> testresult2(-9,1);
	
	std::string program3 = "(- 23 I)";
    Expression result3 = run(program3);
	std::complex<double> testresult3(23,-1);
	
	std::string program4 = "(- I I)";
    Expression result4 = run(program4);
	std::complex<double> testresult4(0,0);
	
	
    REQUIRE(result1 == Expression(testresult1));
	REQUIRE(result2 == Expression(testresult2));
	REQUIRE(result3 == Expression(testresult3));
	REQUIRE(result4 == Expression(testresult4));
  }
  
  { //div
    std::string program1 = "(/ I 4)";
    Expression result1 = run(program1);
	std::complex<double> testresult1(0,0.25);
	
	std::string program2 = "(/ 23 I)";
    Expression result2 = run(program2);
	std::complex<double> testresult2(0,-23);
	
	std::string program3 = "(/ I I)";
    Expression result3 = run(program3);
	std::complex<double> testresult3(1,0);
	
	std::string program4 = "(/ I)";
    Expression result4 = run(program4);
	std::complex<double> testresult4(0,-1);
	
	
    REQUIRE(result1 == Expression(testresult1));
	REQUIRE(result2 == Expression(testresult2));
	REQUIRE(result3 == Expression(testresult3));
	REQUIRE(result4 == Expression(testresult4));
  }
  
  { //sqrt
    std::string program1 = "(sqrt -1)";
    Expression result1 = run(program1);
	std::complex<double> testresult1(0,1);
	
	std::string program2 = "(sqrt -4)";
    Expression result2 = run(program2);
	std::complex<double> testresult2(2,1);
	
	std::string program3 = "(sqrt (* 50 I))";
    Expression result3 = run(program3);
	std::complex<double> testresult3(5,5);
	
	
    REQUIRE(result1 == Expression(testresult1));
	REQUIRE(result2 == Expression(testresult2));
	REQUIRE(result3 == Expression(testresult3));
  }
  
  { //expo
   	std::string program1 = "(begin (define x (^ I 2)) (real x))";
    INFO(program1);
    Expression result1 = run(program1);
    REQUIRE(result1 == Expression(-1.));
	
	std::string program2 = "(^ 4 I)";
    Expression result2 = run(program2);
	std::complex<double> testresult2(5,5);
	
	std::string program3 = "(^ I I)";
    Expression result3 = run(program3);
	std::complex<double> testresult3(5,5);
  }
  
  { //real
    std::string program = "(real I)";
    Expression result = run(program);
    REQUIRE(result == Expression(0.));
  }
  
  { //imag
    std::string program = "(imag I)";
    Expression result = run(program);
    REQUIRE(result == Expression(1.));
  }
  
  { //mag
    std::string program = "(begin (define x (+ 3 (* 4 I))) (mag x))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(5.));
  }
  
  { //arg
    std::string program = "(begin (define x (+ 1 (- I I))) (arg x))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(0.));
  }
  
  { //conj
    std::string program = "(conj I)";
    Expression result = run(program);
    std::complex<double> testresult1(0,-1);
    REQUIRE(result == Expression(testresult1));
  }
}


TEST_CASE( "Test some semantically invalid expressions", "[interpreter]" ) {
  
  std::vector<std::string> programs = {"(@ none)", // so such procedure
				       "(- 1 1 2)", // too many arguments
					   "(/ 1 1 2)", // too many arguments
					   "(+ (list) 4)", // add invalid argument
					   "(* (list) 4)", // mul invalid argument
					   "(- (list) 4)", // sub invalid argument
					   "(- (list))", // neg invalid argument
					   "(/ (list) 4)", // div invalid argument
					   "(/ (list))", // div invalid argument
					   "(sqrt (list))", // sqrt invalid argument
					   "(sqrt 1 2)", // too many arguments
					   "(sqrt a)", // not positive, complex, or negative
					   "(^ 1 1 2)", // too many arguments
					   "(^ a 50)", // not a complex or number
					   "(^ (list) 3)", //invalid argument
					   "(ln 4 2)", // too many arguments
					   "(ln -53)", // not a positive number
					   "(ln I)", // not a number
					   "(ln (list))", //invalid argument
					   "(sin pi 2)", // too many arguments
					   "(cos pi 2)", // too many arguments
					   "(tan pi 2)", // too many arguments
					   "(sin (list))", //invalid argument
					   "(cos (list))", //invalid argument
					   "(tan (list))", //invalid argument
					   "(sin I)", // not a number
					   "(cos I)", // not a number
					   "(tan I)", // not a number
					   "(real (list))", //invalid argument
					   "(real I 4)", // too many arguments
					   "(imag (list))", //invalid argument
					   "(imag I 4)", // too many arguments
					   "(mag I 4)", // too many arguments
					   "(arg I 4)", // too many arguments
					   "(conj I 4)", // too many arguments
					   "(real 4)", // not complex
					   "(imag 4)", // not complex
					   "(mag 4)", // not complex
					   "(arg 4)", // not complex
					   "(conj 4)", // not complex
				       "(define begin 1)", // redefine special form
				       "(define pi 3.14)"}; // redefine builtin symbol
    for(auto s : programs){
      Interpreter interp;

      std::istringstream iss(s);
      
      bool ok = interp.parseStream(iss);
      REQUIRE(ok == true);
      
      REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE( "Test list creation and procedures", "[interpreter]" ) {

  { //create list
    std::string program = "(begin (define mylist (list 1 (+ 1 I) (list 5 4 3 2 1) (list))) (mylist))";
	INFO(program);
	Expression result = run(program);
	std::complex<double> complexPart(1,1);
	std::list<Expression> listPart = {Expression(5), Expression(4), Expression(3), Expression(2), Expression(1)};
	std::list<Expression> listPart2 = {};
	std::list<Expression> testresult = {Expression(1), Expression(complexPart), Expression(listPart), Expression(listPart2)};
	REQUIRE(result == Expression(testresult));
  }
  
  { //create list
    std::string program = "(begin (define mylist (list 1 (list 5 4 3 2 1 (list (+ 1 I))) (list (+ 1 I)))) (mylist))";
	INFO(program);
	Expression result = run(program);
	std::complex<double> complexPart(1,1);
	std::list<Expression> listPart2 = {Expression(complexPart)};
	std::list<Expression> listPart = {Expression(5), Expression(4), Expression(3), Expression(2), Expression(1), Expression(listPart2)};
	std::list<Expression> testresult = {Expression(1), Expression(listPart), Expression(listPart2)};
	REQUIRE(result == Expression(testresult));
  }
  
  { //first
	std::string program = "(first (list 1 2 3))";
    Expression result = run(program);
    REQUIRE(result == Expression(1));  
  }
  
  { //rest
	std::string program = "(rest (list 1 2 3))";
    Expression result = run(program);
    std::list<Expression> testresult = { Expression(2), Expression(3) };
    REQUIRE(result == Expression(testresult));  
  }
  
  { //length
	std::string program = "(length (list 1 2 3 4))";
    Expression result = run(program);
    REQUIRE(result == Expression(4));
  }
  
  { //append complex
    std::string program = "(begin (define x (list 0 1 2 3)) (define y (append x (+ 3 (* 4 I)))))";
    INFO(program);
    Expression result = run(program);
	std::complex<double> complexPart(3,4);
    std::list<Expression> testresult = { Expression(0), Expression(1), Expression(2), Expression(3), Expression(complexPart) };
    REQUIRE(result == Expression(testresult));
  }
  
  { //append number
    std::string program = "(begin (define x (list 0 1 2 3)) (define y (append x (4))))";
    INFO(program);
    Expression result = run(program);
    std::list<Expression> testresult = { Expression(0), Expression(1), Expression(2), Expression(3), Expression(4) };
    REQUIRE(result == Expression(testresult));
  }
  
  { //append list
    std::string program = "(begin (define x (list 0 1 2 3)) (define y (append x x)))";
    INFO(program);
    Expression result = run(program);
	std::list<Expression> listPart = { Expression(0), Expression(1), Expression(2), Expression(3) };
    std::list<Expression> testresult = { Expression(0), Expression(1), Expression(2), Expression(3), Expression(listPart) };
    REQUIRE(result == Expression(testresult));
  }
  
  { //join
    std::string program = "(begin (define x (list 0 1 2 3)) (define y (list (+ 3 I) 100 110)) (define z (join x y)))";
    INFO(program);
    Expression result = run(program);
	std::complex<double> complexPart(3,1);
    std::list<Expression> testresult = { Expression(0), Expression(1), Expression(2), Expression(3), Expression(complexPart), Expression(100), Expression(110) };
    REQUIRE(result == Expression(testresult));
  }
  
  { //range
	std::string program = "(range -2 2 1)";
    Expression result = run(program);
    std::list<Expression> testresult = { Expression(-2), Expression(-1), Expression(0), Expression(1), Expression(2) };
    REQUIRE(result == Expression(testresult));  
  }
}

TEST_CASE( "Test some semantically invalid expressions for list", "[interpreter]" ) {
	
	std::vector<std::string> programs = {"(@ none)", // so such procedure
				       "(first (list 1 2) (list 3 4))", // too many arguments
					   "(first (list))", // empty list
					   "(first (1))", // not a list
					   "(rest (list 1 2) (list 3 4))", // too many arguments
					   "(rest (list))", // empty list
					   "(rest (1))", // not a list
					   "(length (list 1 2) (list 3 4))", // too many arguments
					   "(length (1))", // not a list
					   "(append (list 1 2) (list 3 4) (4))", // too many arguments
					   "(append 2 8)", // first not a list
					   "(join (list 1 2) (list 3 4) (list 3))", // too many arguments
					   "(join (list 1 2) 10)", // not a list
					   "(join 10 (list 1 2))", // not a list
					   "(range 3 -1 1)", // begin less than end
					   "(range 0 5 -1)", // increment not positive
					   "(range 0 I -1)", // invalid argument
				       "(range 0 5 4 3)"}; // too many arguments
	for(auto s : programs){
      Interpreter interp;

      std::istringstream iss(s);
      
      bool ok = interp.parseStream(iss);
      REQUIRE(ok == true);
      
      REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE( "Test lambda", "[interpreter]" ) {
  
  { //evaluate lambda
    std::string program = "(begin (define a 1) (define x 100) (define f (lambda (x) (begin (define b 12) (+ a b x 1)))) (f 2))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(16.));		
  }
  
  { //evaluate lambda
    std::string program = "(begin (define f (lambda (x y) (* x y))) (f 2 2))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(4.));		
  }
  
  { //evaluate lambda
	std::string program = "(begin (define f (lambda (x y z) (* x y z))) (f 2 2 2))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(8.));	
  }
  
  { //print lambda
	std::string program = "(lambda (x) (* x 2))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(Expression(0.) == Expression(0.));
  }
  
  { //print lambda
	std::string program = "(lambda (x) (* 2))";
    INFO(program);
    Expression result = run(program);
	REQUIRE(result.head().asSymbol() == "lambda");
  }
  
  { //print
    std::string program = "(lambda (x) (x))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE();	
  }
 
}

TEST_CASE( "Test some semantically invalid expressions for lambda", "[interpreter]" ) {
	
	std::vector<std::string> programs = {"(@ none)", // so such procedure
				       "(lambda (x) (x) (x))", // invalid number argument
					   "(lambda (2) (* 2 x))", // invalid argument
					   "(lambda (x 2) (* 2 x))", // invalid argument
					   "(lambda (define) (* 2 x))", // invalid argument
					   "(lambda (x define) (* 2 x))", // invalid argument
					   "(define a (lambda (x) (x) (x)))", // invalid number argument
					   "(define a (lambda (2) (* 2 x)))", // invalid argument
					   "(define a (lambda (x 2) (* 2 x)))", // invalid argument
					   "(define a (lambda (define) (* 2 x)))", // invalid argument
					   "(define a (lambda (x define) (* 2 x)))"}; // invalid argument
	
	for(auto s : programs){
      Interpreter interp;

      std::istringstream iss(s);
      
      bool ok = interp.parseStream(iss);
      REQUIRE(ok == true);
      
      REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE( "Test apply", "[interpreter]" ) {
	
  { //apply 
	std::string program = "(apply + (list 1 2 3 4))";
    Expression result = run(program);
    REQUIRE(result == Expression(10.));  
  }
  
  { //with lambda
	std::string program = "(begin (define complexAsList (lambda (x) (list (real x) (imag x)))) (apply complexAsList (list (+ 1 (* 3 I)))))";
    INFO(program);
	Expression result = run(program);
    std::list<Expression> testresult = { Expression(1), Expression(3) };
    REQUIRE(result == Expression(testresult));
  }
   
  { //with lambda
	std::string program = "(begin (define linear (lambda (a b x) (+ (* a x) (+ 4) b))) (apply linear (list 3 4 5)))";
    INFO(program);
	Expression result = run(program);
    REQUIRE(result == Expression(23.));
  }
  
  { //with lambda
	std::string program = "(begin (define a 1) (define x 100) (define f (lambda (x) (begin (define b 12) (+ a b x)))) (apply f (list 2)))";
    INFO(program);
	Expression result = run(program);
    REQUIRE(result == Expression(15.));
  }
   
  { //apply second argument error
	std::string input = "(apply + 3)";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //apply second argument error
	std::string input = "(begin (define linear (lambda (a b x) (+ (* a x) (+ 4) b))) (apply linear 3))";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //apply first argument error
	std::string input = "(apply (+ z I) (list 0))";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //apply invalid number of arguments
	std::string input = "(apply / (list 1 2 4))";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //apply invalid procedure
	std::string input = "(apply x (list 1 2 4))";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //apply invalid arguments
	std::string input = "(begin (lambda (x y) (/x y)) (apply div (list 1 2 4)))";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "Test map", "[interpreter]" ) {
	
  { //map 
	std::string program = "(map sin (list (/ (- pi) 2) 0 (/ pi 2)))";
    Expression result = run(program);
	std::list<Expression> testresult = { Expression(-1), Expression(0), Expression(1) };
    REQUIRE(result == Expression(testresult));
  }
  
  { //map
	std::string program = "(map + (list I 3 2))";
    Expression result = run(program);
	std::complex<double> complexPart(0,1);
	std::list<Expression> testresult = { Expression(complexPart), Expression(3), Expression(2) };
    REQUIRE(result == Expression(testresult));
  }
  
  {  //with lambda
	std::string program = "(begin (define f (lambda (x) (/ x))) (map f (list 1 2 4)))";
    INFO(program);
	Expression result = run(program);
    std::list<Expression> testresult = { Expression(1), Expression(0.5), Expression(0.25) };
    REQUIRE(result == Expression(testresult));
  }
  
  {  //with lambda
	std::string program = "(begin (define f (lambda (x) (+ x))) (map f (list 1 2 I)))";
    INFO(program);
	Expression result = run(program);
	std::complex<double> complexPart(0,1);
    std::list<Expression> testresult = { Expression(1), Expression(2), Expression(complexPart) };
    REQUIRE(result == Expression(testresult));
  }
  
  {  //with lambda
	std::string program = "(begin (define x 100) (define f (lambda (x) (+ x))) (map f (list 1 2 I)))";
    INFO(program);
	Expression result = run(program);
	std::complex<double> complexPart(0,1);
    std::list<Expression> testresult = { Expression(1), Expression(2), Expression(complexPart) };
    //REQUIRE(result == Expression(testresult));
  }
  
  {  //with lambda
	std::string program = "(begin (define x 100) (define f (lambda (x) (+ x))) (map f (range 0 1 0.3)))";
    INFO(program);
	Expression result = run(program);
	std::complex<double> complexPart(0,1);
    std::list<Expression> testresult = { Expression(1), Expression(2), Expression(complexPart) };
    //REQUIRE(result == Expression(testresult));
  }
  
  { //map second argument error
	std::string input = "(map + 3)";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //map first argument error
	std::string input = "(apply 3 (list 1 2 3))";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //map invalid number of arguments
	std::string input = "(map ^ )";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //map invalid number of arguments
	std::string input = "(begin (define addtwo (lambda (x y) (+ x y))) (map addtwo (list 1 2 3)))";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //map invalid number of arguments
	std::string input = "(begin (map (lambda (x y) (+ x y)) (3)))";

	Interpreter interp;
  
	std::istringstream iss(input);
  
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
  
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "Test literal string", "[interpreter]" ) {
  
  { 
    std::string program = "(\"this is a string\")";
    INFO(program);
    Expression result = run(program);
	std::string s = "\"this is a string\"";
    REQUIRE(result == Expression(s));
  }
}

TEST_CASE( "Test set-property and get-property", "[interpreter]" ) {

  { //set-property
	std::string program = "(set-property \"number\" \"three\" (3))";
    INFO(program);
    Expression result = run(program);
	REQUIRE(result == Expression(3.));
  }
  
  { //set-property multiple
	std::string program = "(begin (define a (set-property \"name\" \"eight\" 8)) (define a (set-property \"size\" 8 a)))";
    INFO(program);
    Expression result = run(program);
	REQUIRE(result == Expression(8.));
  }
  
  { //set-property argument number error
	std::string input = "(set-property \"number\" \"three\")";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //set-property 1st argument error
	std::string input = "(set-property number \"three\" (3))";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //set-property 1st argument error
	std::string input = "(set-property (+ 1 2) \"three\" (3))";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //get-property
	std::string program = "(begin (define a (+ 1 I)) (define b (set-property \"note\" \"a complex number\" a)) (get-property \"note\" b))";
    INFO(program);
    Expression result = run(program);
	std::string s = "\"a complex number\"";
    REQUIRE(result == Expression(s));
  }
  
  { //get-property argument number error
	std::string input = "(get-property \"number\")";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //get-property 1st argument error
	std::string input = "(get-property (+ 1 2) \"head\")";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //get-property 1st argument error
	std::string input = "(get-property number 5)";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "Test discrete-plot", "[interpreter]" ) {
	
  { //graph with x and y axis
    std::string program = "(begin (define a (lambda (x) (list x (+ (* 2 x) 1)))) (discrete-plot (map a (range -2 2 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 1))))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE(result == Expression(testresult));
  }
  
  { //graph with x and y axis
    std::string program = "(begin (define c (lambda (x) (list x (+ (* x x) 1)))) (discrete-plot (map c (range -2 2 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 3))))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE(result == Expression(testresult));
  }  
  
  { //graph without an axis
    std::string program = "(begin (define c (lambda (x) (list x (+ (* x x) 1)))) (discrete-plot (map c (range 1 2 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 3))))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE(result == Expression(testresult));
  } 

  { //graph without an axis
    std::string program = "(begin (define c (lambda (x) (list x (+ (* x x) 1)))) (discrete-plot (map c (range -5 -1 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 3))))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE(result == Expression(testresult));
  }
    
  { //graph with x and y axis
    std::string program = "(begin (define a (lambda (x) (list x (+ (* -2 x) 1)))) (discrete-plot (map a (range -2 -1 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 1))))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE(result == Expression(testresult));
  }
  
  { //graph with x and y axis
    std::string program = "(begin (define a (lambda (x) (list x (+ (* -2 x) 1)))) (discrete-plot (map a (range 1 5 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 1))))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE(result == Expression(testresult));
  }
    
  { //number of arguments error
	std::string input = "(begin (define a (lambda (x) (list x (+ (* 2 x) 1)))) (discrete-plot (map a (range -2 2 0.5)) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 1)) (list)))";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //1st must be list
	std::string input = "(begin (define a (lambda (x) (list x (+ (* 2 x) 1)))) (discrete-plot (4) (list (list \"title\" \"The Data\") (list \"abscissa-label\" \"X Label\") (list \"ordinate-label\" \"Y Label\") (list \"text-scale\" 1))))";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //number of arguments error
	std::string input = "(begin (define a (lambda (x) (list x (+ (* 2 x) 1)))) (discrete-plot (map a (range -2 2 0.5)) (4)))";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

TEST_CASE( "Test continuous-plot", "[interpreter]" ) {
  
  { //graph with x and y axis
    std::string program = "(begin (define j (lambda (x) (+ (* 2 x) 1))) (continuous-plot j (list -2 2) (list (list \"title\" \"The linear function y=2x+1\") (list \"abscissa-label\" \"x\") (list \"ordinate-label\" \"y\") (list \"text-scale\" 2))))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE(result == Expression(testresult));
  }
  
  { //without labels
    std::string program = "(begin (define f (lambda (x) (sin x))) (continuous-plot f (list (-pi) pi)))";
	INFO(program);
	Expression result = run(program);
	//REQUIRE(result == Expression(testresult));
  }
  
  { //argument number error
	std::string input = "(begin (define j (lambda (x) (+ (* 2 x) 1))) (continuous-plot (list -2 2)))";
	Interpreter interp;
	std::istringstream iss(input); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //first must be lambda
    std::string program = "(begin (define j (lambda (x) (+ (* 2 x) 1))) (continuous-plot / (list -2 2) (list (list \"title\" \"The linear function y=2x+1\") (list \"abscissa-label\" \"x\") (list \"ordinate-label\" \"y\") (list \"text-scale\" 2))))";
	Interpreter interp;
	std::istringstream iss(program); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //lambda must be single variable
    std::string program = "(begin (define j (lambda (x y) (+ (* 2 x) 1))) (continuous-plot j (list -2 2) (list (list \"title\" \"The linear function y=2x+1\") (list \"abscissa-label\" \"x\") (list \"ordinate-label\" \"y\") (list \"text-scale\" 2))))";
	Interpreter interp;
	std::istringstream iss(program); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //second must be list
    std::string program = "(begin (define j (lambda (x) (+ (* 2 x) 1))) (continuous-plot j (2) (list (list \"title\" \"The linear function y=2x+1\") (list \"abscissa-label\" \"x\") (list \"ordinate-label\" \"y\") (list \"text-scale\" 2))))";
	Interpreter interp;
	std::istringstream iss(program); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
  
  { //third must be list
    std::string program = "(begin (define j (lambda (x) (+ (* 2 x) 1))) (continuous-plot j (list -2 2) (4)))";
	Interpreter interp;
	std::istringstream iss(program); 
	bool ok = interp.parseStream(iss);
	REQUIRE(ok == true);
	REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
  }
}

void worker1(inputQueue & inQ){
	
  for(int i = 0; i < 15; ++i){
	  std::string line = "Hello";
	  inQ.push(line);
  }
}

void worker2(outputQueue & outQ){
	
  for(int i = 0; i < 15; ++i){
	  std::string str = "Hello";
	  Expression line = Atom(str);
	  outQ.push(line);
  }
}

TEST_CASE( "Test thread safe message queue", "[interpreter]" ) {
  
  message_queue<std::string> inputQueue;
  message_queue<Expression> outputQueue;
  
  std::thread th1(worker1, std::ref(inputQueue));
  std::thread th2(worker2, std::ref(outputQueue));
  
  while(true){
	  if(inputQueue.size() == 15){
		  break;
	  }
  }
  
  while(true){
	  if(outputQueue.size() == 15){
		  break;
	  }
  }
  
  th1.join();
  th2.join();
}

TEST_CASE( "Test for exceptions from semantically incorrect input", "[interpreter]" ) {

  std::string input = R"((+ 1 a))";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "Test malformed define", "[interpreter]" ) {

  std::string input = R"((define a 1 2))";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "Test using number as procedure", "[interpreter]" ) {
    std::string input = R"(
(1 2 3)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}
