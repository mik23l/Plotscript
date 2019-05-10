#include <QTest>

#include <QPlainTextEdit>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QtMath> 

#include "notebook_app.hpp"

class NotebookTest : public QObject {
  Q_OBJECT
  
public:
  int findLines(QGraphicsScene * scene, QRectF bbox, qreal margin);
  int findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents);
  int findPoints(QGraphicsScene * scene, QPointF center, qreal radius);
  int intersectsLine(QGraphicsScene * scene, QPointF center, qreal radius);

private slots:
  void initTestCase();
  void findByName();
  void errorOutput();
  void expressionOutput();
  void lambOutput();
  void pointOutput();
  void pointChecks();
  void lineOutput();
  void lineChecks();
  void textOutput();
  void textChecks();
  void listOutput();
  void discretePlot();
  void continuousPlot();

private:
  NotebookApp widget;
};

void NotebookTest::initTestCase(){
  widget.show();
}

void NotebookTest::findByName(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  QVERIFY2(in, "Could not find widget with name: 'input'");
  QVERIFY2(out, "Could not find widget with name: 'output'");
}

void NotebookTest::errorOutput(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
    
  in->setPlainText("(get-property \"key\" (3))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  auto item = scene->itemAt(QPointF(0, 0), QTransform());
  QGraphicsTextItem * text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  QString result = text->toPlainText();
  
  QCOMPARE(result, QString("NONE"));
 
  in->clear();
  
  in->setPlainText("(begin))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  item = scene->itemAt(QPointF(0, 0), QTransform());
  text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  result = text->toPlainText();
  
  QCOMPARE(result, QString("Error: Invalid Expression. Could not parse."));
  
  in->clear();
  
  in->setPlainText("(begin (define a I) (first a))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  item = scene->itemAt(QPointF(0, 0), QTransform());
  text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  result = text->toPlainText();
  
  QCOMPARE(result, QString("Error in call to first: argument must be a list."));
  
  in->clear();
}

void NotebookTest::expressionOutput(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
    
  in->setPlainText("(cos pi)");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  auto item = scene->itemAt(QPointF(0, 0), QTransform());
  QGraphicsTextItem * text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  QString result = text->toPlainText();
  
  QCOMPARE(result, QString("(-1)"));
 
  in->clear();
  
  in->setPlainText("(begin (define title \"The Title\") (title))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  item = scene->itemAt(QPointF(0, 0), QTransform());
  text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  result = text->toPlainText();
  
  QCOMPARE(result, QString("(\"The Title\")"));
  
  in->clear();
}

void NotebookTest::lambOutput(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
    
  in->setPlainText("(define inc (lambda (x) (+ x 1)))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  QList<QGraphicsItem *> items = scene->items();
  
  QCOMPARE(items.size(), 0);
    
  in->clear();
}

void NotebookTest::pointOutput(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
  
  in->setPlainText("(make-point 0 0)");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  QList<QGraphicsItem *> items = scene->items();
  
  QGraphicsItem * point = items[0];
  QGraphicsEllipseItem * result = qgraphicsitem_cast<QGraphicsEllipseItem *>(point);
  
  auto rect = result->rect();
  auto size = rect.size();
   
  auto position = rect.center();
  
  QCOMPARE(size, QSizeF(QSize(0, 0)));
  QCOMPARE(position, QPointF(0, 0));
 
  in->clear();
  
  in->setPlainText("(set-property \"size\" 20 (make-point 0 0))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  items.clear();
  items = scene->items();
  
  point = items[0];
  result = qgraphicsitem_cast<QGraphicsEllipseItem *>(point);
  
  rect = result->rect();
  size = rect.size();
   
  position = rect.center();
  
  QCOMPARE(size, QSizeF(QSize(20, 20)));
  QCOMPARE(position, QPointF(0, 0));
  
  in->clear();
}

void NotebookTest::pointChecks(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
    
  in->setPlainText("(set-property \"size\" -2 (make-point 0 0))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  auto item = scene->itemAt(QPointF(0, 0), QTransform());
  QGraphicsTextItem * text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  QString result = text->toPlainText();
  
  QCOMPARE(result, QString("Error: point size not positive."));
 
  in->clear();
}

void NotebookTest::lineOutput(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
  
  in->setPlainText("(make-line (make-point 0 0) (make-point 20 20))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  QList<QGraphicsItem *> items = scene->items();
  
  QGraphicsItem * point = items[0];
  QGraphicsLineItem * result = qgraphicsitem_cast<QGraphicsLineItem *>(point);
  
  auto line = result->line();
  auto pen = result->pen();
  
  QCOMPARE(line, QLineF(QPointF(0, 0), QPointF(20, 20)));
  QCOMPARE(pen.width(), 1);
 
  in->clear();
  
  in->setPlainText("(set-property \"thickness\" (4) (make-line (make-point 0 0) (make-point 20 20)))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  items.clear();
  items = scene->items();
  
  point = items[0];
  result = qgraphicsitem_cast<QGraphicsLineItem *>(point);
  
  line = result->line();
  pen = result->pen();
      
  QCOMPARE(line, QLineF(QPointF(0, 0), QPointF(20, 20)));
  QCOMPARE(pen.width(), 4);
  
  in->clear();
}

void NotebookTest::lineChecks(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
    
  in->setPlainText("(make-line (set-property \"size\" -3 (make-point 0 0)) (make-point 20 20))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  auto item = scene->itemAt(QPointF(0, 0), QTransform());
  QGraphicsTextItem * text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  QString result = text->toPlainText();
  
  QCOMPARE(result, QString("Error: point size not positive."));
 
  in->clear();
  
  in->setPlainText("(make-line 5 4)");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  item = scene->itemAt(QPointF(0, 0), QTransform());
  text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  result = text->toPlainText();
  
  QCOMPARE(result, QString("Error: line not made up of points."));
  
  in->clear();
  
  in->setPlainText("(set-property \"thickness\" (-4) (make-line (make-point 0 0) (make-point 20 20)))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  item = scene->itemAt(QPointF(0, 0), QTransform());
  text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  result = text->toPlainText();
  
  QCOMPARE(result, QString("Error: line thickness not positive."));
  
  in->clear();
}

void NotebookTest::textOutput(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
    
  in->setPlainText("(make-text \"Hello World!\")");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
   
  auto item = scene->itemAt(QPointF(0, 0), QTransform());
  QGraphicsTextItem * text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  QString result = text->toPlainText();
  //auto position = text->pos();
       
  //QCOMPARE(position, QPointF(-7.5625,-5));
  QCOMPARE(text->scale(), double(1));
  QCOMPARE(text->rotation(), double(0));
  QCOMPARE(result, QString("Hello World!"));
 
  in->clear();
  
  in->setPlainText("(set-property \"scale\" (3) (set-property \"rotation\" (1) (make-text \"Hello World!\")))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
   
  item = scene->itemAt(QPointF(0, 0), QTransform());
  text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  result = text->toPlainText();
  //position = text->pos();
        
  //QCOMPARE(position, QPointF(-7.5625,-5));
  QCOMPARE(text->scale(), double(3));
  double degrees = qRadiansToDegrees(double(1));
  QCOMPARE(text->rotation(), degrees);
  QCOMPARE(result, QString("Hello World!"));
 
  in->clear();
  
}

void NotebookTest::textChecks(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
    
  in->setPlainText("(set-property \"position\" (set-property \"size\" -20 (make-point 0 0)) (make-text \"Hi\"))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  auto item = scene->itemAt(QPointF(0, 0), QTransform());
  QGraphicsTextItem * text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  QString result = text->toPlainText();
  
  QCOMPARE(result, QString("Error: point size not positive or point scale not positive."));
 
  in->clear();
  
  in->setPlainText("(set-property \"position\" (0) (make-text \"Hi\"))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  item = scene->itemAt(QPointF(0, 0), QTransform());
  text = qgraphicsitem_cast<QGraphicsTextItem *>(item);
  
  result = text->toPlainText();
  
  QCOMPARE(result, QString("Error: position not a point."));
}

void NotebookTest::listOutput(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
    
  in->setPlainText("(list (set-property \"size\" 1 (make-point 0 0)) (set-property \"size\" 2 (make-point 4 0)) (set-property \"size\" 4 (make-point 8 0)) (set-property \"size\" 8 (make-point 16 0)) (set-property \"size\" 16 (make-point 32 0)) (set-property \"size\" 32 (make-point 64 0)))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  QList<QGraphicsItem *> items = scene->items();
  
  for(int i = 0; i < 6; ++i){
	  QGraphicsItem * point = items[i];
	  QGraphicsEllipseItem * result = qgraphicsitem_cast<QGraphicsEllipseItem *>(point);
	  
	  auto rect = result->rect();
	  auto size = rect.size();
	  auto position = rect.center();
	  
	  if (i == 0){
		QCOMPARE(size, QSizeF(QSize(32, 32)));
		QCOMPARE(position, QPointF(64, 0));
	  }
	  else if (i == 1){
		QCOMPARE(size, QSizeF(QSize(16, 16)));
		QCOMPARE(position, QPointF(32, 0));
	  }
	  else if (i == 2){
		QCOMPARE(size, QSizeF(QSize(8, 8)));
		QCOMPARE(position, QPointF(16, 0));
	  }
	  else if (i == 3){
		QCOMPARE(size, QSizeF(QSize(4, 4)));
		QCOMPARE(position, QPointF(8, 0));
	  }
	  else if (i == 4){
		QCOMPARE(size, QSizeF(QSize(2, 2)));
		QCOMPARE(position, QPointF(4, 0));
	  }
	  else{
		QCOMPARE(size, QSizeF(QSize(1, 1)));
		QCOMPARE(position, QPointF(0, 0));
	  }
  }
  
  in->clear();
  
  in->setPlainText("(list (make-line (make-point 0 0) (make-point 0 20)) (make-line (make-point 10 0) (make-point 10 20)) (make-line (make-point 20 0) (make-point 20 20)))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  items.clear();
  items = scene->items();
  
  for(int i = 0; i < 3; ++i){
	  QGraphicsItem * point = items[i];
	  QGraphicsLineItem * result = qgraphicsitem_cast<QGraphicsLineItem *>(point);
	  
	  auto line = result->line();
	  auto pen = result->pen();
	  
	  if (i == 0){
		QCOMPARE(line, QLineF(QPointF(20, 0), QPointF(20, 20)));
	    QCOMPARE(pen.width(), 1);
	  }
	  else if (i == 1){
		QCOMPARE(line, QLineF(QPointF(10, 0), QPointF(10, 20)));
	    QCOMPARE(pen.width(), 1);
	  }
	  else{
		QCOMPARE(line, QLineF(QPointF(0, 0), QPointF(0, 20)));
	    QCOMPARE(pen.width(), 1);
	  }
  }
    
  in->clear();
  
  in->setPlainText("(begin (define xloc 0) (define yloc 0) (list (set-property \"position\" (make-point (+ xloc 20) yloc) (make-text \"Hi\")) (set-property \"position\" (make-point (+ xloc 40) yloc) (make-text \"Hi\")) (set-property \"position\" (make-point (+ xloc 60) yloc) (make-text \"Hi\")) (set-property \"position\" (make-point (+ xloc 80) yloc) (make-text \"Hi\")) (set-property \"position\" (make-point (+ xloc 100) yloc) (make-text \"Hi\"))))");
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  items.clear();
  items = scene->items();
    
  for(int i = 0; i < 5; ++i){
	  QGraphicsTextItem * text = qgraphicsitem_cast<QGraphicsTextItem *>(items[i]);
  	  QString result = text->toPlainText();
	  //auto position = text->pos();
	  
	  if (i == 0){
		QCOMPARE(result, QString("Hi"));
		//QCOMPARE(position, QPointF(88, -5));
		QCOMPARE(text->scale(), double(1));
		QCOMPARE(text->rotation(), double(0));
	  }
	  else if (i == 1){
		QCOMPARE(result, QString("Hi"));
		//QCOMPARE(position, QPointF(68, -5));
		QCOMPARE(text->scale(), double(1));
		QCOMPARE(text->rotation(), double(0));
	  }
	  else if (i == 2){
		QCOMPARE(result, QString("Hi"));
		//QCOMPARE(position, QPointF(48, -5));
		QCOMPARE(text->scale(), double(1));
		QCOMPARE(text->rotation(), double(0));
	  }
	  else if (i == 3){
		QCOMPARE(result, QString("Hi"));
		//QCOMPARE(position, QPointF(28, -5));
		QCOMPARE(text->scale(), double(1));
		QCOMPARE(text->rotation(), double(0));
	  }
	  else{
		QCOMPARE(result, QString("Hi"));
		//QCOMPARE(position, QPointF(8, -5));
		QCOMPARE(text->scale(), double(1));
		QCOMPARE(text->rotation(), double(0));
	  }
  }
  
  in->clear();
}

void NotebookTest::discretePlot(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
  
  std::string program = R"((discrete-plot (list (list -1 -1) (list 1 1)) 
		  (list (list "title" "The Title") 
          (list "abscissa-label" "X Label") 
          (list "ordinate-label" "Y Label") ))
		  )";
  
  in->setPlainText(QString::fromStdString(program));
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  // first check total number of items
  // 8 lines + 2 points + 7 text = 17
  auto items = scene->items();
  QCOMPARE(items.size(), 17);
  
  // make them all selectable
  foreach(auto item, items){
    item->setFlag(QGraphicsItem::ItemIsSelectable);
  }
 
  double scalex = 20.0/2.0;
  double scaley = 20.0/2.0;

  double xmin = scalex*-1;
  double xmax = scalex*1;
  double ymin = scaley*-1;
  double ymax = scaley*1;
  double xmiddle = (xmax+xmin)/2;
  double ymiddle = (ymax+ymin)/2;
  
   // check title
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax+3)), 0, QString("The Title")), 1);
  
  // check abscissa label
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin-3)), 0, QString("X Label")), 1);
  
  // check ordinate label
  QCOMPARE(findText(scene, QPointF(xmin-3, -ymiddle), -90, QString("Y Label")), 1);

  // check abscissa min label
  QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-1")), 1);

  // check abscissa max label
  QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("1")), 1);

  // check ordinate min label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-1")), 1);

  // check ordinate max label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("1")), 1);
  
  // check the bounding box bottom
  QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);

  // check the bounding box top
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);

  // check the bounding box left and (-1, -1) stem
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 2);

  // check the bounding box right and (1, 1) stem
  QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 2);

  // check the abscissa axis
  QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);

  // check the ordinate axis 
  QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);
  
  // check the point at (-1,-1)
  QCOMPARE(findPoints(scene, QPointF(-10, 10), 0.6), 1);
    
  // check the point at (1,1)
  QCOMPARE(findPoints(scene, QPointF(10, -10), 0.6), 1); 
}

