#ifndef __CONTROLLERTAB_H__
#define __CONTROLLERTAB_H__

#include <QtGui>

class ControllerTab: public QWidget
{
	Q_OBJECT

	public:
		ControllerTab(QWidget * parent = 0);
		~ControllerTab();

	protected slots:
		void DefineAllKeys(void);

	public:
		QPushButton * redefineAll;
};

#endif	// __CONTROLLERTAB_H__
