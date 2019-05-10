#ifndef OUTPUT_WIDGET_H
#define OUTPUT_WIDGET_H

#include <QWidget>

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QList>

class QGraphicsScene;
class QGraphicsView;

class OutputWidget : public QWidget{
    Q_OBJECT
	
public:
    OutputWidget(QWidget *parent = nullptr);
	
protected:
    void resizeEvent(QResizeEvent *);

private:
	QGraphicsScene * scene;
    QGraphicsView * view;
	
public slots:
	void updateScene();
	void updateError(QString error);
	void updateExpression(QString exp);
	void updateLambda();
	void updatePoint(double x, double y, double size);
	void updateLine(double thickness, double x1, double y1, double x2, double y2);
	void updateText(double x, double y, QString text, double scale, double rotation);
};

#endif // OUTPUT_WIDGET_H