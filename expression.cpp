#include "expression.hpp"

#include <sstream>
#include <list>
#include <iomanip>

#include "environment.hpp"
#include "semantic_error.hpp"

Expression::Expression(){}

Expression::Expression(const Atom & a){

  m_head = a;
}

// recursive copy
Expression::Expression(const Expression & a){

  m_head = a.m_head;
  for(auto e : a.m_tail){
    m_tail.push_back(e);
  }
  m_prop = a.m_prop;
}

Expression::Expression(const std::list<Expression>& a) {

	m_head = Atom("List");
	m_tail.clear();
	for (auto it = a.begin(); it != a.end(); ++it) {
		m_tail.push_back(*it);
	}
}

Expression::Expression(const std::vector<Expression>& a) {

	m_head = Atom("lambda");
	m_tail.clear();
	for (auto it = a.begin(); it != a.end(); ++it) {
		m_tail.push_back(*it);
	}
}

Expression & Expression::operator=(const Expression & a){

  // prevent self-assignment
  if(this != &a){
    m_head = a.m_head;
    m_tail.clear();
    for(auto e : a.m_tail){
      m_tail.push_back(e);
    } 
	m_prop = a.m_prop;
  }
  
  return *this;
}


Atom & Expression::head(){
  return m_head;
}

const Atom & Expression::head() const{
  return m_head;
}

bool Expression::isHeadNumber() const noexcept{
  return m_head.isNumber();
}

bool Expression::isHeadSymbol() const noexcept{
  return m_head.isSymbol();
}  

bool Expression::isHeadComplex() const noexcept{
  return m_head.isComplex();
} 

void Expression::append(const Atom & a){
  m_tail.emplace_back(a);
}


Expression * Expression::tail(){
  Expression * ptr = nullptr;
  
  if(m_tail.size() > 0){
    ptr = &m_tail.back();
  }

  return ptr;
}

std::string Expression::objName()
{
	std::string name = "NONE";

	if (m_prop.size() > 0) {
		auto result = m_prop.find("\"object-name\"");
		if (result != m_prop.end()) {
			name = (result->second).head().asSymbol();
		}
	}

	return name;
}

double Expression::pointSize()
{
	double size = 0;
	
	if (m_prop.size() > 0) {
		auto result = m_prop.find("\"size\"");
		if (result != m_prop.end()) {
			size = (result->second).head().asNumber();
		}
	}

	return size;
}

double Expression::lineThick()
{
	double thick = 0;
	
	if (m_prop.size() > 0) {
		auto result = m_prop.find("\"thickness\"");
		if (result != m_prop.end()) {
			thick = (result->second).head().asNumber();
		}
	}

	return thick;
}

Expression Expression::textPos()
{
	Expression pos;
	
	if (m_prop.size() > 0) {
		auto result = m_prop.find("\"position\"");
		if (result != m_prop.end()) {
			pos = result->second;
		}
	}

	return pos;
}

double Expression::textScale()
{
	double scale = 1;
	
	if (m_prop.size() > 0) {
		auto result = m_prop.find("\"scale\"");
		if (result != m_prop.end()) {
			scale = (result->second).head().asNumber();
		}
	}

	return scale;
}

double Expression::textRotation()
{
	double rotation = 0;
	
	if (m_prop.size() > 0) {
		auto result = m_prop.find("\"rotation\"");
		if (result != m_prop.end()) {
			rotation = (result->second).head().asNumber();
		}
	}

	return rotation;
}

Expression::ConstIteratorType Expression::tailConstBegin() const noexcept{
  return m_tail.cbegin();
}

Expression::ConstIteratorType Expression::tailConstEnd() const noexcept{
  return m_tail.cend();
}

Expression apply(const Atom & op, const std::vector<Expression> & args, Environment & env){

  // head must be a symbol
  if(!op.isSymbol()){
    throw SemanticError("Error during evaluation: procedure name not symbol");
  }
  
  // must map to a proc
  if(env.is_proc(op)){
	  // map from symbol to proc
	  Procedure proc = env.get_proc(op);

	  // call proc with args
	  return proc(args);
  }
  else if (env.is_lamb(op))  {
	  Expression exp = env.get_lamb(op);

	  std::vector<Expression> parameters;
	  for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		  for (auto m = (*e).tailConstBegin(); m != (*e).tailConstEnd(); ++m) {
			  parameters.push_back(*m);
		  }
		  break;
	  }
	  if (args.size() == parameters.size()) {
		  Expression result;
		  std::map<std::string, Expression> exist;
		  int paramSize = parameters.size();
		  for (int i = 0; i < paramSize; ++i) {
			  if (env.is_exp(parameters[i].head().asSymbol())) {
				  exist[parameters[i].head().asSymbol()] = env.get_exp(parameters[i].head().asSymbol());
				  //rm then add
				  env.rm_exp(parameters[i].head());
				  env.add_exp(parameters[i].head(), args[i]);
			  }
			  else
			  {
				  //add
				  env.add_exp(parameters[i].head(), args[i]);
			  }
		  }

		  // map from symbol to proc
		  SpecialProc spec = env.get_spec(Atom("lambda"));

		  //evaluate
		  int tailPoint = 0;
		  for (auto l = exp.tailConstBegin(); l != exp.tailConstEnd(); ++l) {
			  if (tailPoint != 0) {
				  // call proc with args
				  std::vector<Expression> express;
				  express.push_back(*l);
				  result = spec(express, env);
			  }
			  ++tailPoint;
		  }

		  for (int i = 0; i < paramSize; ++i) {
			  if (env.is_exp(parameters[i].head().asSymbol()))
				  env.rm_exp(parameters[i].head());
		  }

		  for (std::map<std::string, Expression>::iterator it = exist.begin(); it != exist.end(); ++it) {
			  env.add_exp(Atom(it->first), it->second);
		  }
		  return result;
	  }
	  else
	  {
		  throw SemanticError("Error in call to procedure: invalid number of arguments.");
	  }
  }
  // if maps to apply or map
  else if ((op.asSymbol() == "apply") || (op.asSymbol() == "map")) {
	  SpecialProc spec = env.get_spec(op);
	  return spec(args, env);
  }
  else
  {
	  throw SemanticError("Error during evaluation: symbol does not name a procedure");
  }  
}

