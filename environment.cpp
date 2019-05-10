#include "environment.hpp"

#include <cassert>
#include <cmath>
#include <complex>
#include <list>
#include <map>			  

#include "environment.hpp"
#include "semantic_error.hpp"

/*********************************************************************** 
Helper Functions
**********************************************************************/

// predicate, the number of args is nargs
bool nargs_equal(const std::vector<Expression> & args, unsigned nargs){
  return args.size() == nargs;
}

/*********************************************************************** 
Each of the functions below have the signature that corresponds to the
typedef'd Procedure function pointer.
**********************************************************************/

// the default procedure always returns an expresison of type None
Expression default_proc(const std::vector<Expression> & args){
  args.size(); // make compiler happy we used this parameter
  return Expression();
};

Expression add(const std::vector<Expression> & args){
  
  // check all aruments are numbers, while adding
  int isComplex = 0;
  std::complex<double> result(0,0);
  for( auto & a :args){
	if(a.isHeadNumber()){
	  result += a.head().asNumber();
	}
	else if(a.isHeadComplex()){
	  result += a.head().asComplex();
	  ++isComplex;
	}
	else{
		throw SemanticError("Error in call to add, invalid argument.");
	}
  }
  
  if (isComplex > 0){
	return Expression(result);
  }
  else {
	double realresult = real(result);
	return Expression(realresult);
  }
};

Expression mul(const std::vector<Expression> & args){
 
  // check all aruments are numbers, while multiplying
  int isComplex = 0;
  std::complex<double> result(1,0);
  for( auto & a :args){
	if(a.isHeadNumber()){
	  result *= a.head().asNumber();
	}
	else if(a.isHeadComplex()){
	  result *= a.head().asComplex();
	  ++isComplex;
	}
	else{
		throw SemanticError("Error in call to mul, invalid argument.");
	}
  }
  
  if (isComplex > 0){
	return Expression(result);
  }
  else {
	double realresult = real(result);
	return Expression(realresult);
  }
};

Expression subneg(const std::vector<Expression> & args){

  int isComplex = 0;
  std::complex<double> result(0,0);

  // preconditions
  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
      result = -args[0].head().asNumber();
    }
	else if (args[0].isHeadComplex()){
	  ++isComplex;
	  result = -(args[0].head().asComplex());
	}
    else{
      throw SemanticError("Error in call to negate: invalid argument.");
    }
  }
  else if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = args[0].head().asNumber() - args[1].head().asNumber();
    }
	else if ((args[0].isHeadComplex()) && (args[1].isHeadNumber())){
		++isComplex;
		result = args[0].head().asComplex() - args[1].head().asNumber();
	}
	else if ((args[0].isHeadNumber()) && (args[1].isHeadComplex())){
		++isComplex;
		result = args[0].head().asNumber() - args[1].head().asComplex();
	}
	else if ((args[0].isHeadComplex()) && (args[1].isHeadComplex())){
		++isComplex;
		result = args[0].head().asComplex() - args[1].head().asComplex();
	}
    else{      
      throw SemanticError("Error in call to subtraction: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to subtraction or negation: invalid number of arguments.");
  }

  if (isComplex > 0){
	return Expression(result);
  }
  else {
	double realresult = real(result);
	return Expression(realresult);
  }
};

Expression div(const std::vector<Expression> & args){

  int isComplex = 0;
  std::complex<double> result(0,0);

  if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = args[0].head().asNumber() / args[1].head().asNumber();
    }
	else if ((args[0].isHeadComplex()) && (args[1].isHeadNumber())){
		++isComplex;
		result = args[0].head().asComplex() / args[1].head().asNumber();
	}
	else if ((args[0].isHeadNumber()) && (args[1].isHeadComplex())){
		++isComplex;
		result = args[0].head().asNumber() / args[1].head().asComplex();
	}
	else if ((args[0].isHeadComplex()) && (args[1].isHeadComplex())){
		++isComplex;
		result = args[0].head().asComplex() / args[1].head().asComplex();
	}
    else{      
      throw SemanticError("Error in call to division: invalid argument.");
    }
  }
  else if(nargs_equal(args,1)) {
	  if(args[0].isHeadNumber()){
		  result = 1 / (args[0].head().asNumber());
	  }
	  else if(args[0].isHeadComplex()){
		  ++isComplex;
		  result = pow (args[0].head().asComplex(), -1);
	  }
	  else 
		  throw SemanticError("Error in call to division: invalid argument.");
  }
  else{
    throw SemanticError("Error in call to division: invalid number of arguments.");
  }
  
  if (isComplex > 0){
	return Expression(result);
  }
  else {
	double realresult = real(result);
	return Expression(realresult);
  }
};

