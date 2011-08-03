//
// help.cpp - Help file
//
// by James L. Hammons
// (C) 2011 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  08/01/2011  Created this file
//

// STILL TO DO:
//

#include "help.h"

HelpWindow::HelpWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog)
{
	setWindowTitle(tr("Virtual Jaguar Help"));

	// Need to set the size as well...
//  resize(560, 480);
	resize(560, 480);

	layout = new QVBoxLayout();
	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

//	image = new QLabel();
//	image->setAlignment(Qt::AlignRight);
//	image->setPixmap(QPixmap(":/res/vj_title_small.png"));
//	layout->addWidget(image);

//	QString s = QString(tr("SVN %1<br>")).arg(__DATE__);
	QString s;// = QString("");
	s.append(tr(
		"<h1>Virtual Jaguar Documentation</h1>"
		"<br><br>"
		"<b><i>Coming soon!</i></b>"
	));
	text = new QTextBrowser;
	text->setHtml(s);
	layout->addWidget(text);
}


#if 0
#include "htmlviewer.moc"
HtmlViewerWindow *htmlViewerWindow;

HtmlViewerWindow::HtmlViewerWindow() {
  setObjectName("html-window");
  resize(560, 480);
  setGeometryString(&config().geometry.htmlViewerWindow);
  application.windowList.add(this);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(0);
  setLayout(layout);

  document = new QTextBrowser;
  layout->addWidget(document);
}

void HtmlViewerWindow::show(const char *title, const char *htmlData) {
  document->setHtml(string() << htmlData);
  setWindowTitle(title);
  Window::show();
}
#endif