Expression Expression::handle_lookup(const Atom & head, const Environment & env){
    if(head.isSymbol()){ // if symbol is in env return value
	  std::string s = head.asSymbol();
      if(env.is_exp(head)){
		return env.get_exp(head);
      }
	  else if(s[0] == '"'){
		  return Expression(head);
	  }
      else{
		throw SemanticError("Error during evaluation: unknown symbol");
      }
    }
    else if(head.isNumber()){
      return Expression(head);
    }
    throw SemanticError("Error during evaluation: Invalid type in terminal expression");

}

Expression Expression::handle_begin(Environment & env){
  
  /*
  if(m_tail.size() == 0){
    throw SemanticError("Error during evaluation: zero arguments to begin");
  }
  */
  
  // evaluate each arg from tail, return the last
  Expression result;
  for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
    result = it->eval(env);
  }
  
  return result;
}


Expression Expression::handle_define(Environment & env){

  // tail must have size 3 or error
  if(m_tail.size() != 2){
    throw SemanticError("Error during evaluation: invalid number of arguments to define");
  }
  
  // tail[0] must be symbol
  if(!m_tail[0].isHeadSymbol()){
    throw SemanticError("Error during evaluation: first argument to define not symbol");
  }

  // but tail[0] must not be a special-form or procedure
  std::string s = m_tail[0].head().asSymbol();
  if((s == "define") || (s == "begin") || (s == "lambda")){
    throw SemanticError("Error during evaluation: attempt to redefine a special-form");
  }
  
  if(env.is_proc(s)){
    throw SemanticError("Error during evaluation: attempt to redefine a built-in procedure");
  }
	
  // eval tail[1]
  s = m_tail[1].head().asSymbol();
  // eval differently if lambda function
  if (s == "lambda") {
	  if (m_tail[1].m_tail.size() != 2) {
		  throw SemanticError("Error during evaluation: invalid number of arguments to lambda");
	  }
	  // tail[0] must be symbol
	  if (!m_tail[1].m_tail[0].isHeadSymbol()) {
		  throw SemanticError("Error during evaluation: first argument not symbol");
	  }
	  if (m_tail[1].m_tail[0].m_tail.size() > 0) {
		  int tailSize = m_tail[1].m_tail[0].m_tail.size();
		  for (int i = 0; i < tailSize; i++) {
			  if (!m_tail[1].m_tail[0].m_tail[i].isHeadSymbol()) {
				  throw SemanticError("Error during evaluation: first argument not symbol");
			  }
		  }
	  }
	  std::list<Expression> parameters;
	  // and tail[0] must not be a special-form or procedure
	  std::string s = m_tail[1].m_tail[0].head().asSymbol();
	  // add parameters to a list
	  if ((s == "define") || (s == "begin") || (s == "lambda") || (s == "apply") || (s == "map") || (env.is_proc(s))) {
		  throw SemanticError("Error during evaluation: attempt to set parameter as a special-form or built-in procedure.");
	  }
	  parameters.push_back(Expression(Atom(s)));
	  if (m_tail[1].m_tail[0].m_tail.size() > 0) {
		  int tail0Size = m_tail[1].m_tail[0].m_tail.size();
		  for (int i = 0; i < tail0Size; i++) {
			  std::string s = m_tail[1].m_tail[0].m_tail[i].head().asSymbol();
			  if ((s == "define") || (s == "begin") || (s == "lambda") || (s == "apply") || (s == "map") || (env.is_proc(s))) {
				  throw SemanticError("Error during evaluation: attempt to set parameter as a special-form or built-in procedure.");
			  }
			  parameters.push_back(Expression(Atom(s)));
		  }
	  }
	  std::vector<Expression> result;
	  result.push_back(parameters);
	  result.push_back(m_tail[1].m_tail[1]);

	  env.add_lamb(m_tail[0].head().asSymbol(), result);

	  return Expression(result);
  }
  else if (m_tail[1].head().asSymbol() == "set-property") 
  {
	  Expression result = m_tail[1].eval(env);

	  if (env.is_exp(m_tail[0].head())) {
		  //remove and add new
		  env.rm_exp(m_tail[0].head());

		  env.add_exp(m_tail[0].head(), result);

		  return result;
	  }

	  //and add to env
	  env.add_exp(m_tail[0].head(), result);

	  return result;
  }
  else
  {
	  Expression result = m_tail[1].eval(env);

	  //and add to env
	  env.add_exp(m_tail[0].head(), result);

	  return result;
  }
}

