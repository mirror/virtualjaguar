//
// keygrabber.h - Widget to grab a key and dismiss itself
//
// by James L. Hammons
// (C) 2011 Underground Software
//

#ifndef __KEYGRABBER_H__
#define __KEYGRABBER_H__

#include <QtGui>

class KeyGrabber: public QDialog
{
	Q_OBJECT

	public:
		KeyGrabber(QWidget * parent = 0);
		~KeyGrabber();
		void SetKeyText(int);

	protected:
		void keyPressEvent(QKeyEvent *);

private:
		QLabel * label;

	public:
		int key;
};

#endif	// __KEYGRABBER_H__
