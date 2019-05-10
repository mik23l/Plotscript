#include "notebook_app.hpp"
#include "input_widget.hpp"
#include "output_widget.hpp"

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "consumer.hpp"

#include <QLayout>
#include <QPushButton>
#include <QString>
#include <QTimer>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <thread>
#include <chrono>

NotebookApp::NotebookApp(QWidget *parent) : QWidget(parent) 
{
	input = new InputWidget();
	input->setObjectName("input");
    output = new OutputWidget();
	output->setObjectName("output");
	
	start = new QPushButton("Start Kernel");
    start->setObjectName("start");
    stop = new QPushButton("Stop Kernel");
    stop->setObjectName("stop");
    reset = new QPushButton("Reset Kernel");
    reset->setObjectName("reset");
    interrupt = new QPushButton("Interrupt");
    interrupt->setObjectName("interrupt");
	
	timer = new QTimer(this);

    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(start);
    buttonLayout->addWidget(stop);
    buttonLayout->addWidget(reset);
    buttonLayout->addWidget(interrupt);

    auto layout = new QVBoxLayout();
	layout->addLayout(buttonLayout);
    layout->addWidget(input);
    layout->addWidget(output);
	
	connect(input, SIGNAL(changedScene()), output, SLOT(updateScene()));
	
	connect(timer, SIGNAL(timeout()), this, SLOT(loop()));
	
	connect(this, SIGNAL(changedScene()), output, SLOT(updateScene()));
	connect(this, SIGNAL(changedError(QString)), output, SLOT(updateError(QString)));
	connect(this, SIGNAL(changedExpression(QString)), output, SLOT(updateExpression(QString)));
	connect(this, SIGNAL(changedLambda()), output, SLOT(updateLambda()));
	connect(this, SIGNAL(changedPoint(double, double, double)), output, SLOT(updatePoint(double, double, double)));
	connect(this, SIGNAL(changedLine(double, double, double, double, double)), output, SLOT(updateLine(double, double, double, double, double)));
	connect(this, SIGNAL(changedText(double, double, QString, double, double)), output, SLOT(updateText(double, double, QString, double, double)));
	
	connect(input, SIGNAL(send(QString)), this, SLOT(eval(QString)));
	
	connect(start, SIGNAL(pressed()), this, SLOT(Start()));
    connect(stop, SIGNAL(pressed()), this, SLOT(Stop()));
    connect(reset, SIGNAL(pressed()), this, SLOT(Reset()));
    connect(interrupt, SIGNAL(pressed()), this, SLOT(Interrupt()));
		
    setLayout(layout);
	
	c1 = new Consumer(&inQ, &outQ, &interp);
	consumer_thread = new std::thread(*c1);
	
	startup();
}

NotebookApp::~NotebookApp()
{
	inQ.push("%exit");
	//send stop to thread
	consumer_thread->join();
	delete consumer_thread;
	delete c1;
}

void NotebookApp::startup()
{
	std::ifstream ifs(STARTUP_FILE);
  
    if(!ifs){
	  emit changedScene();
	  emit changedError("Error: Could not open file for reading.");
    }
  
    if(!interp.parseStream(ifs)){
	  emit changedScene();
	  emit changedError("Error: Invalid Expression. Could not parse.");
    }
    else{
  	  try{
	    Expression start = interp.evaluate();
	  }
	  catch(const SemanticError & ex){
	    std::string error(ex.what());
		QString err = QString::fromStdString(error);
		emit changedScene();
		emit changedError(err);
	  }	
    }
}

void NotebookApp::loop()
{
	
}

void NotebookApp::eval(QString line)
{
	std::string strline = (line.toStdString());
	
	emit changedScene();
	
	if(run) {
		inQ.push(strline);
		
		Expression exp;
		//check output queue for result
		while (!outQ.try_pop(exp)) {
			timer->start(1);
		}
		
		std::string name = exp.objName();

		if(name == "\"point\"") {
			isPoint(exp);
		}
		else if(name == "\"line\"") {
			isLine(exp);
		}
		else if(name == "\"text\"") {
			isText(exp);
		}
		else if(exp.head().asSymbol() == "lambda") {
			isLambda();
		}
		else if(exp.head().asSymbol() == "List") {
			isList(exp);
		}
		else
			isExpression(exp);	
	}
	else 
		emit changedError("Error: interpreter kernel not running");
}