Expression Expression::handle_lambda(Environment & env){

	if (m_tail.size() != 2) {
		throw SemanticError("Error during evaluation: invalid number of arguments to lambda");
	}
	// tail[0] must be symbol
	if (!m_tail[0].isHeadSymbol()) {
		throw SemanticError("Error during evaluation: first argument not symbol");
	}
	if (m_tail[0].m_tail.size() > 0) {
		int tailSize = m_tail[0].m_tail.size();
		for (int i = 0; i < tailSize; i++) {
			if (!m_tail[0].m_tail[i].isHeadSymbol()) {
				throw SemanticError("Error during evaluation: first argument not symbol");
			}
		}
	}
	std::list<Expression> parameters;
	// and tail[0] must not be a special-form or procedure
	std::string s = m_tail[0].head().asSymbol();
	// add parameters to a list
	if ((s == "define") || (s == "begin") || (s == "lambda") || (s == "apply") || (s == "map") || (env.is_proc(s))) {
		throw SemanticError("Error during evaluation: attempt to set parameter as a special-form or built-in procedure.");
	}
	parameters.push_back(Expression(Atom(s)));
	if (m_tail[0].m_tail.size() > 0) {
		int tail0Size = m_tail[0].m_tail.size();
		for (int i = 0; i < tail0Size; i++) {
			std::string s = m_tail[0].m_tail[i].head().asSymbol();
			if ((s == "define") || (s == "begin") || (s == "lambda") || (s == "apply") || (s == "map") || (env.is_proc(s))) {
				throw SemanticError("Error during evaluation: attempt to set parameter as a special-form or built-in procedure.");
			}
			parameters.push_back(Expression(Atom(s)));
		}
	}
	std::vector<Expression> result;
	result.push_back(parameters);
	result.push_back(m_tail[1]);

	return Expression(result);
}

Expression Expression::handle_setprop(const std::vector<Expression>& args)
{
	if (args.size() == 3){
		if (args[0].isHeadSymbol()){
			std::string s = args[0].head().asSymbol();
			Expression exp;
			exp = args[2];
			exp.m_prop[s] = args[1];
			return Expression(exp);
		}
		else
			throw SemanticError("Error in call to set-property: first argument not a string.");
	}
	else 
		throw SemanticError("Error in call to set-property: invalid number of arguments.");
}

Expression Expression::handle_getprop(const std::vector<Expression>& args){
	
	if (args.size() == 2) {
		if (args[0].isHeadSymbol()) {
			std::string s = args[0].head().asSymbol();
			Expression exp;

			auto result = args[1].m_prop.find(args[0].head().asSymbol());
			if (result != args[1].m_prop.end()) {
				exp = result->second;
			}
			return exp;
		}
		else
			throw SemanticError("Error in call to set-property: first argument not a string.");
	}
	else
		throw SemanticError("Error in call to set-property: invalid number of arguments.");
}

