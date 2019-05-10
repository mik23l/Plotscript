#include "output_widget.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QRect>
#include <QFont>
#include <QLayout>
#include <QtMath> 

#include <QDebug>

OutputWidget::OutputWidget(QWidget *parent) : QWidget(parent)
{
	scene = new QGraphicsScene();

    view = new QGraphicsView();
    view->setScene(scene);
	view->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    view->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	
    auto outputLayout = new QVBoxLayout();
    outputLayout->addWidget(view);

    setLayout(outputLayout);
}

void OutputWidget::resizeEvent(QResizeEvent *)
{
	scene->setSceneRect(scene->itemsBoundingRect());
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void OutputWidget::updateScene()
{
	scene->clear();
}

void OutputWidget::updateError(QString error)
{
    QGraphicsTextItem * text = new QGraphicsTextItem(error);
	scene->addItem(text);
	
	scene->setSceneRect(scene->itemsBoundingRect());
	view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void OutputWidget::updateExpression(QString exp)
{
    QGraphicsTextItem * text = new QGraphicsTextItem(exp);
	scene->addItem(text);
	
	scene->setSceneRect(scene->itemsBoundingRect());
	view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void OutputWidget::updateLambda()
{
	scene->clear();
}

void OutputWidget::updatePoint(double x, double y, double size)
{
	QGraphicsEllipseItem * point = new QGraphicsEllipseItem((x-(size/2)), (y-(size/2)), size, size);
	QPen pen;
	pen.setWidth(0);
	point->setBrush(QBrush(Qt::black));
	point->setPen(pen);
	scene->addItem(point);
	
	scene->setSceneRect(scene->itemsBoundingRect());
	view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void OutputWidget::updateLine(double thickness, double x1, double y1, double x2, double y2)
{
	QPen pen;
	pen.setWidth(thickness);
	QGraphicsLineItem * line = new QGraphicsLineItem(x1, y1, x2, y2);
	line->setPen(pen);
	scene->addItem(line);
	
	scene->setSceneRect(scene->itemsBoundingRect());
	view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void OutputWidget::updateText(double x, double y, QString text, double scale, double rotation)
{
    QGraphicsTextItem * textItem;
	auto font = QFont("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPointSize(1);
	std::string str = text.toStdString();
	
	if(str.size() > 2){
		str.erase(0, 2);
		str.erase(str.size() - 2);
	}
    textItem = scene->addText(QString::fromStdString(str));
	textItem->setFont(font);
	textItem->setPos((x-(textItem->boundingRect().width()/2)), (y-(textItem->boundingRect().height()/2)));
	textItem->setTransformOriginPoint(textItem->boundingRect().center());
		
	textItem->setScale(scale);
	double degrees = qRadiansToDegrees(rotation);
	textItem->setRotation(degrees);
		
	scene->setSceneRect(scene->itemsBoundingRect());
	view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}