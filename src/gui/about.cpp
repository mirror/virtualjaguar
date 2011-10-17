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
// JLH  01/22/2010  Fleshed out the credits a bit more
// JLH  01/22/2010  Fixed centering and decorating of window
// JLH  10/08/2011  Updated credits, added Esc & Return as exit keys
//

// STILL TO DO:
//

#include "about.h"
#include "version.h"

AboutWindow::AboutWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog)
{
	setWindowTitle(tr("About Virtual Jaguar..."));

	layout = new QVBoxLayout();
	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	image = new QLabel();
	image->setAlignment(Qt::AlignRight);
	image->setPixmap(QPixmap(":/res/vj_title_small.png"));
	layout->addWidget(image);

	QString s;// = QString("");
	s.append(tr(
		"<table>"
		"<tr><td align='right'><b>Version: </b></td><td>"
		VJ_RELEASE_VERSION " (" VJ_RELEASE_SUBVERSION ")"
		"</td></tr>"
		"<tr><td align='right'><b>Coders: </b></td><td>James L. Hammons (shamus)<br>Niels Wagenaar (nwagenaar)<br>Carwin Jones (Caz)<br>Adam Green</td></tr>"
		"<tr><td align='right'><b>Testers: </b></td><td>Cyrano Jones, Robert R, TheUMan, Dissection,<br>overridex, geormetal</td></tr>"
		"<tr><td align='right'><b>Build Team: </b></td><td>ggn (win32)<br>LinkoVitch, goldenegg (MacOS)</td></tr>"
		"<tr><td align='right'><b>Homepage: </b></td><td>http://icculus.org/virtualjaguar/</td></tr>"
		"</table>"
		"<br><br>"
		"<i>The Virtual Jaguar team would like to express their gratitude to:</i>"
		"<br><br>"
		"<b>Aaron Giles</b> for the original CoJag sources<br>"
		"<b>David Raingeard</b> for the original Virtual Jaguar sources<br>"
		"<b>Karl Stenerud</b> for his Musashi 68K emulator<br>"
		"<b>Sam Lantinga</b> for his amazing SDL libraries<br>"
		"<b>Ryan C. Gordon</b> for Virtual Jaguar's web presence<br>"
		"<b>Curt Vendel</b> for various Jaguar & other goodies (you <i>rock!</i>)<br>"
		"<b>Reboot</b> for reasons too numerous to mention<br>"
		"<b>The Free Jaguar Project</b> (you know why) ;-)<br>"
		"The guys over at <b>Atari Age</b> :-)<br>"
		"<b>byuu</b> for BSNES and showing us what was possible"
	));
	text = new QLabel(s);
	layout->addWidget(text);
}

void AboutWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Return)
		hide();
}
