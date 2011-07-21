#ifndef __CONTROLLERWIDGET_H__
#define __CONTROLLERWIDGET_H__

#include <QtGui>

class ControllerWidget: public QWidget
{
	Q_OBJECT

	public:
		ControllerWidget(QWidget * parent = 0);
		~ControllerWidget();

	//need paint, mousemove, mousedown, mouseup, etc
};

#endif	// __CONTROLLERWIDGET_H__
