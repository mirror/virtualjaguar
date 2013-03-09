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
//		QSize sizeHint(void) const;

	protected slots:
		void DefineAllKeys(void);

	private:
		QLabel * label;
		QComboBox * profile;
		QPushButton * redefineAll;

	public:
		ControllerWidget * controllerWidget;
};

#endif	// __CONTROLLERTAB_H__
