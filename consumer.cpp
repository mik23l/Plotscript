#include "consumer.hpp"

Consumer::Consumer(inputQueue *inQ, outputQueue *outQ, Interpreter *interpreter) {
	
  inq = inQ;
  outq = outQ;
  interp = interpreter;
}

