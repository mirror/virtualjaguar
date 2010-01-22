//
// about.cpp - Credits
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/21/2010  Created this file
//

#include "about.h"

AboutWindow::AboutWindow(QWidget * parent/*= 0*/): QWidget()
{
	setWindowTitle("About Virtual Jaguar...");

	layout = new QVBoxLayout();
	layout->setSizeConstraint(QLayout::SetFixedSize);
//	layout->setMargin(Style::WindowMargin);
//	layout->setSpacing(Style::WidgetSpacing);
	setLayout(layout);

//  logo = new Logo;
//  logo->setFixedSize(600, 106);
//  layout->addWidget(logo);
	image = new QLabel();
	image->setPixmap(QPixmap(":/res/vj_title_small.png"));
//hrm, this doesn't work
	layout->addWidget(image);

	text = new QLabel(tr(
		"<table>"
		"<tr><td align='right'><b>Version: </b></td><td>2.0.0</td></tr>"
		"<tr><td align='right'><b>Author: </b></td><td>James L. Hammons & others</td></tr>"
		"<tr><td align='right'><b>Homepage: </b></td><td>http://icculus.org/virtualjaguar/</td></tr>"
		"</table>"
	));
	layout->addWidget(text);
}


// Byuu's version...
#if 0
#include "about.moc"
AboutWindow *aboutWindow;

AboutWindow::AboutWindow() : QbWindow(config().geometry.aboutWindow) {
  setObjectName("about-window");
  setWindowTitle("About bsnes ...");

  layout = new QVBoxLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  logo = new Logo;
  logo->setFixedSize(600, 106);
  layout->addWidget(logo);

  info = new QLabel(string() <<
    "<table>"
    "<tr><td align='right'><b>Version: </b></td><td>" << bsnesVersion << "</td></tr>"
    "<tr><td align='right'><b>Author: </b></td><td>byuu</td></tr>"
    "<tr><td align='right'><b>Homepage: </b></td><td>http://byuu.org/</td></tr>"
    "</table>"
  );
  layout->addWidget(info);
}

void AboutWindow::Logo::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  QPixmap pixmap(":/logo.png");
  painter.drawPixmap(0, 0, pixmap);
}
#endif
