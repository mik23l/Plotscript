#ifndef CONSUMER_HPP
#define CONSUMER_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "message_queue.hpp"

typedef message_queue<std::string> inputQueue;
typedef message_queue<Expression> outputQueue;

class Consumer {
public:
	Consumer(inputQueue *inQ, outputQueue *outQ, Interpreter *interpreter);

	void operator()() const {
		std::string line;
		while (true) {
			inq->wait_and_pop(line);
			if (line == "%stop" || line == "%reset" || line == "%exit") {
				break;
			}
			else if (line == "%start") {}
			else {
				std::istringstream expression(line);

				Expression exp;

				if (!interp->parseStream(expression)) {
					exp = (Atom("Error: Invalid Expression. Could not parse."));
					outq->push(exp);
				}
				else {
					try {
						exp = interp->evaluate();
						outq->push(exp);
					}
					catch (const SemanticError & ex) {
						std::string error(ex.what());
						exp = (Atom(error));
						outq->push(exp);
					}
				}
			}
		}		
	}

private:
	inputQueue *inq;
	outputQueue *outq;
	Interpreter *interp;
};

#endif