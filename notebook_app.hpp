#ifndef NOTEBOOK_APP_H
#define NOTEBOOK_APP_H

#include "input_widget.hpp"
#include "output_widget.hpp"

#include "interpreter.hpp"
#include "startup_config.hpp"
#include "message_queue.hpp"
#include "consumer.hpp"

#include <thread>
#include <chrono>

#include <QWidget>
#include <QEventLoop>

class InputWidget;
class OutputWidget;
class QPushButton;
class QTimer;

class NotebookApp : public QWidget{
    Q_OBJECT

public:
    NotebookApp(QWidget *parent = 0);
	~NotebookApp();
	void startup();
	void isExpression(Expression exp);
	void isLambda();
	void isList(Expression exp);
	void isPoint(Expression exp);
	void isLine(Expression exp);
	void isText(Expression exp);
	
public slots:
	void eval(QString line);
	void loop();
	void Start();
    void Stop();
    void Reset();
    void Interrupt();
	
signals:
	void changedScene();
	void changedError(QString error);
	void changedExpression(QString exp);
	void changedLambda();
	void changedPoint(double x, double y, double size);
	void changedLine(double thickness, double x1, double y1, double x2, double y2);
	void changedText(double x, double y, QString text, double scale, double rotation);
	void changedDiscrete(QList<QGraphicsEllipseItem *> points, QList<QGraphicsLineItem *> lines, QList<QGraphicsTextItem *> labels);
	void finishEval();
	
	
private:
	InputWidget * input;
	OutputWidget * output;
	QPushButton * start;
    QPushButton * stop;
    QPushButton * reset;
    QPushButton * interrupt;
	QTimer * timer;
	
	Interpreter interp;
	inputQueue inQ;
    outputQueue outQ;
	Consumer *c1;
	std::thread *consumer_thread;
	bool run = true;
	bool killLoopFlag_;
};

#endif // NOTEBOOK_APP_H