void NotebookTest::continuousPlot(){
  auto in = widget.findChild<InputWidget *>("input");
  auto out = widget.findChild<OutputWidget *>("output");
  
  auto view = out->findChild<QGraphicsView *>();
  auto scene = view->scene();
  
  std::string program = R"((begin (define j (lambda (x) (+ (* 2 x) 1))) (continuous-plot j (list -2 2) (list (list "title" "The linear function y=2x+1") (list "abscissa-label" "x") (list "ordinate-label" "y") (list "text-scale" 2)))))";
  
  in->setPlainText(QString::fromStdString(program));
  QTest::keyRelease(in, Qt::Key_Return, Qt::ShiftModifier, 10);
  
  auto items = scene->items();
  
  // make them all selectable
  foreach(auto item, items){
    item->setFlag(QGraphicsItem::ItemIsSelectable);
  }
 
  double scalex = 20.0/4.0;
  double scaley = 20.0/8.0;

  double xmin = scalex*-2;
  double xmax = scalex*2;
  double ymin = scaley*-3;
  double ymax = scaley*5;
  double xmiddle = (xmax+xmin)/2;
  double ymiddle = (ymax+ymin)/2;
  
   // check title
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax+3)), 0, QString("The linear function y=2x+1")), 1);
  
  // check abscissa label
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin-3)), 0, QString("x")), 1);
  
  // check ordinate label
  QCOMPARE(findText(scene, QPointF(xmin-3, -ymiddle), -90, QString("y")), 1);

  // check abscissa min label
  QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-2")), 1);

  // check abscissa max label
  QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("2")), 1);

  // check ordinate min label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-3")), 1);

  // check ordinate max label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("5")), 1);
  
  // check the bounding box bottom
  QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);

  // check the bounding box top
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);

  // check the bounding box left and (-1, -1) stem
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 1);

  // check the bounding box right and (1, 1) stem
  QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 1);

  // check the abscissa axis
  QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);

  // check the ordinate axis 
  QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);
}