Expression sqrt(const std::vector<Expression> & args){

  int isComplex = 0;
  std::complex<double> result(0,0);

  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
		if(args[0].head().asNumber() >= 0) {
			result = sqrt (args[0].head().asNumber());
		}
		else if(args[0].head().asNumber() == -1) {
			++isComplex;
			result = {0,1};
		}
		else {
		    ++isComplex;
			double value = -(args[0].head().asNumber());
			double sqrted = sqrt(value);
			result = {sqrted,1};
		}
    }
	else if(args[0].isHeadComplex()) {
	  result = sqrt (args[0].head().asComplex());
	  ++isComplex;
	}
    else{
      throw SemanticError("Error in call to square root: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to square root: invalid number of arguments.");
  }

  if (isComplex > 0){
	return Expression(result);
  }
  else {
	double realresult = real(result);
	return Expression(realresult);
  }
};

Expression expo(const std::vector<Expression> & args){

  int isComplex = 0;
  std::complex<double> result(0,0);

  if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
		result = pow (args[0].head().asNumber(), args[1].head().asNumber());
    }
	else if ((args[0].isHeadComplex()) && (args[1].isHeadNumber())){
		++isComplex;
		result = pow (args[0].head().asComplex(), args[1].head().asNumber());
	}
	else if ((args[0].isHeadNumber()) && (args[1].isHeadComplex())){
		++isComplex;
		result = pow (args[0].head().asNumber(), args[1].head().asComplex());
	}
	else if ((args[0].isHeadComplex()) && (args[1].isHeadComplex())){
		++isComplex;
		result = pow (args[0].head().asComplex(), args[1].head().asComplex());
	}
    else{      
      throw SemanticError("Error in call to exponential: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to exponential: invalid number of arguments.");
  }
  
  if (isComplex > 0){
	return Expression(result);
  }
  else {
	double realresult = real(result);
	return Expression(realresult);
  }
};

Expression ln(const std::vector<Expression> & args){

  double result = 0;

  // preconditions
  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
		if(args[0].head().asNumber() > 0) {
			result = log (args[0].head().asNumber());
		}
		else {
			throw SemanticError("Error in call to natural logarithm: argument must be positive.");
		}
    }
    else{
      throw SemanticError("Error in call to natural logarithm: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to natural logarithm: invalid number of arguments.");
  }

  return Expression(result);
};

Expression sin(const std::vector<Expression> & args){

  double result = 0;  

  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
		result = sin (args[0].head().asNumber());
    }
    else{      
      throw SemanticError("Error in call to trigonometric sine: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to trigonometric sine: invalid number of arguments.");
  }
  
  return Expression(result);
};

Expression cos(const std::vector<Expression> & args){

  double result = 0;  

  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
		result = cos (args[0].head().asNumber());
    }
    else{      
      throw SemanticError("Error in call to trigonometric cosine: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to trigonometric cosine: invalid number of arguments.");
  }
  
  return Expression(result);
};

Expression tan(const std::vector<Expression> & args){

  double result = 0;  

  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
		result = tan (args[0].head().asNumber());
    }
    else{      
      throw SemanticError("Error in call to trigonometric tangent : invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to trigonometric tangent : invalid number of arguments.");
  }
  
  return Expression(result);
};

