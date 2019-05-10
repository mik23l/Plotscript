#ifndef INPUT_WIDGET_H
#define INPUT_WIDGET_H

#include <QObject>
#include <QPlainTextEdit>

class QPlainTextEdit;

class InputWidget : public QPlainTextEdit{
    Q_OBJECT
	
public:
    InputWidget(QPlainTextEdit *parent = nullptr);
	
public slots:
	void sendString();
	
protected:
    void keyReleaseEvent(QKeyEvent *);
	
signals:
    void keyPressShiftEnter();
	void send(QString line);
	void changedScene();

private:
	QPlainTextEdit * input;
};

#endif // INPUT_WIDGET_H