//
// keygrabber.h - Widget to grab a key and dismiss itself
//
// by James L. Hammons
// (C) 2011 Underground Software
//

#ifndef __KEYGRABBER_H__
#define __KEYGRABBER_H__

#include <QtGui>

//class GeneralTab;
//class ControllerTab;
//class AlpineTab;

class KeyGrabber: public QDialog
{
	Q_OBJECT

	public:
		KeyGrabber(QWidget * parent = 0);
		~KeyGrabber();
//		void UpdateVJSettings(void);
		void SetText(QString);
//		int GetKeyGrabbed(void);

	protected:
		void keyPressEvent(QKeyEvent *);

//	private:
//		void LoadDialogFromSettings(void);

	private:
//		QTabWidget * tabWidget;
//		QDialogButtonBox * buttonBox;
		QLabel * label;

	public:
		int key;
//		GeneralTab * generalTab;
//		ControllerTab * controllerTab;
//		AlpineTab * alpineTab;
};

#endif	// __KEYGRABBER_H__