Expression Expression::handle_discrete(const std::vector<Expression>& args) {

	std::list<Expression> result;
	if (args.size() == 2) {
		std::string s = args[0].head().asSymbol();
		if (s == "List") {
			std::string s = args[1].head().asSymbol();
			if (s == "List") {
				double N = 20;
				double A = 3;
				double B = 3;
				double C = 2;
				double D = 2;
				double P = 0.5;
				double xLength;
				double yLength;
				double maxX;
				double minX;
				double maxY;
				double minY;
				double scalex;
				double scaley;
				double xmiddle;
				double ymiddle;
				std::string AUvalue;
				std::string ALvalue;
				std::string OUvalue;
				std::string OLvalue;

				std::vector<Expression> xCoords;
				std::vector<Expression> yCoords;
				for (size_t i = 0; i < (args[0].m_tail.size()); ++i) {
					xCoords.push_back(Expression(args[0].m_tail[i].m_tail[0].head().asNumber()));
					yCoords.push_back(Expression(-(args[0].m_tail[i].m_tail[1].head().asNumber())));
				}

				maxX = xCoords.front().head().asNumber();
				minX = xCoords.back().head().asNumber();
				for (size_t g = 0; g < xCoords.size(); ++g) {
					if (maxX < xCoords[g].head().asNumber()) {
						maxX = xCoords[g].head().asNumber();
					}
					if (minX > xCoords[g].head().asNumber()) {
						minX = xCoords[g].head().asNumber();
					}
				}
				maxY = yCoords.back().head().asNumber();
				minY = yCoords.front().head().asNumber();
				for (size_t g = 0; g < yCoords.size(); ++g) {
					if (maxY > yCoords[g].head().asNumber()) {
						maxY = yCoords[g].head().asNumber();
					}
					if (minY < yCoords[g].head().asNumber()) {
						minY = yCoords[g].head().asNumber();
					}
				}
				
				xLength = sqrt(pow((maxX - minX), 2));
				yLength = sqrt(pow((maxY - minY), 2));
				
				std::stringstream AU;
				std::stringstream AL;
				AU << std::setprecision(2) << maxX;
				AL << std::setprecision(2) << minX;
				std::string AUstr = AU.str();
				std::string ALstr = AL.str();
				std::stringstream OU;
				std::stringstream OL;
				OU << std::setprecision(2) << -maxY;
				OL << std::setprecision(2) << -minY;
				std::string OUstr = OU.str();
				std::string OLstr = OL.str();

				AUvalue = "\"" + AUstr + "\"";
				ALvalue = "\"" + ALstr + "\"";
				OUvalue = "\"" + OUstr + "\"";
				OLvalue = "\"" + OLstr + "\"";

				scalex = N / xLength;
				scaley = N / yLength;

				maxX = maxX * scalex;
				minX = minX * scalex;
				maxY = maxY * scaley;
				minY = minY * scaley;

				xmiddle = (maxX + minX) / 2;
				ymiddle = (maxY + minY) / 2;

				int xSize = xCoords.size();
				for (int g = 0; g < xSize; ++g) {
					double newX = xCoords[0].head().asNumber() * scalex;
					xCoords.push_back(Expression(newX));
					xCoords.erase(xCoords.begin());
				}

				int ySize = yCoords.size();
				for (int g = 0; g < ySize; ++g) {
					double newY = yCoords[0].head().asNumber() * scaley;
					yCoords.push_back(Expression(newY));
					yCoords.erase(yCoords.begin());
				}

				//graph lines
				if ((((maxY > 0) && (minY > 0)) || ((maxY < 0) && (minY < 0))) && (((maxX > 0) && (minX > 0)) || ((maxX < 0) && (minX < 0)))) {
					if ((maxY > 0) && (minY > 0)) {
						//points and lines inside
						for (size_t m = 0; m < xCoords.size(); ++m) {
							//make-point						
							Expression point1 = make_point(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), P);
							result.push_back(point1);
							//make-line
							Expression line1 = make_line(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), xCoords[m].head().asNumber(), maxY, 0);
							result.push_back(line1);
						}
					}
					else {
						//points and lines inside
						for (size_t m = 0; m < xCoords.size(); ++m) {
							//make-point						
							Expression point1 = make_point(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), P);
							result.push_back(point1);
							//make-line
							Expression line1 = make_line(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), xCoords[m].head().asNumber(), minY, 0);
							result.push_back(line1);
						}
					}
				}
				else if (((maxY > 0) && (minY > 0)) || ((maxY < 0) && (minY < 0))) {
					if ((maxY > 0) && (minY > 0)) {
						//points and lines inside
						for (size_t m = 0; m < xCoords.size(); ++m) {
							//make-point						
							Expression point1 = make_point(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), P);
							result.push_back(point1);
							//make-line
							Expression line1 = make_line(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), xCoords[m].head().asNumber(), maxY, 0);
							result.push_back(line1);
						}
					}
					else {
						//points and lines inside
						for (size_t m = 0; m < xCoords.size(); ++m) {
							//make-point						
							Expression point1 = make_point(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), P);
							result.push_back(point1);
							//make-line
							Expression line1 = make_line(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), xCoords[m].head().asNumber(), minY, 0);
							result.push_back(line1);
						}
					}
					//Y origin
					Expression lineY1 = make_line(0, minY, 0, maxY, 0);
					result.push_back(lineY1);
				}
				else if (((maxX > 0) && (minX > 0)) || ((maxX < 0) && (minX < 0))) {
					//points and lines inside
					for (size_t m = 0; m < xCoords.size(); ++m) {
						//make-point						
						Expression point1 = make_point(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), P);
						result.push_back(point1);
						//make-line
						Expression line1 = make_line(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), xCoords[m].head().asNumber(), 0, 0);
						result.push_back(line1);
					}
					//X origin
					Expression lineX1 = make_line(minX, 0, maxX, 0, 0);
					result.push_back(lineX1);
				}
				else {
					//points and lines inside
					for (size_t m = 0; m < xCoords.size(); ++m) {
						//make-point						
						Expression point1 = make_point(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), P);
						result.push_back(point1);
						//make-line
						Expression line1 = make_line(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), xCoords[m].head().asNumber(), 0, 0);
						result.push_back(line1);
					}
					//X origin
					Expression lineX1 = make_line(minX, 0, maxX, 0, 0);
					result.push_back(lineX1);
					//Y origin
					Expression lineY1 = make_line(0, minY, 0, maxY, 0);
					result.push_back(lineY1);
				}
				//Outer lines
				Expression border1 = make_line(minX, maxY, maxX, maxY, 0);
				result.push_back(border1);
				Expression border2 = make_line(minX, minY, maxX, minY, 0);
				result.push_back(border2);
				Expression border3 = make_line(minX, maxY, minX, minY, 0);
				result.push_back(border3);
				Expression border4 = make_line(maxX, maxY, maxX, minY, 0);
				result.push_back(border4);

				//Coordinate labels
				double textScale = 1;
				for (size_t i = 0; i < (args[1].m_tail.size()); ++i) {
					if (args[1].m_tail[i].m_tail[0].head().asSymbol() == "\"text-scale\"") {
						textScale = args[1].m_tail[i].m_tail[1].head().asNumber();
					}
				}
				Expression cLabel1 = make_text(maxX, minY + C, AUvalue, textScale, 0);
				result.push_back(cLabel1);
				Expression cLabel2 = make_text(minX, minY + C, ALvalue, textScale, 0);
				result.push_back(cLabel2);
				Expression cLabel3 = make_text(minX - D, maxY, OUvalue, textScale, 0);
				result.push_back(cLabel3);
				Expression cLabel4 = make_text(minX - D, minY, OLvalue, textScale, 0);
				result.push_back(cLabel4);

				for (size_t i = 0; i < (args[1].m_tail.size()); ++i) {
					std::list<Expression> label;
					if (args[1].m_tail[i].m_tail[0].head().asSymbol() == "\"title\"") {
						Expression cLabel = make_text(xmiddle, maxY - A, args[1].m_tail[i].m_tail[1].head().asSymbol(), textScale, 0);
						result.push_back(cLabel);
					}
					else if (args[1].m_tail[i].m_tail[0].head().asSymbol() == "\"abscissa-label\"") {
						Expression cLabel = make_text(xmiddle, minY + A, args[1].m_tail[i].m_tail[1].head().asSymbol(), textScale, 0);
						result.push_back(cLabel);
					}
					else if (args[1].m_tail[i].m_tail[0].head().asSymbol() == "\"ordinate-label\"") {
						Expression cLabel = make_text(minX - B, ymiddle, args[1].m_tail[i].m_tail[1].head().asSymbol(), textScale, (-90 * (atan(1) * 4) / 180));
						result.push_back(cLabel);
					}
				}
			}
			else {
				throw SemanticError("Error in call to discrete-plot: second argument must be a list.");
			}
		}
		else {
			throw SemanticError("Error in call to discrete-plot: first argument must be a list.");
		}
	}
	else {
		throw SemanticError("Error in call to discrete-plot: invalid number of arguments.");
	}

	return result;
}

