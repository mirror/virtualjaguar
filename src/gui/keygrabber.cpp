//
// keygrabber.cpp - Widget to grab a key and dismiss itself
//
// by James L. Hammons
// (C) 2011 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  07/18/2011  Created this file
//

#include "keygrabber.h"


KeyGrabber::KeyGrabber(QWidget * parent/*= 0*/): QDialog(parent)
{
	label = new QLabel(this);
	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->addWidget(label);
	setLayout(mainLayout);
	setWindowTitle(tr("Grab"));
}

KeyGrabber::~KeyGrabber()
{
}

void KeyGrabber::SetText(QString keyText)
{
	QString text = QString(tr("Press key for \"%1\"<br>(ESC to cancel)")).arg(keyText);
	label->setText(text);
}

void KeyGrabber::keyPressEvent(QKeyEvent * e)
{
	key = e->key();
	accept();
}