void NotebookApp::Start()
{
	if(run){}
	else{
		delete consumer_thread;
		run = true;
		//start thread
		consumer_thread = new std::thread(*c1);
	}	
}

void NotebookApp::Stop()
{
	if(run){
		inQ.push("%stop");
		//stop thread
		consumer_thread->join();
		run = false;
	}
	else{}	
}

void NotebookApp::Reset()
{
	if(run){	
		inQ.push("%reset");
		//stop thread
		consumer_thread->join();
		//reset environment
		run = true;
		Interpreter temp;
		interp = temp;
		startup();
		delete c1;
		delete consumer_thread;
		//start thread
		c1 = new Consumer(&inQ, &outQ, &interp);
		consumer_thread = new std::thread(*c1);
	}
	else {
		run = true;
		//reset environment
		Interpreter temp;
		interp = temp;
		startup();
		delete c1;
		delete consumer_thread;	
		//start thread
		c1 = new Consumer(&inQ, &outQ, &interp);
		consumer_thread = new std::thread(*c1);
	}	
}

void NotebookApp::Interrupt()
{
	Reset();
	emit changedScene();
	emit changedError("Error: interpreter kernel interrupted");
}

void NotebookApp::isExpression(Expression exp)
{
	std::stringstream output;
	output << exp;
	emit changedExpression(QString::fromStdString(output.str()));
}

void NotebookApp::isLambda()
{
	emit changedLambda();
}

void NotebookApp::isList(Expression exp)
{
	for (auto v = exp.tailConstBegin(); v != exp.tailConstEnd(); ++v) {
	  Expression value = (*v);
	  std::string valObj = value.objName();
	  if(valObj == "\"point\"") {
		isPoint(value);
	  }
	  else if(valObj == "\"line\"") {
		isLine(value);
  	  }
	  else if(valObj == "\"text\"") {
		isText(value);
	  }
	  else if(value.head().asSymbol() == "List") {
		isList(value);
	  }
	  else
		isExpression(value);
	}	
}

void NotebookApp::isPoint(Expression exp)
{
	QList<double> coord;
	for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
	  coord.append((*e).head().asNumber());
	}
	double x = coord[0];
	double y = coord[1];
	double size = exp.pointSize();
	if (size >= 0)
		emit changedPoint(x, y, size);
	else
		emit changedError("Error: point size not positive.");
}
	
void NotebookApp::isLine(Expression exp)
{
	QList<double> coords;
	bool point = true;
	bool error = false;
	for (auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e) {
		Expression p = (*e);
		if(error != true) {
			if(p.objName() == "\"point\"") {
				if(p.pointSize() >= 0){
					for (auto ee = (*e).tailConstBegin(); ee != (*e).tailConstEnd(); ++ee) {
						coords.push_back((*ee).head().asNumber());
					}
				}
				else {
					point = false;
					emit changedError("Error: point size not positive.");
					error = true;
				}
			}
			else {
				point = false;
				emit changedError("Error: line not made up of points.");
				error = true;
			}
		}
		
	}
	if(error != true) {
		double thickness = exp.lineThick();
		if((point == true) && (thickness >= 0)) {
			double x1 = coords[0];
			double y1 = coords[1];
			double x2 = coords[2];
			double y2 = coords[3];
			emit changedLine(thickness, x1, y1, x2, y2);
		}
		else
			emit changedError("Error: line thickness not positive.");
	}
}
	
void NotebookApp::isText(Expression exp)
{
	std::stringstream output;
	output << exp;
	
	Expression point;
	point = exp.textPos();
	
	std::string valObj = point.objName();
    if(valObj == "\"point\"") {
  	  
	  QList<double> coord;
	  for (auto e = point.tailConstBegin(); e != point.tailConstEnd(); ++e) {
		  coord.append((*e).head().asNumber());
	  }
	  double x = coord[0];
	  double y = coord[1];
	  double size = point.pointSize();
	  double scale = exp.textScale();
	  double rotation = exp.textRotation();
	  if ((size >= 0) && (scale >= 0))
	  	  emit changedText(x, y, QString::fromStdString(output.str()), scale, rotation);
	  else
		  emit changedError("Error: point size not positive or point scale not positive.");
    }
	else
		emit changedError("Error: position not a point.");
}