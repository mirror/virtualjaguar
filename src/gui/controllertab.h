#ifndef __CONTROLLERTAB_H__
#define __CONTROLLERTAB_H__

#include <QtGui>
#include <stdint.h>

class ControllerWidget;

class ControllerTab: public QWidget
{
	Q_OBJECT

	public:
		ControllerTab(QWidget * parent = 0);
		~ControllerTab();

	protected slots:
		void DefineAllKeys(void);
		void ChangeProfile(int);

	private:
		QLabel * label;
		QComboBox * controllerList;
		QComboBox * profileList;
		QPushButton * redefineAll;

	public:
		ControllerWidget * controllerWidget;
		int profile;
};

#endif	// __CONTROLLERTAB_H__
