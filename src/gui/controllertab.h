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
		void DrawBorderedText(QPainter &, int, int, QString);

	public:
		int p1Keys[21];

	// Class data
	private:
		static char keyName1[96][16];
		static char keyName2[64][16];
};

#endif	// __CONTROLLERTAB_H__