Expression Expression::handle_continuous(const std::vector<Expression>& args, Environment & env){
	
	std::list<Expression> result;
	if (args.size() >= 2) {
		if (env.is_lamb(args[0].head().asSymbol())) {
			//check if one argument lambda function
			Expression lamb = env.get_lamb(args[0].head());
			std::vector<Expression> parameters;
			for (auto e = lamb.tailConstBegin(); e != lamb.tailConstEnd(); ++e) {
				for (auto m = (*e).tailConstBegin(); m != (*e).tailConstEnd(); ++m) {
					parameters.push_back(*m);
				}
				break;
			}
			int paramSize = parameters.size();
			if (paramSize == 1) {
				std::string s = args[1].head().asSymbol();
				if (s == "List") {
					double N = 20;
					double A = 3;
					double B = 3;
					double C = 2;
					double D = 2;
					double M = 50;
					double MAX = 10;
					double maxX = args[1].m_tail[1].head().asNumber();
					double minX = args[1].m_tail[0].head().asNumber();
					double maxY;
					double minY;
					double xLength = sqrt(pow((maxX - minX), 2));
					double yLength;
					double scalex;
					double scaley;
					double xIntervals = xLength / (M - 1);
					double xmiddle;
					double ymiddle;
					std::string AUvalue;
					std::string ALvalue;
					std::string OUvalue;
					std::string OLvalue;

					std::vector<Expression> xCoords;
					xCoords.push_back(Expression(minX));
					for (int i = 0; i < (M - 2); ++i) {
						xCoords.push_back(Expression(xCoords[i].head().asNumber() + xIntervals));
					}
					xCoords.push_back(Expression(maxX));

					std::list<Expression> X;
					for (size_t k = 0; k < xCoords.size(); ++k) {
						X.push_back(Expression(xCoords[k].head().asNumber()));
					}

					std::vector<Expression> findY;
					findY.push_back(args[0].head());
					findY.push_back(Expression(X));
					SpecialProc spec = env.get_spec(Atom("map"));
					Expression Y = spec(findY, env);
					findY.pop_back();

					std::vector<Expression> yCoords;
					for (auto e = Y.tailConstBegin(); e != Y.tailConstEnd(); ++e) {
						yCoords.push_back(Expression(-((*e).head().asNumber())));
					}

					maxY = yCoords.back().head().asNumber();
					minY = yCoords.front().head().asNumber();
					for (size_t g = 0; g < yCoords.size(); ++g) {
						if (maxY > yCoords[g].head().asNumber()) {
							maxY = yCoords[g].head().asNumber();
						}
						if (minY < yCoords[g].head().asNumber()) {
							minY = yCoords[g].head().asNumber();
						}
					}

					yLength = sqrt(pow((maxY - minY), 2));

					std::stringstream AU;
					std::stringstream AL;
					AU << std::setprecision(2) << maxX;
					AL << std::setprecision(2) << minX;
					std::string AUstr = AU.str();
					std::string ALstr = AL.str();
					std::stringstream OU;
					std::stringstream OL;
					OU << std::setprecision(2) << -maxY;
					OL << std::setprecision(2) << -minY;
					std::string OUstr = OU.str();
					std::string OLstr = OL.str();

					AUvalue = "\"" + AUstr + "\"";
					ALvalue = "\"" + ALstr + "\"";
					OUvalue = "\"" + OUstr + "\"";
					OLvalue = "\"" + OLstr + "\"";

					scalex = N / xLength;
					scaley = N / yLength;

					maxX = maxX * scalex;
					minX = minX * scalex;
					maxY = maxY * scaley;
					minY = minY * scaley;

					xmiddle = (maxX + minX) / 2;
					ymiddle = (maxY + minY) / 2;

					int xSize = xCoords.size();
					for (int g = 0; g < xSize; ++g) {
						double newX = xCoords[0].head().asNumber() * scalex;
						xCoords.push_back(Expression(newX));
						xCoords.erase(xCoords.begin());
					}

					int ySize = yCoords.size();
					for (int g = 0; g < ySize; ++g) {
						double newY = yCoords[0].head().asNumber() * scaley;
						yCoords.push_back(Expression(newY));
						yCoords.erase(yCoords.begin());
					}

					std::list<Expression> newX;
					bool split = true;
					for (int toMax = 0; toMax < MAX; ++toMax) {
						if (split == true) {
							int numPts = xCoords.size() - 2;
							int numSplit = 0;
							for (int ptPos = 0; ptPos < numPts; ++ptPos) {
								//find slopes
								double m1 = ((yCoords[ptPos+1].head().asNumber()) - (yCoords[ptPos].head().asNumber())) / ((xCoords[ptPos + 1].head().asNumber()) - (xCoords[ptPos].head().asNumber()));
								double m2 = ((yCoords[ptPos+2].head().asNumber()) - (yCoords[ptPos+1].head().asNumber())) / ((xCoords[ptPos+2].head().asNumber()) - (xCoords[ptPos+1].head().asNumber()));
								//find angle
								double angle = (atan((m2 - m1) / (1 + m2 * m1))) * 180 / (atan(1) * 4) + 180;
								if (!((angle > 175) && (angle < 185))) {
									//split
									//find midpoint
									double midpt1x = ((xCoords[ptPos + 1].head().asNumber()) + (xCoords[ptPos].head().asNumber())) / 2;
									newX.push_back(Expression(midpt1x / scalex));
									findY.push_back(Expression(newX));
									Expression newY1 = spec(findY, env);
									findY.pop_back();
									newX.clear();
									double midpt1y = newY1.m_tail[0].head().asNumber() * -scaley;
									double midpt2x = ((xCoords[ptPos + 2].head().asNumber()) + (xCoords[ptPos + 1].head().asNumber())) / 2;
									newX.push_back(Expression(midpt2x / scalex));
									findY.push_back(Expression(newX));
									Expression newY2 = spec(findY, env);
									findY.pop_back();
									newX.clear();
									double midpt2y = newY2.m_tail[0].head().asNumber() * -scaley;
									
									//add points
									xCoords.insert(xCoords.begin()+ptPos+1, Expression(midpt1x));
									xCoords.insert(xCoords.begin()+ptPos+3, Expression(midpt2x));
									yCoords.insert(yCoords.begin()+ptPos+1, Expression(midpt1y));
									yCoords.insert(yCoords.begin()+ptPos+3, Expression(midpt2y));
									
									ptPos = ptPos + 4;
									numPts = xCoords.size() - 2;
									++numSplit;
								}
								//if at the end of line list
								if (ptPos == (numPts - 1)) {
									if (numSplit > 0) {
										split = true;
									}
									else
										split = false;
								}
							}
						}
						else {
							toMax = 10;
						}
					}

					//points and lines inside
					for (size_t m = 0; m < (xCoords.size() - 1); ++m) {
						//make-line
						Expression line1 = make_line(xCoords[m].head().asNumber(), yCoords[m].head().asNumber(), xCoords[m+1].head().asNumber(), yCoords[m+1].head().asNumber(), 0);
						result.push_back(line1);
					}
					
					//graph lines
					if ((((maxY > 0) && (minY > 0)) || ((maxY < 0) && (minY < 0))) && (((maxX > 0) && (minX > 0)) || ((maxX < 0) && (minX < 0)))) {}
					else if (((maxY > 0) && (minY > 0)) || ((maxY < 0) && (minY < 0))) {
						//Y origin
						Expression lineY1 = make_line(0, minY, 0, maxY, 0);
						result.push_back(lineY1);
					}
					else if (((maxX > 0) && (minX > 0)) || ((maxX < 0) && (minX < 0))) {
						//X origin
						Expression lineX1 = make_line(minX, 0, maxX, 0, 0);
						result.push_back(lineX1);
					}
					else {
						//X origin
						Expression lineX1 = make_line(minX, 0, maxX, 0, 0);
						result.push_back(lineX1);
						//Y origin
						Expression lineY1 = make_line(0, minY, 0, maxY, 0);
						result.push_back(lineY1);
					}

					//Outer lines
					Expression border1 = make_line(minX, maxY, maxX, maxY, 0);
					result.push_back(border1);
					Expression border2 = make_line(minX, minY, maxX, minY, 0);
					result.push_back(border2);
					Expression border3 = make_line(minX, maxY, minX, minY, 0);
					result.push_back(border3);
					Expression border4 = make_line(maxX, maxY, maxX, minY, 0);
					result.push_back(border4);

					//if options are there, set the labels
					if (args.size() == 3) {
						std::string s = args[2].head().asSymbol();
						if (s == "List") {
							//Coordinate labels
							double textScale = 1;
							for (size_t i = 0; i < (args[2].m_tail.size()); ++i) {
								if (args[2].m_tail[i].m_tail[0].head().asSymbol() == "\"text-scale\"") {
									textScale = args[2].m_tail[i].m_tail[1].head().asNumber();
								}
							}
							Expression cLabel1 = make_text(maxX, minY + C, AUvalue, textScale, 0);
							result.push_back(cLabel1);
							Expression cLabel2 = make_text(minX, minY + C, ALvalue, textScale, 0);
							result.push_back(cLabel2);
							Expression cLabel3 = make_text(minX - D, maxY, OUvalue, textScale, 0);
							result.push_back(cLabel3);
							Expression cLabel4 = make_text(minX - D, minY, OLvalue, textScale, 0);
							result.push_back(cLabel4);

							for (size_t i = 0; i < (args[2].m_tail.size()); ++i) {
								std::list<Expression> label;
								if (args[2].m_tail[i].m_tail[0].head().asSymbol() == "\"title\"") {
									Expression cLabel = make_text(xmiddle, maxY - A, args[2].m_tail[i].m_tail[1].head().asSymbol(), textScale, 0);
									result.push_back(cLabel);
								}
								else if (args[2].m_tail[i].m_tail[0].head().asSymbol() == "\"abscissa-label\"") {
									Expression cLabel = make_text(xmiddle, minY + A, args[2].m_tail[i].m_tail[1].head().asSymbol(), textScale, 0);
									result.push_back(cLabel);
								}
								else if (args[2].m_tail[i].m_tail[0].head().asSymbol() == "\"ordinate-label\"") {
									Expression cLabel = make_text(minX - B, ymiddle, args[2].m_tail[i].m_tail[1].head().asSymbol(), textScale, (-90 * (atan(1) * 4) / 180));
									result.push_back(cLabel);
								}
							}
						}
						else {
							throw SemanticError("Error in call to continuous-plot: third argument must be a list.");
						}
					}
					else {
						//Coordinate labels
						double textScale = 1;
						Expression cLabel1 = make_text(maxX, minY + C, AUvalue, textScale, 0);
						result.push_back(cLabel1);
						Expression cLabel2 = make_text(minX, minY + C, ALvalue, textScale, 0);
						result.push_back(cLabel2);
						Expression cLabel3 = make_text(minX - D, maxY, OUvalue, textScale, 0);
						result.push_back(cLabel3);
						Expression cLabel4 = make_text(minX - D, minY, OLvalue, textScale, 0);
						result.push_back(cLabel4);
					}
				}
				else {
					throw SemanticError("Error in call to continuous-plot: second argument must be a list.");
				}
			}
			else {
				throw SemanticError("Error in call to continuous-plot: lambda function must be of a single variable.");
			}							
		}
		else {
			throw SemanticError("Error in call to continuous-plot: first argument must be a lambda function.");
		}
	}
	else {
		throw SemanticError("Error in call to continuous-plot: invalid number of arguments.");
	}

	return result;
}

