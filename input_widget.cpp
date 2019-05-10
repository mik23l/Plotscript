#include "input_widget.hpp"

#include <QPlainTextEdit>
#include <QString>
#include <QLayout>


InputWidget::InputWidget(QPlainTextEdit *parent) : QPlainTextEdit(parent)
{
	connect(this, SIGNAL(keyPressShiftEnter()), this, SLOT(sendString()));
}

void InputWidget::keyReleaseEvent(QKeyEvent * event)
{
    //Handle Shift-Enter
    if(((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) && (event->modifiers() == Qt::ShiftModifier)){
        emit keyPressShiftEnter();
    }
}

void InputWidget::sendString()
{
	QString line = (this->toPlainText());

	if(line.isEmpty()){}
	else{
		emit changedScene();
		emit send(line);			
	}
}