/* 
findText - find text in a scene centered at a specified point with a given 
           rotation and string contents  
 */
int NotebookTest::findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents){
  
  int numtext(0);
  foreach(auto item, scene->items(center)){
    if(item->type() == QGraphicsTextItem::Type){
      QGraphicsTextItem * text = static_cast<QGraphicsTextItem *>(item);
      if((text->toPlainText() == contents) &&
     (text->rotation() == rotation) &&
     (text->pos() + text->boundingRect().center() == center)){
    numtext += 1;
      }
    }
  }

  return numtext;
}

/* 
findLines - find lines in a scene contained within a bounding box 
            with a small margin
 */
int NotebookTest::findLines(QGraphicsScene * scene, QRectF bbox, qreal margin){

  QPainterPath selectPath;

  QMarginsF margins(margin, margin, margin, margin);
  selectPath.addRect(bbox.marginsAdded(margins));
  scene->setSelectionArea(selectPath, Qt::ContainsItemShape);
  
  int numlines(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsLineItem::Type){
      numlines += 1;
    }
  }

  return numlines;
}

/* 
findPoints - find points in a scene contained within a specified rectangle
 */
int NotebookTest::findPoints(QGraphicsScene * scene, QPointF center, qreal radius){
  
  QPainterPath selectPath;
  selectPath.addRect(QRectF(center.x()-radius, center.y()-radius, 2*radius, 2*radius));
  scene->setSelectionArea(selectPath, Qt::ContainsItemShape);

  int numpoints(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsEllipseItem::Type){
      numpoints += 1;
    }
  }

  return numpoints;
}

/* 
intersectsLine - find lines in a scene that intersect a specified rectangle
 */
int NotebookTest::intersectsLine(QGraphicsScene * scene, QPointF center, qreal radius){
              
  QPainterPath selectPath;
  selectPath.addRect(QRectF(center.x()-radius, center.y()-radius, 2*radius, 2*radius));
  scene->setSelectionArea(selectPath, Qt::IntersectsItemShape);

  int numlines(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsLineItem::Type){
      numlines += 1;
    }
  }

  return numlines;
}

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
