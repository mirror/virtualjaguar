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
	profileList(new QComboBox(this)),
	redefineAll(new QPushButton(tr("Define All Inputs"))),
	controllerWidget(new ControllerWidget(this))
{
	QVBoxLayout * layout = new QVBoxLayout;
	QHBoxLayout * top = new QHBoxLayout;
	layout->addLayout(top);
	top->addWidget(label);
	top->addWidget(profileList, 0, Qt::AlignLeft);
	layout->addWidget(controllerWidget);
	layout->addWidget(redefineAll, 0, Qt::AlignHCenter);
	setLayout(layout);
	// At least by doing this, it keeps the QComboBox from resizing itself too
	// large and breaking the layout. :-P
	setFixedWidth(sizeHint().width());

	connect(redefineAll, SIGNAL(clicked()), this, SLOT(DefineAllKeys()));
	connect(profileList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeProfile(int)));

	// Set up the profile combobox (Keyboard is the default, and always
	// present)
	profileList->addItem(tr("Keyboard"));

	for(int i=0; i<Gamepad::numJoysticks; i++)
		profileList->addItem(Gamepad::GetJoystickName(i));
}


ControllerTab::~ControllerTab()
{
}


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


void ControllerTab::ChangeProfile(int profile)
{
printf("You selected profile: %s\n", (profile == 0 ? "Keyboard" : Gamepad::GetJoystickName(profile - 1)));
}

#if 0
The profiles need the following:

 - The name of the controller
 - A unique human readable ID
 - The key definitions for that controller (keyboard keys can be mixed in)

So there can be more than one profile for each unique controller; the
relationship is many-to-one. So basically, how it works it like this: SDL
reports all connected controllers. If there are none connected, the default
controller is the keyboard (which can have multiple profiles). The UI only
presents those profiles which are usuable with the controllers that are plugged
in, all else is ignored. The user can pick the profile for the controller and
configure the keys for it; the UI automagically saves everything.

How to handle the case of identical controllers being plugged in? How does the
UI know which is which? Each controller will have a mapping to a default
Jaguar controller (#1 or #2). Still doesn't prevent confusion though. Actually,
it can: The profile can have a field that maps it to a preferred Jaguar
controller, which can also be both (#1 AND #2--in this case we can set it to
zero which means no preference). If the UI detects two of the same controller
and each can be mapped to the same profile, it assigns them in order since it
doesn't matter, the profiles are identical.

The default profile is always available and is the keyboard (hey, we're PC
centric here). The default profile is usually #0.

Can there be more than one keyboard profile? Why not? You will need separate
ones for controller #1 and controller #2.

A profile might look like this:

Field 1: Nostomo N45 Analog
Field 2: Dad's #1
Field 3: Jaguar controller #1
Field 4: The button/stick mapping

Profile # would be implicit in the order that they are stored in the internal
data structure.

When a new controller is plugged in with no profiles attached, it defaults to
a set keyboard layout which the user can change. So every new controller will
always have at least one profile.
#endif

