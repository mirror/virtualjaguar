//
// about.h: Credits where credits are due ;-)
//
// by James L. Hammons
// (C) 2010 Underground Software
//

#ifndef __ABOUT_H__
#define __ABOUT_H__

#include <QtGui>

class AboutWindow: public QWidget
{
	public:
		AboutWindow(QWidget * parent = 0);

	private:
		QVBoxLayout * layout;
		QLabel * text;
};

#endif	// __ABOUT_H__


#if 0
class AboutWindow : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  struct Logo : public QWidget {
    void paintEvent(QPaintEvent*);
  } *logo;
  QLabel *info;

  AboutWindow();
};

extern AboutWindow *aboutWindow;
#endif