Expression Expression::make_point(double x, double y, double size){

	std::list<Expression> result;
	result.push_back(Atom(x));
	result.push_back(Atom(y));
	Expression point = Expression(result);
	point.m_prop["\"object-name\""] = Expression(Atom("\"point\""));
	point.m_prop["\"size\""] = Expression(Atom(size));
	return point;
}

Expression Expression::make_line(double x1, double y1, double x2, double y2, double thickness){

	std::list<Expression> result;
	Expression point1 = make_point(x1, y1, 0);
	Expression point2 = make_point(x2, y2, 0);
	result.push_back(point1);
	result.push_back(point2);
	Expression line = Expression(result);
	line.m_prop["\"object-name\""] = Expression(Atom("\"line\""));
	line.m_prop["\"thickness\""] = Expression(Atom(thickness));
	return line;
}

Expression Expression::make_text(double x, double y, std::string text, double scale, double rotation){

	Expression string = Expression(Atom(text));
	Expression point = make_point(x, y, 0);
	string.m_prop["\"object-name\""] = Expression(Atom("\"text\""));
	string.m_prop["\"position\""] = point;
	string.m_prop["\"scale\""] = Expression(Atom(scale));
	string.m_prop["\"rotation\""] = Expression(Atom(rotation));
	return string;
}