Expression real(const std::vector<Expression> & args){
  
  std::complex<double> result(0,0);
  if(nargs_equal(args,1)){
    if(args[0].isHeadComplex()){
		double realresult = real(args[0].head().asComplex());
	    return Expression(realresult);
    }
    else{      
      throw SemanticError("Error in call to real: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to real: invalid number of arguments.");
  }
};

Expression imag(const std::vector<Expression> & args){
  
  std::complex<double> result(0,0);
  if(nargs_equal(args,1)){
    if(args[0].isHeadComplex()){
		double realresult = imag(args[0].head().asComplex());
	    return Expression(realresult);
    }
    else{      
      throw SemanticError("Error in call to imag: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to imag: invalid number of arguments.");
  }
};

Expression mag(const std::vector<Expression> & args){
  
  std::complex<double> result(0,0);
  if(nargs_equal(args,1)){
    if(args[0].isHeadComplex()){
		double realresult = abs(args[0].head().asComplex());
	    return Expression(realresult);
    }
    else{      
      throw SemanticError("Error in call to mag: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to mag: invalid number of arguments.");
  }
};

Expression arg(const std::vector<Expression> & args){
  
  std::complex<double> result(0,0);
  if(nargs_equal(args,1)){
    if(args[0].isHeadComplex()){
		double realresult = arg(args[0].head().asComplex());
	    return Expression(realresult);
    }
    else{      
      throw SemanticError("Error in call to arg: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to arg: invalid number of arguments.");
  }
};

Expression conj(const std::vector<Expression> & args){
  
  std::complex<double> result(0,0);
  if(nargs_equal(args,1)){
    if(args[0].isHeadComplex()){
		result = conj(args[0].head().asComplex());
	    return Expression(result);
    }
    else{      
      throw SemanticError("Error in call to conj: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to conj: invalid number of arguments.");
  }
};

Expression list(const std::vector<Expression> & args) {

	std::list<Expression> result;
	for (auto & a : args) {
		result.push_back(a);
	}

	return result;
};

Expression first(const std::vector<Expression> & args) {

	std::list<Expression> result;
	if (nargs_equal(args, 1)) {
		if (args.front().head().asSymbol() == "List") {
			for (std::vector<Expression>::const_iterator it = args[0].tailConstBegin(); it != args[0].tailConstEnd(); ++it) {
				result.push_back(*it);
			}
			if (result.size() != 0) {
				return Expression(result.front());
			}
			else {
				throw SemanticError("Error in call to first: list is empty.");
			}
		}
		else {
			throw SemanticError("Error in call to first: argument must be a list.");
		}
	}
	else {
		throw SemanticError("Error in call to first: invalid number of arguments.");
	}
};

Expression rest(const std::vector<Expression> & args) {
	
	std::list<Expression> result;
	if (nargs_equal(args, 1)) {
		if (args.front().head().asSymbol() == "List") {
			for (std::vector<Expression>::const_iterator it = args[0].tailConstBegin(); it != args[0].tailConstEnd(); ++it) {
				result.push_back(*it);
			}
			if (result.size() != 0) {
				result.pop_front();
				return Expression(result);
			}
			else {
				throw SemanticError("Error in call to rest: list is empty.");
			}
		}
		else {
			throw SemanticError("Error in call to rest: argument must be a list.");
		}
	}
	else {
		throw SemanticError("Error in call to rest: invalid number of arguments.");
	}
};

Expression length(const std::vector<Expression> & args) {
	
	std::list<Expression> result;
	if (nargs_equal(args, 1)) {
		if (args.front().head().asSymbol() == "List") {
			for (std::vector<Expression>::const_iterator it = args[0].tailConstBegin(); it != args[0].tailConstEnd(); ++it) {
				result.push_back(*it);
			}
			return Expression(result.size());
		}
		else {
			throw SemanticError("Error in call to length: argument must be a list.");
		}
	}
	else {
		throw SemanticError("Error in call to length: invalid number of arguments.");
	}
};

Expression append(const std::vector<Expression> & args) {
	
	std::list<Expression> result;
	if (nargs_equal(args, 2)) {
		if (args[0].head().asSymbol() == "List") {
			for (std::vector<Expression>::const_iterator it = args[0].tailConstBegin(); it != args[0].tailConstEnd(); ++it) {
				result.push_back(*it);
			}
			if (args[1].head().asSymbol() == "List") {
				std::list<Expression> inner;
				for (std::vector<Expression>::const_iterator it = args[1].tailConstBegin(); it != args[1].tailConstEnd(); ++it) {
					inner.push_back(*it);
				}
				result.push_back(inner);
			}
			else if (!args[1].head().isSymbol())
			{
				result.push_back(Expression(args[1].head()));
			}
			return result;
		}
		else {
			throw SemanticError("Error in call to append: first argument must be a list.");
		}
	}
	else {
		throw SemanticError("Error in call to append: invalid number of arguments.");
	}
};

Expression join(const std::vector<Expression> & args) {
	
	std::list<Expression> result;
	if (nargs_equal(args, 2)) {
		if (args[0].head().asSymbol() == "List") {
			for (std::vector<Expression>::const_iterator it = args[0].tailConstBegin(); it != args[0].tailConstEnd(); ++it) {
				result.push_back(*it);
			}
			if (args[1].head().asSymbol() == "List") {
				for (std::vector<Expression>::const_iterator it = args[1].tailConstBegin(); it != args[1].tailConstEnd(); ++it) {
					result.push_back(*it);
				}
			}
			else 
			{
				throw SemanticError("Error in call to join: second argument must be a list.");
			}
			return Expression(result);
		}
		else {
			throw SemanticError("Error in call to join: first argument must be a list.");
		}
	}
	else {
		throw SemanticError("Error in call to join: invalid number of arguments.");
	}
};

Expression range(const std::vector<Expression> & args) {

	std::list<Expression> result;
	if (nargs_equal(args, 3)) {
		if (args[0].isHeadNumber() && args[1].isHeadNumber() && args[2].isHeadNumber()) {
			if (args[0].head().asNumber() < args[1].head().asNumber()) {
				if (args[2].head().asNumber() > 0) {
					double startValue = args[0].head().asNumber();
					double endValue = args[1].head().asNumber();
					double incrementValue = args[2].head().asNumber();
					for (double i = startValue; i <= endValue; i += incrementValue) {
						result.push_back(Expression(Atom(i)));
					}
				}
				else {
					throw SemanticError("Error in call to range: Increment must be positive.");
				}
			}
			else {
				throw SemanticError("Error in call to range: first argument must be < second argument.");
			}
		}
		else {
			throw SemanticError("Error in call to range: invalid argument.");
		}
	}
	else {
		throw SemanticError("Error in call to range: invalid number of arguments.");
	}

	return Expression(result);
};

Expression lambda(const std::vector<Expression> & args, Environment & env) {

	Expression exp;

	return exp.eval_lambda(args, env);
};

Expression apply(const std::vector<Expression> & args, Environment & env) {

	if (nargs_equal(args, 2)) {
		std::string m = args[0].head().asSymbol();
		if (env.is_proc(args[0].head())) {
			std::string s = args[1].head().asSymbol();
			if (s == "list") {
				//send to evaluate list and get back the list
				std::vector<Expression> listResults;
				Expression express;
				listResults = express.eval_app_map(env, args[1]);

				// map from symbol to proc
				Procedure proc = env.get_proc(args[0].head());

				// call proc with args
				return proc(listResults);
			}
			else {
				throw SemanticError("Error in call to apply: second argument must be a list.");
			}
		}
		else if (env.is_lamb(args[0].head())) {
			std::string s = args[1].head().asSymbol();
			if (s == "list") {
				//send to evaluate list and get back the list
				std::vector<Expression> listResults;
				Expression express;
				listResults = express.eval_app_map(env, args[1]);
				
				Expression exp = env.get_lamb(args[0].head());
				
			    std::vector<Expression> parameters;
			    for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
				  for (auto m = (*e).tailConstBegin(); m != (*e).tailConstEnd(); ++m) {
					  parameters.push_back(*m);
				  }
				  break;
			    }
				if (listResults.size() == parameters.size()) {
				  std::map<std::string, Expression> exist;
				  int paramSize = parameters.size();
				  for (int i = 0; i < paramSize; ++i) {
					  if (env.is_exp(parameters[i].head().asSymbol())) {
						  exist[parameters[i].head().asSymbol()] = env.get_exp(parameters[i].head().asSymbol());
						  //rm then add
						  env.rm_exp(parameters[i].head());
						  env.add_exp(parameters[i].head(), listResults[i]);
					  }
					  else
					  {
						  //add
						  env.add_exp(parameters[i].head(), listResults[i]);
					  }
				  }
				  // map from symbol to proc
				  SpecialProc spec = env.get_spec(Atom("lambda"));

				  //evaluate
				  int tailPoint = 0;
				  Expression result;
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
			else {
				throw SemanticError("Error in call to apply: second argument must be a list.");
			}
		}
		else {
			throw SemanticError("Error in call to apply: first argument must be a procedure.");
		}
	}
	else {
		throw SemanticError("Error in call to apply: invalid number of arguments.");
	}
}

Expression map(const std::vector<Expression> & args, Environment & env) {

	std::list<Expression> result;
	if (nargs_equal(args, 2)) {
		std::string m = args[0].head().asSymbol();
		if (env.is_proc(args[0].head())) {
			std::string s = args[1].head().asSymbol();
			if (s == "list") {
				//send to evaluate list and get back the list
				std::vector<Expression> listResults;
				Expression express;
				listResults = express.eval_app_map(env, args[1]);
				
				// map from symbol to proc
				Procedure proc = env.get_proc(args[0].head());
				
				std::vector<Expression> arguments;
				
				//send each value in list to procedure and send result to result list
				int listSize = listResults.size();
				for (int i = 0; i < listSize; i++) {
					arguments.push_back(listResults[i]);
					result.push_back(proc(arguments));
					arguments.clear();
				}			
			}
			else {
				throw SemanticError("Error in call to map: second argument must be a list.");
			}
		}
		else if (env.is_lamb(args[0].head())) {
			std::string s = args[1].head().asSymbol();
			if ((s == "list") || (s == "List")) {
				//send to evaluate list and get back the list
				std::vector<Expression> listResults;
				Expression express;
				listResults = express.eval_app_map(env, args[1]);
				
				Expression exp = env.get_lamb(args[0].head());
				std::vector<Expression> arguments;

				std::vector<Expression> parameters;
				for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
					for (auto m = (*e).tailConstBegin(); m != (*e).tailConstEnd(); ++m) {
						parameters.push_back(*m);
					}
					break;
				}
				int paramSize = parameters.size();
				
				//send each value in list to procedure and send result to result list
				int listSize = listResults.size();
				for (int i = 0; i < listSize; i++) {
					arguments.push_back(listResults[i]);
					for(int k = 0; k < paramSize; k++){
						if (arguments.size() == parameters.size()) {
							std::map<std::string, Expression> exist;
							if (env.is_exp(parameters[k].head().asSymbol())) {
								exist[parameters[k].head().asSymbol()] = env.get_exp(parameters[k].head().asSymbol());
								//rm then add
								env.rm_exp(parameters[k].head());
								env.add_exp(parameters[k].head(), arguments[0]);
							}
							else
							{
								//add
								env.add_exp(parameters[k].head(), arguments[0]);
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
									result.push_back(spec(express, env));
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
						}
						else {
							throw SemanticError("Error in call to procedure: invalid number of arguments.");
						}
					}
					arguments.pop_back();
					
				}
				return result;
			}
			else if (s == "range") {
				std::vector<Expression> values;
				for (auto e = args[1].tailConstBegin(); e != args[1].tailConstEnd(); ++e) {
					values.push_back(*e);
				}
				Procedure proc = env.get_proc(Atom("range"));
				Expression listResults;
				listResults = proc(values);

				Expression exp = env.get_lamb(args[0].head());
				std::vector<Expression> arguments;

				std::vector<Expression> parameters;
				for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
					for (auto m = (*e).tailConstBegin(); m != (*e).tailConstEnd(); ++m) {
						parameters.push_back(*m);
					}
					break;
				}
				int paramSize = parameters.size();

				//send each value in list to procedure and send result to result list
				for (auto e = listResults.tailConstBegin(); e != listResults.tailConstEnd(); ++e) {
					arguments.push_back(*e);
					for (int k = 0; k < paramSize; k++) {
						if (arguments.size() == parameters.size()) {
							std::map<std::string, Expression> exist;
							if (env.is_exp(parameters[k].head().asSymbol())) {
								exist[parameters[k].head().asSymbol()] = env.get_exp(parameters[k].head().asSymbol());
								//rm then add
								env.rm_exp(parameters[k].head());
								env.add_exp(parameters[k].head(), arguments[0]);
							}
							else
							{
								//add
								env.add_exp(parameters[k].head(), arguments[0]);
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
									result.push_back(spec(express, env));
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
						}
						else {
							throw SemanticError("Error in call to procedure: invalid number of arguments.");
						}
					}
					arguments.pop_back();
				}
				return result;
			}
			else {
				throw SemanticError("Error in call to map: second argument must be a list.");
			}
		}
		else {
			throw SemanticError("Error in call to map: first argument must be a procedure.");
		}
	}
	else {
		throw SemanticError("Error in call to map: invalid number of arguments.");
	}

	return Expression(result);
}

const double PI = std::atan2(0, -1);
const double negPI = -(std::atan2(0, -1));
const double EXP = std::exp(1);
const std::complex<double> I(0,1);
const std::complex<double> negI(0,-1);

Environment::Environment(){

  reset();
}

bool Environment::is_known(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  return envmap.find(sym.asSymbol()) != envmap.end();
}

bool Environment::is_exp(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  auto result = envmap.find(sym.asSymbol());
  return (result != envmap.end()) && (result->second.type == ExpressionType);
}

Expression Environment::get_exp(const Atom & sym) const{

  Expression exp;
  
  if(sym.isSymbol()){
    auto result = envmap.find(sym.asSymbol());
    if((result != envmap.end()) && (result->second.type == ExpressionType)){
      exp = result->second.exp;
    }
  }

  return exp;
}

void Environment::add_exp(const Atom & sym, const Expression & exp){

  if(!sym.isSymbol()){
    throw SemanticError("Attempt to add non-symbol to environment");
  }
    
  // error if overwriting symbol map
  if(envmap.find(sym.asSymbol()) != envmap.end()){
    throw SemanticError("Attempt to overwrite symbol in environemnt");
  }

  envmap.emplace(sym.asSymbol(), EnvResult(ExpressionType, exp)); 
}

void Environment::rm_exp(const Atom & sym) {

	auto result = envmap.find(sym.asSymbol());
	envmap.erase(result);
}

bool Environment::is_proc(const Atom & sym) const{
	
  if(!sym.isSymbol()) return false;
  
  auto result = envmap.find(sym.asSymbol());
  return (result != envmap.end()) && (result->second.type == ProcedureType);
}

Procedure Environment::get_proc(const Atom & sym) const{

  if(sym.isSymbol()){
    auto result = envmap.find(sym.asSymbol());
    if((result != envmap.end()) && (result->second.type == ProcedureType)){
      return result->second.proc;
    }
  }

  return default_proc;
}

SpecialProc Environment::get_spec(const Atom & sym) const {

	auto result = envmap.find(sym.asSymbol());
	return result->second.spec;
}

bool Environment::is_lamb(const Atom & sym) const
{
	if (!sym.isSymbol()) return false;

	auto result = envmap.find(sym.asSymbol());
	return (result != envmap.end()) && (result->second.type == LambdaType);
}

Expression Environment::get_lamb(const Atom & sym) const {

	Expression exp;

	if (sym.isSymbol()) {
		auto result = envmap.find(sym.asSymbol());
		if ((result != envmap.end()) && (result->second.type == LambdaType)) {
			exp = result->second.exp;
		}
	}

	return exp;
}

void Environment::add_lamb(const Atom & sym, const Expression & exp) {

	if (!sym.isSymbol()) {
		throw SemanticError("Attempt to add non-symbol to environment");
	}

	// error if overwriting symbol map
	if (envmap.find(sym.asSymbol()) != envmap.end()) {
		throw SemanticError("Attempt to overwrite symbol in environemnt");
	}

	envmap.emplace(sym.asSymbol(), EnvResult(LambdaType, exp));
}

/*
Reset the environment to the default state. First remove all entries and
then re-add the default ones.
 */
void Environment::reset(){

  envmap.clear();
  
  // Built-In value of pi
  envmap.emplace("pi", EnvResult(ExpressionType, Expression(PI)));
  
  // Built-In value of pi
  envmap.emplace("-pi", EnvResult(ExpressionType, Expression(negPI)));
  
  // Built-In value of e
  envmap.emplace("e", EnvResult(ExpressionType, Expression(EXP)));
  
  // Built-In value of I
  envmap.emplace("I", EnvResult(ExpressionType, Expression(I)));

  // Built-In value of I
  envmap.emplace("-I", EnvResult(ExpressionType, Expression(negI)));

  // Procedure: add;
  envmap.emplace("+", EnvResult(ProcedureType, add)); 

  // Procedure: subneg;
  envmap.emplace("-", EnvResult(ProcedureType, subneg)); 

  // Procedure: mul;
  envmap.emplace("*", EnvResult(ProcedureType, mul)); 

  // Procedure: div;
  envmap.emplace("/", EnvResult(ProcedureType, div)); 
  
  // Procedure: sqrt;
  envmap.emplace("sqrt", EnvResult(ProcedureType, sqrt)); 
  
  // Procedure: expo;
  envmap.emplace("^", EnvResult(ProcedureType, expo));
  
  // Procedure: ln;
  envmap.emplace("ln", EnvResult(ProcedureType, ln));
  
  // Procedure: sin;
  envmap.emplace("sin", EnvResult(ProcedureType, sin));
  
  // Procedure: cos;
  envmap.emplace("cos", EnvResult(ProcedureType, cos));
  
  // Procedure: tan;
  envmap.emplace("tan", EnvResult(ProcedureType, tan));
  
  // Procedure: real;
  envmap.emplace("real", EnvResult(ProcedureType, real));
  
  // Procedure: imag;
  envmap.emplace("imag", EnvResult(ProcedureType, imag));
  
  // Procedure: mag;
  envmap.emplace("mag", EnvResult(ProcedureType, mag));
  
  // Procedure: arg;
  envmap.emplace("arg", EnvResult(ProcedureType, arg));
  
  // Procedure: conj;
  envmap.emplace("conj", EnvResult(ProcedureType, conj));

  // Procedure: list;
  envmap.emplace("list", EnvResult(ProcedureType, list));

  // Procedure: first;
  envmap.emplace("first", EnvResult(ProcedureType, first));

  // Procedure: rest;
  envmap.emplace("rest", EnvResult(ProcedureType, rest));

  // Procedure: length;
  envmap.emplace("length", EnvResult(ProcedureType, length));

  // Procedure: append;
  envmap.emplace("append", EnvResult(ProcedureType, append));

  // Procedure: join;
  envmap.emplace("join", EnvResult(ProcedureType, join));

  // Procedure: range;
  envmap.emplace("range", EnvResult(ProcedureType, range));

  // Procedure: lambda;
  envmap.emplace("lambda", EnvResult(SpecialType, lambda));

  //Procedure: apply
  envmap.emplace("apply", EnvResult(SpecialType, apply));
  
  //Procedure: map
  envmap.emplace("map", EnvResult(SpecialType, map));
}
