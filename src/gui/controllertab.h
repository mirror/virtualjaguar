#ifndef __CONTROLLERTAB_H__
#define __CONTROLLERTAB_H__

#include <QtGui>
#include <stdint.h>

class ControllerTab: public QWidget
{
	Q_OBJECT

	public:
		ControllerTab(QWidget * parent = 0);
		~ControllerTab();
		void UpdateLabel(void);

	protected slots:
		void DefineAllKeys(void);

	private:
		QPushButton * redefineAll;
		void DrawBorderedText(QPainter &, int, int, QString);

	public:
		uint32_t p1Keys[21];

	private:
		QLabel * controllerPic;

		// Class data
		static char keyName1[96][16];
		static char keyName2[64][16];
};

#endif	// __CONTROLLERTAB_H__