// this is a simple recursive version. the iterative version is more
// difficult with the ast data structure used (no parent pointer).
// this limits the practical depth of our AST
Expression Expression::eval(Environment & env){
  
  if(m_tail.empty() && m_head.asSymbol() != "list"){
    return handle_lookup(m_head, env);
  }
  // handle begin special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "begin"){
    return handle_begin(env);
  }
  // handle define special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "define"){
    return handle_define(env);
  }
  // handle lambda special-form
  else if (m_head.isSymbol() && m_head.asSymbol() == "lambda") {
	  return handle_lambda(env);
  }
  // handle apply or map procedure
  else if ((m_head.isSymbol() && (m_head.asSymbol() == "apply")) || (m_head.isSymbol() && (m_head.asSymbol() == "map"))) {
	  std::string s = m_tail[0].m_head.asSymbol();
	  if ((m_tail[0].m_tail.size() > 0) && (s != "lambda")) {
		  throw SemanticError("Error: first argument must be a procedure.");
	  }
	  else {
		  std::vector<Expression> results;
		  for (Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it) {
			  results.push_back(*it);
		  }
		  return apply(m_head, results, env);
	  }
  }
  else if ((m_head.asSymbol() == "continuous-plot")) {
	  std::vector<Expression> results;
	  int l = 0;
	  for (Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it) {
		  if (l != 0) {
			  results.push_back(it->eval(env));
		  }
		  else 
			  results.push_back(*it);
		  ++l;
	  }
	  return handle_continuous(results, env);
  }
  // else attempt to treat as procedure
  else{ 
    std::vector<Expression> results;
    for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
      results.push_back(it->eval(env));
    }
	if ((m_head.asSymbol() == "set-property") || (m_head.asSymbol() == "get-property") || (m_head.asSymbol() == "discrete-plot")) {
		if (m_head.asSymbol() == "set-property") {
			return handle_setprop(results);
		}
		else if (m_head.asSymbol() == "get-property")
		{
			return handle_getprop(results);
		}
		else if (m_head.asSymbol() == "discrete-plot")
		{
			return handle_discrete(results);
		}
	}
	return apply(m_head, results, env);
  }
}

