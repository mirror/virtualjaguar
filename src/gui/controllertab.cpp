//
// controllertab.cpp: "Controller" tab on the config dialog
//
// Part of the Virtual Jaguar Project
// (C) 2011 Underground Software
// See the README and GPLv3 files for licensing and warranty information
//
// JLH = James Hammons <jlhamm@acm.org>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JLH  06/23/2011  Created this file
// JLH  07/20/2011  Fixed a bunch of stuff
//

#include "controllertab.h"

#include "controllerwidget.h"
#include "gamepad.h"
#include "joystick.h"
#include "keygrabber.h"


ControllerTab::ControllerTab(QWidget * parent/*= 0*/): QWidget(parent),
	label(new QLabel(tr("Controller:"))),
	profile(new QComboBox(this)),
	redefineAll(new QPushButton(tr("Define All Inputs"))),
	controllerWidget(new ControllerWidget(this))
{
	QVBoxLayout * layout = new QVBoxLayout;
	QHBoxLayout * top = new QHBoxLayout;
	layout->addLayout(top);
	top->addWidget(label);
	top->addWidget(profile, 0, Qt::AlignLeft);
	layout->addWidget(controllerWidget);
	layout->addWidget(redefineAll, 0, Qt::AlignHCenter);
//	layout->setFixedWidth(label->width());
//	layout->setSizeConstraint(QLayout::SetFixedSize);
//	top->setSizeConstraint(QLayout::SetFixedSize);
//printf("cw width = %i, label width = %i (min=%i, sizehint=%i)\n", controllerWidget->width(), label->width(), label->minimumWidth(), label->sizeHint().width());
	// This is ugly, ugly, ugly. But it works. :-P It's a shame that Qt's
	// layout system doesn't seem to allow for a nicer way to handle this.
//	profile->setFixedWidth(controllerWidget->sizeHint().width() - label->sizeHint().width());
	setLayout(layout);
	setFixedWidth(sizeHint().width());

	connect(redefineAll, SIGNAL(clicked()), this, SLOT(DefineAllKeys()));

//this is the default. :-/ need to set it somewhere else i guess...
//	profile->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
	profile->addItem(tr("Keyboard"));

	for(int i=0; i<Gamepad::numJoysticks; i++)
		profile->addItem(Gamepad::GetJoystickName(i));
}


ControllerTab::~ControllerTab()
{
}


//QSize ControllerTab::sizeHint(void) const
//{
//	return 
//}


void ControllerTab::DefineAllKeys(void)
{
//	char jagButtonName[21][10] = { "Up", "Down", "Left", "Right",
//		"*", "7", "4", "1", "0", "8", "5", "2", "#", "9", "6", "3",
//		"A", "B", "C", "Option", "Pause" };
	int orderToDefine[21] = { 0, 1, 2, 3, 18, 17, 16, 20, 19, 7, 11, 15, 6, 10, 14, 5, 9, 13, 8, 4, 12 };
	KeyGrabber keyGrab(this);

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
//		keyGrab.SetText(jagButtonName[orderToDefine[i]]);
		keyGrab.SetKeyText(orderToDefine[i]);
		keyGrab.exec();
		int key = keyGrab.key;

		if (key == Qt::Key_Escape)
			break;

		// Otherwise, populate the appropriate spot in the settings & update screen...
		controllerWidget->keys[orderToDefine[i]] = key;
		controllerWidget->update();
	}
}

