#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "message_queue.hpp"
#include "consumer.hpp"
#include "cntlc_tracer.cpp"

typedef message_queue<std::string> inputQueue;
typedef message_queue<Expression> outputQueue;

void prompt(){
  std::cout << "\nplotscript> ";
}

std::string readline(){
  std::string line;
  std::getline(std::cin, line);

  return line;
}

void error(const std::string & err_str){
  std::cerr << "Error: " << err_str << std::endl;
}

void info(const std::string & err_str){
  std::cout << "Info: " << err_str << std::endl;
}

int eval_from_stream(std::istream & stream){

  std::ifstream ifs(STARTUP_FILE);
  
  if(!ifs){
    error("Could not open file for reading.");
  }
  
  Interpreter interp;
  
  if(!interp.parseStream(ifs)){
    error("Invalid Program. Could not parse.");
  }
  else{
    try{
      Expression exp = interp.evaluate();
    }
    catch(const SemanticError & ex){
      std::cerr << ex.what() << std::endl;
    }	
  }
  
  if(!interp.parseStream(stream)){
    error("Invalid Program. Could not parse.");
    return EXIT_FAILURE;
  }
  else{
    try{
      Expression exp = interp.evaluate();
      std::cout << exp << std::endl;
    }
    catch(const SemanticError & ex){
      std::cerr << ex.what() << std::endl;
      return EXIT_FAILURE;
    }	
  }

  return EXIT_SUCCESS;
}

int eval_from_file(std::string filename){
      
  std::ifstream ifs(filename);
  
  if(!ifs){
    error("Could not open file for reading.");
    return EXIT_FAILURE;
  }
  
  return eval_from_stream(ifs);
}

int eval_from_command(std::string argexp){

  std::istringstream expression(argexp);

  return eval_from_stream(expression);
}

// A REPL is a repeated read-eval-print loop
void repl(){
	
  std::ifstream ifs(STARTUP_FILE);
  
  if(!ifs){
    error("Could not open file for reading.");
  }
   
  Interpreter interp;
  
  if(!interp.parseStream(ifs)){
    error("Invalid Program. Could not parse.");
  }
  else{
    try{
      Expression exp = interp.evaluate();
    }
    catch(const SemanticError & ex){
      std::cerr << ex.what() << std::endl;
    }	
  }
  
  inputQueue inQ;
  outputQueue outQ;
  
  bool run = true;
  
  Consumer *c1;
  c1 = new Consumer(&inQ, &outQ, &interp);
  std::thread *consumer_thread;
  consumer_thread = new std::thread(*c1);
  
  // call the platform-specific code
  install_handler();

  while (true) {

	  // reset the status flag
	  global_status_flag = 0;

	  prompt();

	  std::string line = readline();
	  
	  if (line == "%exit") {
		  if(run) {
			  inQ.push(line);
			  consumer_thread->join();
			  return;
		  }
		  else
			  return;
	  }

	  if (std::cin.fail() || std::cin.eof()) {
		  std::cin.clear(); // reset cin state
		  line.clear();
		  std::cout << "\n";
	  }

	  if (line.empty()) continue;

	  if (!run) {
		  if (line[0] != '%') {
			  std::cout << "Error: interpreter kernel not running" << std::endl;
		  }
		  if (line == "%start") {
			  delete consumer_thread;				
			  run = true;
			  //start thread
			  consumer_thread = new std::thread(*c1);
		  }
		  else if (line == "%reset") {
			  run = true;
			  //reset environment
			  //start thread
			  Interpreter temp;
			  interp = temp;
			  delete c1;
			  delete consumer_thread;							
			  c1 = new Consumer(&inQ, &outQ, &interp);
			  consumer_thread = new std::thread(*c1);
		  }
		  else if (line == "%stop") { run = false; }
	  }
	  else {
		  if (line == "%stop") {
			  inQ.push(line);
			  consumer_thread->join();
			  run = false;
		  }
		  else if (line == "%reset") {
			  inQ.push(line);
			  consumer_thread->join();
			  //reset environment
			  run = true;
			  Interpreter temp;
			  interp = temp;
			  delete c1;
			  delete consumer_thread;
			  //start thread			  
			  c1 = new Consumer(&inQ, &outQ, &interp);
			  consumer_thread = new std::thread(*c1);
		  }
		  else if (line == "%start") { run = true; }
		  else {
			  // reset the status flag
			  global_status_flag = 0;

			  //add line to input queue	
			  inQ.push(line);

			  Expression exp;
			  //check output queue for result
			  while (!outQ.try_pop(exp)) {
				  if ((global_status_flag > 0)){
					break;
				  }
				  std::this_thread::sleep_for(std::chrono::milliseconds(1));
			  }
			  if(global_status_flag == 0) {
				std::cout << exp << std::endl;
			  }
			  else {
				std::cout << "Error: interpreter kernel interrupted\n";
				outQ.try_pop(exp);
			  }
		  }
		}
	}
}

int main(int argc, char *argv[])
{	
  if(argc == 2){
	return eval_from_file(argv[1]);
  }
  else if(argc == 3){
	if(std::string(argv[1]) == "-e"){
	  return eval_from_command(argv[2]);
	}
	else{
	  error("Incorrect number of command line arguments.");
	}
  }
  else{
	repl();
	return EXIT_SUCCESS;
  }
	
  return EXIT_SUCCESS;
}