Expression Expression::eval_lambda(const std::vector<Expression> & args, Environment & env){

	Environment temp = env;
	Expression result;
	Expression tempExp;
	tempExp.m_head = Atom("lamda");
	tempExp.m_tail.push_back(args[0]);
	for (Expression::IteratorType it = tempExp.m_tail.begin(); it != tempExp.m_tail.end(); ++it) {
		result = (it->eval(env));
	}
	env = temp;
	return result;
}

std::vector<Expression> Expression::eval_app_map(Environment & env, Expression arguments)
{
	std::vector<Expression> results;
	for (Expression::IteratorType it = arguments.m_tail.begin(); it != arguments.m_tail.end(); ++it) {
		results.push_back(it->eval(env));
	}
	return results;
}

std::ostream & operator<<(std::ostream & out, const Expression & exp){

  if(exp.head().isComplex()){
	out << exp.head(); 
  }
  else if (exp.head().asSymbol().find("Error") != std::string::npos) {
	out << exp.head();
  }
  else if (exp.head().asSymbol() == "List") {
	  out << "(";
	  int tailSize = 0;
	  for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		  ++tailSize;
	  }
	  int printSize = 0;
	  for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		  out << *e;
		  ++printSize;
		  if (printSize != tailSize) {
			  out << " ";
		  }
	  }
	  out << ")";
  }
  else if (exp.head().asSymbol() == "lambda") {
	  out << "(";
	  for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		  out << *e;
		  break;
	  }
	  out << " ";
	  int tailSize = 0;
	  for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		  if (tailSize != 0) {
			  out << *e;
		  }
		  ++tailSize;
	  }
	  out << ")";
  }
  else if (exp.head().isNone()) {
	  out << "NONE";
  }
  else {
    out << "(";
    out << exp.head();
	int tSize = 0;
	for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		++tSize;
	}
	if (tSize > 0)
		out << " ";
	int  pSize = 0;
    for(auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e){
		out << *e;
		++pSize;
		if(pSize != tSize)
			out << " ";
    }
    out << ")";
  }

  return out;
}

bool Expression::operator==(const Expression & exp) const noexcept{

  bool result = (m_head == exp.m_head);

  result = result && (m_tail.size() == exp.m_tail.size());

  if(result){
    for(auto lefte = m_tail.begin(), righte = exp.m_tail.begin();
	(lefte != m_tail.end()) && (righte != exp.m_tail.end());
	++lefte, ++righte){
      result = result && (*lefte == *righte);
    }
  }

  return result;
}

bool operator!=(const Expression & left, const Expression & right) noexcept{

  return !(left == right);
}
