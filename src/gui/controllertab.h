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

	private:
		QPushButton * redefineAll;

	public:
		int p1Keys[21];
};

#endif	// __CONTROLLERTAB_H__
